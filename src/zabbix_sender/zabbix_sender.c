/* 
** ZABBIX
** Copyright (C) 2000-2005 SIA Zabbix
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**/

#include "common.h"

#include "threads.h"
#include "comms.h"
#include "cfg.h"
#include "log.h"
#include "zbxgetopt.h"
#include "zbxjson.h"

char *progname = NULL;

char title_message[] = "ZABBIX send";

char usage_message[] = "[-Vhv] {[-zpsI] -ko | [-zpI] -i <file>} [-c <file>]";

#ifdef HAVE_GETOPT_LONG
char *help_message[] = {
	"Options:",
	"  -c --config <File>                   Specify configuration file",
	"",
	"  -z --zabbix-server <Server>          Hostname or IP address of ZABBIX Server",
	"  -p --port <Server port>              Specify port number of server trapper running on the server. Default is 10051",
	"  -s --host <Hostname>                 Specify host name. Host IP address and DNS name will not work.",
	"  -I --source-address <ip address>     Specify source IP address",
	"",
	"  -k --key <Key>                       Specify metric name (key) we want to send",
	"  -o --value <Key value>               Specify value of the key",
	"",
	"  -i --input-file <input_file>         Load values from input file",
	"                                       Each line of file contains: <zabbix_server> <hostname> <port> <key> <value>",
	"",
	"  -v --verbose                         Verbose mode, -vv for more details",
	"",
	" Other options:",
	"  -h --help                            Give this help",
	"  -V --version                         Display version number",
        0 /* end of text */
};
#else
char *help_message[] = {
	"Options:",
	"  -c <File>                    Specify configuration file",
	"",
	"  -z <Server>                  Hostname or IP address of ZABBIX Server.",
	"  -p <Server port>             Specify port number of server trapper running on the server. Default is 10051.",
	"  -s <Hostname>                Specify hostname or IP address of a host.",
	"  -I <ip address>              Specify source IP address",
	"",
	"  -k <Key>                     Specify metric name (key) we want to send.",
	"  -o <Key value>               Specify value of the key.",
	"",
	"  -i <Input file>              Load values from input file.",
	"                               Each line of file contains: <zabbix_server> <hostname> <port> <key> <value>.",
	"",
	"  -v                           Verbose mode",
	"",
	" Other options:",
	"  -h                           Give this help.",
	"  -V                           Display version number.",
        0 /* end of text */
};
#endif

/* COMMAND LINE OPTIONS */

/* long options */

static struct zbx_option longopts[] =
{
	{"config",		1,	NULL,	'c'},
	{"zabbix-server",	1,	NULL,	'z'},
	{"port",		1,	NULL,	'p'},
	{"host",		1,	NULL,	's'},
	{"source-address",	1,	NULL,	'I'},
	{"key",			1,	NULL,	'k'},
	{"value",		1,	NULL,	'o'},
	{"input-file",		1,	NULL,	'i'},
	{"verbose",        	0,      NULL,	'v'},
	{"help",        	0,      NULL,	'h'},
	{"version",     	0,      NULL,	'V'},
	{0,0,0,0}
};

/* short options */

static char     shortopts[] = "c:I:z:p:s:k:o:i:vhV";

/* end of COMMAND LINE OPTIONS*/

static int	CONFIG_LOG_LEVEL = LOG_LEVEL_CRIT;

static char*	INPUT_FILE = NULL;

static char*	CONFIG_SOURCE_IP = NULL;
static char*	ZABBIX_SERVER = NULL;
unsigned short	ZABBIX_SERVER_PORT = 0;
static char*	ZABBIX_HOSTNAME = NULL;
static char*	ZABBIX_KEY = NULL;
static char*	ZABBIX_KEY_VALUE = NULL;

#if !defined(_WINDOWS)

static void    send_signal_handler( int sig )
{
	if( SIGALRM == sig )
	{
		signal( SIGALRM, send_signal_handler );
		zabbix_log( LOG_LEVEL_WARNING, "Timeout while executing operation");
	}
 
	if( SIGQUIT == sig || SIGINT == sig || SIGTERM == sig )
	{
/*		fprintf(stderr,"\nGot QUIT or INT or TERM signal. Exiting..." ); */
	}
	exit( FAIL );
}

#endif /* NOT _WINDOWS */

typedef struct zbx_active_metric_type
{
	char		*source_ip, *server;
	unsigned short	port;
	struct zbx_json	json;
} ZBX_THREAD_SENDVAL_ARGS;

/******************************************************************************
 *                                                                            *
 * Function: check_response                                                   *
 *                                                                            *
 * Purpose: Check if json response is SUCCEED                                 *
 *                                                                            *
 * Parameters: result SUCCEED or FAIL                                         *
 *                                                                            *
 * Return value:  SUCCEED - processed successfully                            *
 *                FAIL - an error occured                                     *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int	check_response(char *response)
{
	struct 		zbx_json_parse jp;
	const char 	*p;
	char		value[MAX_STRING_LEN];
	char		info[MAX_STRING_LEN];

	int	ret = SUCCEED;

	ret = zbx_json_open(response, &jp);

	if(SUCCEED == ret)
	{
		if (NULL == (p = zbx_json_pair_by_name(&jp, ZBX_PROTO_TAG_RESPONSE))
				|| NULL == zbx_json_decodevalue(p, value, sizeof(value)))
		{
			ret = FAIL;
		}
	}

	if(SUCCEED == ret)
	{
		if(strcmp(value, ZBX_PROTO_VALUE_SUCCESS) != 0)
		{
			ret = FAIL;
		}
	}

	if (NULL != (p = zbx_json_pair_by_name(&jp, ZBX_PROTO_TAG_INFO))
			&& NULL != zbx_json_decodevalue(p, info, sizeof(info)))
	{
		printf("Info from server: \"%s\"\n",
			info);
	}

	return ret;
}

static ZBX_THREAD_ENTRY(send_value, args)
{
	ZBX_THREAD_SENDVAL_ARGS *sentdval_args;

	zbx_sock_t	sock;

	char	*answer = NULL;

	int		tcp_ret = FAIL, ret = FAIL;

	assert(args);

	sentdval_args = ((ZBX_THREAD_SENDVAL_ARGS *)args);

#if !defined(_WINDOWS)
	signal( SIGINT,  send_signal_handler );
	signal( SIGTERM, send_signal_handler );
	signal( SIGQUIT, send_signal_handler );
	signal( SIGALRM, send_signal_handler );
#endif /* NOT _WINDOWS */

	if (SUCCEED == (tcp_ret = zbx_tcp_connect(&sock, CONFIG_SOURCE_IP, sentdval_args->server, sentdval_args->port, SENDER_TIMEOUT))) {
		tcp_ret = zbx_tcp_send(&sock, sentdval_args->json.buffer);

		if( SUCCEED == tcp_ret )
		{
			if( SUCCEED == (tcp_ret = zbx_tcp_recv(&sock, &answer)) )
			{
				zabbix_log( LOG_LEVEL_DEBUG, "Answer [%s]", answer);
				if( !answer || check_response(answer) != SUCCEED )
				{
					zabbix_log( LOG_LEVEL_WARNING, "Incorrect answer from server [%s]", answer);
				}
				else
				{
					ret = SUCCEED;
				}
			}
		}

	}
	zbx_tcp_close(&sock);

	if( FAIL == tcp_ret )
	{
		zabbix_log(LOG_LEVEL_DEBUG, "Send value error: %s", zbx_tcp_strerror());
	}

	zbx_tread_exit(ret);
}

static void    init_config(const char* config_file)
{
	char*	config_source_ip_from_conf = NULL;
	char*	zabbix_server_from_conf = NULL;
	int	zabbix_server_port_from_conf = 0;
	char*	zabbix_hostname_from_conf = NULL;
	char*	c = NULL;

	struct cfg_line cfg[]=
	{
		/* PARAMETER	,VAR				,FUNC	,TYPE(0i,1s)	,MANDATORY	,MIN			,MAX		*/
		{"SourceIP"	,&config_source_ip_from_conf	,0	,TYPE_STRING	,PARM_OPT	,0			,0		},
		{"Server"	,&zabbix_server_from_conf	,0	,TYPE_STRING	,PARM_OPT	,0			,0		},
		{"ServerPort"	,&zabbix_server_port_from_conf	,0	,TYPE_INT	,PARM_OPT	,MIN_ZABBIX_PORT	,MAX_ZABBIX_PORT},
		{"Hostname"	,&zabbix_hostname_from_conf	,0	,TYPE_STRING	,PARM_OPT	,0			,0		},
		{0}
	};

	if( config_file )
	{
		parse_cfg_file(config_file, cfg);

		if (NULL != config_source_ip_from_conf)
		{
			if (NULL == CONFIG_SOURCE_IP)	/* apply parameter only if unsetted */
			{
				CONFIG_SOURCE_IP = strdup(config_source_ip_from_conf);
			}
			zbx_free(config_source_ip_from_conf);
		}

		if( zabbix_server_from_conf )
		{
			if( !ZABBIX_SERVER )
			{ /* apply parameter only if unsetted */
				if( (c = strchr(zabbix_server_from_conf, ',')) )
				{ /* get only first server */
					*c = '\0';
				}
				ZABBIX_SERVER = strdup(zabbix_server_from_conf);
			}
			zbx_free(zabbix_server_from_conf);
		}

		if( !ZABBIX_SERVER_PORT && zabbix_server_port_from_conf )
		{ /* apply parameter only if unsetted */
			ZABBIX_SERVER_PORT = zabbix_server_port_from_conf;
		}

		if( zabbix_hostname_from_conf )
		{
			if( !ZABBIX_HOSTNAME )
			{ /* apply parameter only if unsetted */
				ZABBIX_HOSTNAME = strdup(zabbix_hostname_from_conf);
			}
			zbx_free(zabbix_hostname_from_conf);
		}
	}
}

static zbx_task_t parse_commandline(int argc, char **argv)
{
	zbx_task_t      task    = ZBX_TASK_START;
	char    ch      = '\0';

	/* Parse the command-line. */
	while ((ch = (char)zbx_getopt_long(argc, argv, shortopts, longopts, NULL)) != (char)EOF)
		switch (ch) {
			case 'c':
				CONFIG_FILE = strdup(zbx_optarg);
				break;
			case 'h':
				help();
				exit(-1);
				break;
			case 'V':
				version();
				exit(-1);
				break;
			case 'I': 
				CONFIG_SOURCE_IP = strdup(zbx_optarg);
				break;
			case 'z': 
				ZABBIX_SERVER = strdup(zbx_optarg);
				break;
			case 'p':
				ZABBIX_SERVER_PORT = (unsigned short)atoi(zbx_optarg);
				break;
			case 's':
				ZABBIX_HOSTNAME = strdup(zbx_optarg);
				break;
			case 'k':
				ZABBIX_KEY = strdup(zbx_optarg);
				break;
			case 'o':
				ZABBIX_KEY_VALUE = strdup(zbx_optarg);
				break;
			case 'i':
				INPUT_FILE = strdup(zbx_optarg);
				break;
			case 'v':
				if(CONFIG_LOG_LEVEL == LOG_LEVEL_WARNING)
					CONFIG_LOG_LEVEL = LOG_LEVEL_DEBUG;
				else
					CONFIG_LOG_LEVEL = LOG_LEVEL_WARNING;
				break;
			default:
				usage();
				exit(FAIL);
				break;
		}

	if(ZABBIX_SERVER == NULL)
	{
		usage();
		exit(FAIL);
	}

	return task;
}

int main(int argc, char **argv)
{
	FILE	*in;

	char	in_line[MAX_STRING_LEN],
		*s,
		*hostname,
		*key,
		*key_value;

	int	task = ZBX_TASK_START,
		total_count = 0,
		succeed_count = 0,
		ret = SUCCEED;

	ZBX_THREAD_SENDVAL_ARGS sentdval_args;

	progname = get_programm_name(argv[0]);

	task = parse_commandline(argc, argv);

	init_config(CONFIG_FILE);

	zabbix_open_log(LOG_TYPE_UNDEFINED, CONFIG_LOG_LEVEL, NULL);

	if (NULL == ZABBIX_SERVER)
	{
		zabbix_log(LOG_LEVEL_WARNING, "'Server' parameter required");
		goto exit;
	}
	if (0 == ZABBIX_SERVER_PORT)
		ZABBIX_SERVER_PORT = 10051;

	if (MIN_ZABBIX_PORT > ZABBIX_SERVER_PORT)
	{
		zabbix_log(LOG_LEVEL_WARNING, "Incorrect port number [%d]. Allowed [%d:%d]",
				(int)ZABBIX_SERVER_PORT, (int)MIN_ZABBIX_PORT, (int)MAX_ZABBIX_PORT);
		goto exit;
	}

	sentdval_args.server	= ZABBIX_SERVER;
	sentdval_args.port	= ZABBIX_SERVER_PORT;

	zbx_json_init(&sentdval_args.json, ZBX_JSON_STAT_BUF_LEN);
	zbx_json_addstring(&sentdval_args.json, ZBX_PROTO_TAG_REQUEST, ZBX_PROTO_VALUE_SENDER_DATA, ZBX_JSON_TYPE_STRING);
	zbx_json_addarray(&sentdval_args.json, ZBX_PROTO_TAG_DATA);

	if( INPUT_FILE )
	{
		if (NULL == (in = fopen(INPUT_FILE, "r")) )
		{
			zabbix_log(LOG_LEVEL_WARNING, "Cannot open [%s] [%s]", INPUT_FILE, strerror(errno));
			return FAIL;
		}

		while (NULL != fgets(in_line, sizeof(in_line), in))	/* <hostname> <key> <value> */
		{
			total_count++; /* also used as inputline */

			hostname = in_line;

			if (NULL == (key = strchr(hostname, ' ')))
			{
				zabbix_log(LOG_LEVEL_WARNING, "[line %d] 'Key' required", total_count);
				continue;
			}

			*key++ = '\0';

			if (NULL == (key_value = strchr(key, ' ')))
			{
				zabbix_log(LOG_LEVEL_WARNING, "[line %d] 'Key value' required", total_count);
				continue;
			}

			*key_value++ = '\0';

			for (s = key_value; s && *s; s++)
			{
				if(*s == '\r' || *s == '\n' )
				{
					*s = '\0';
					break;
				}
			}

			zbx_json_addobject(&sentdval_args.json, NULL);
			zbx_json_addstring(&sentdval_args.json, ZBX_PROTO_TAG_HOST, hostname, ZBX_JSON_TYPE_STRING);
			zbx_json_addstring(&sentdval_args.json, ZBX_PROTO_TAG_KEY, key, ZBX_JSON_TYPE_STRING);
			zbx_json_addstring(&sentdval_args.json, ZBX_PROTO_TAG_VALUE, key_value, ZBX_JSON_TYPE_STRING);
			zbx_json_close(&sentdval_args.json);

			succeed_count++;
		}

		fclose(in);
	}
	else
	{
		total_count++;

		do /* try block simulation */
		{
			if (NULL == ZABBIX_HOSTNAME)
			{
				zabbix_log(LOG_LEVEL_WARNING, "'Hostname' parameter required");
				break;
			}
			if (NULL == ZABBIX_KEY)
			{
				zabbix_log(LOG_LEVEL_WARNING, "Key required");
				break;
			}
			if (NULL == ZABBIX_KEY_VALUE)
			{
				zabbix_log(LOG_LEVEL_WARNING, "Key value required");
				break;
			}

			hostname	= ZABBIX_HOSTNAME;
			key		= ZABBIX_KEY;
			key_value	= ZABBIX_KEY_VALUE;

			zbx_json_addobject(&sentdval_args.json, NULL);
			zbx_json_addstring(&sentdval_args.json, ZBX_PROTO_TAG_HOST, hostname, ZBX_JSON_TYPE_STRING);
			zbx_json_addstring(&sentdval_args.json, ZBX_PROTO_TAG_KEY, key, ZBX_JSON_TYPE_STRING);
			zbx_json_addstring(&sentdval_args.json, ZBX_PROTO_TAG_VALUE, key_value, ZBX_JSON_TYPE_STRING);
			zbx_json_close(&sentdval_args.json);

			succeed_count++;
		}
		while(0); /* try block simulation */
	}

	if (succeed_count > 0)
	{
		if (SUCCEED == zbx_thread_wait(zbx_thread_start(send_value, &sentdval_args)))
		{
			printf("sent: %d; skipped: %d; total: %d\n", succeed_count, (total_count - succeed_count), total_count);
		}
		else
		{
			printf("Sending failed. Use option -vv for more detailed output.\n");
		}
	}

	zbx_json_free(&sentdval_args.json);
exit:
	zabbix_close_log();

	return ret;
}

