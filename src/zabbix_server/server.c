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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <string.h>


/* Required for getpwuid */
#include <pwd.h>

#include <signal.h>
#include <errno.h>

#include <time.h>
/* getopt() */
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "cfg.h"
#include "pid.h"
#include "db.h"
#include "log.h"
#include "zlog.h"

#include "common.h"
#include "functions.h"
#include "expression.h"
#include "sysinfo.h"

#include "daemon.h"

#include "alerter/alerter.h"
#include "housekeeper/housekeeper.h"
#include "pinger/pinger.h"
#include "poller/poller.h"
#include "poller/checks_snmp.h"
#include "timer/timer.h"
#include "trapper/trapper.h"
#include "nodewatcher/nodewatcher.h"
#include "utils/nodechange.h"

#define       LISTENQ 1024

char *progname = NULL;
char title_message[] = "ZABBIX Server (daemon)";
char usage_message[] = "[-hv] [-c <file>]";

#ifndef HAVE_GETOPT_LONG
char *help_message[] = {
        "Options:",
        "  -c <file>       Specify configuration file",
        "  -h              give this help",
        "  -n <nodeid>     convert database data to new nodeid",
        "  -v              display version number",
        0 /* end of text */
};
#else
char *help_message[] = {
        "Options:",
        "  -c --config <file>       Specify configuration file",
        "  -h --help                give this help",
        "  -n --new-nodeid <nodeid> convert database data to new nodeid",
        "  -v --version             display version number",
        0 /* end of text */
};
#endif

struct option longopts[] =
{
	{"config",	1,	0,	'c'},
	{"help",	0,	0,	'h'},
	{"new-nodeid",	1,	0,	'n'},
	{"version",	0,	0,	'v'},
	{0,0,0,0}
};


pid_t	*threads=NULL;


int	CONFIG_ALERTER_FORKS		= 1;
int	CONFIG_HOUSEKEEPER_FORKS	= 1;
int	CONFIG_NODEWATCHER_FORKS	= 1;
int	CONFIG_PINGER_FORKS		= 1;
int	CONFIG_POLLER_FORKS		= 5;
int	CONFIG_TIMER_FORKS		= 1;
int	CONFIG_TRAPPERD_FORKS		= 5;
int	CONFIG_UNREACHABLE_POLLER_FORKS	= 1;

int	CONFIG_LISTEN_PORT		= 10051;
char	*CONFIG_LISTEN_IP		= NULL;
int	CONFIG_TRAPPER_TIMEOUT		= TRAPPER_TIMEOUT;
/**/
/*int	CONFIG_NOTIMEWAIT		=0;*/
int	CONFIG_HOUSEKEEPING_FREQUENCY	= 1;
int	CONFIG_SENDER_FREQUENCY		= 30;
int	CONFIG_PINGER_FREQUENCY		= 60;
int	CONFIG_DISABLE_PINGER		= 0;
int	CONFIG_DISABLE_HOUSEKEEPING	= 0;
int	CONFIG_UNREACHABLE_PERIOD	= 45;
int	CONFIG_UNREACHABLE_DELAY	= 15;
int	CONFIG_UNAVAILABLE_DELAY	= 60;
int	CONFIG_LOG_LEVEL		= LOG_LEVEL_WARNING;
char	*CONFIG_ALERT_SCRIPTS_PATH	= NULL;
char	*CONFIG_FPING_LOCATION		= NULL;
char	*CONFIG_DBHOST			= NULL;
char	*CONFIG_DBNAME			= NULL;
char	*CONFIG_DBUSER			= NULL;
char	*CONFIG_DBPASSWORD		= NULL;
char	*CONFIG_DBSOCKET		= NULL;
int	CONFIG_DBPORT			= 3306;
int	CONFIG_ENABLE_REMOTE_COMMANDS	= 0;

int	CONFIG_NODEID			= 0;
int	CONFIG_MASTER_NODEID		= 0;

/* From table config */
int	CONFIG_REFRESH_UNSUPPORTED	= 0;

/******************************************************************************
 *                                                                            *
 * Function: init_config                                                      *
 *                                                                            *
 * Purpose: parse config file and update configuration parameters             *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments: will terminate process if parsing fails                          *
 *                                                                            *
 ******************************************************************************/
void	init_config(void)
{
	static struct cfg_line cfg[]=
	{
/*		 PARAMETER	,VAR	,FUNC,	TYPE(0i,1s),MANDATORY,MIN,MAX	*/
		{"StartPingers",&CONFIG_PINGER_FORKS,0,TYPE_INT,PARM_OPT,0,255},
		{"StartPollers",&CONFIG_POLLER_FORKS,0,TYPE_INT,PARM_OPT,0,255},
		{"StartPollersUnreachable",&CONFIG_UNREACHABLE_POLLER_FORKS,0,TYPE_INT,PARM_OPT,0,255},
		{"StartTrappers",&CONFIG_TRAPPERD_FORKS,0,TYPE_INT,PARM_OPT,0,255},
		{"HousekeepingFrequency",&CONFIG_HOUSEKEEPING_FREQUENCY,0,TYPE_INT,PARM_OPT,1,24},
		{"SenderFrequency",&CONFIG_SENDER_FREQUENCY,0,TYPE_INT,PARM_OPT,5,3600},
		{"PingerFrequency",&CONFIG_PINGER_FREQUENCY,0,TYPE_INT,PARM_OPT,1,3600},
		{"FpingLocation",&CONFIG_FPING_LOCATION,0,TYPE_STRING,PARM_OPT,0,0},
		{"Timeout",&CONFIG_TIMEOUT,0,TYPE_INT,PARM_OPT,1,30},
		{"TrapperTimeout",&CONFIG_TRAPPER_TIMEOUT,0,TYPE_INT,PARM_OPT,1,30},
		{"UnreachablePeriod",&CONFIG_UNREACHABLE_PERIOD,0,TYPE_INT,PARM_OPT,1,3600},
		{"UnreachableDelay",&CONFIG_UNREACHABLE_DELAY,0,TYPE_INT,PARM_OPT,1,3600},
		{"UnavailableDelay",&CONFIG_UNAVAILABLE_DELAY,0,TYPE_INT,PARM_OPT,1,3600},
		{"ListenIP",&CONFIG_LISTEN_IP,0,TYPE_STRING,PARM_OPT,0,0},
		{"ListenPort",&CONFIG_LISTEN_PORT,0,TYPE_INT,PARM_OPT,1024,32768},
/*		{"NoTimeWait",&CONFIG_NOTIMEWAIT,0,TYPE_INT,PARM_OPT,0,1},*/
		{"DisablePinger",&CONFIG_DISABLE_PINGER,0,TYPE_INT,PARM_OPT,0,1},
		{"DisableHousekeeping",&CONFIG_DISABLE_HOUSEKEEPING,0,TYPE_INT,PARM_OPT,0,1},
		{"DebugLevel",&CONFIG_LOG_LEVEL,0,TYPE_INT,PARM_OPT,0,4},
		{"PidFile",&APP_PID_FILE,0,TYPE_STRING,PARM_OPT,0,0},
		{"LogFile",&CONFIG_LOG_FILE,0,TYPE_STRING,PARM_OPT,0,0},
		{"AlertScriptsPath",&CONFIG_ALERT_SCRIPTS_PATH,0,TYPE_STRING,PARM_OPT,0,0},
		{"DBHost",&CONFIG_DBHOST,0,TYPE_STRING,PARM_OPT,0,0},
		{"DBName",&CONFIG_DBNAME,0,TYPE_STRING,PARM_MAND,0,0},
		{"DBUser",&CONFIG_DBUSER,0,TYPE_STRING,PARM_OPT,0,0},
		{"DBPassword",&CONFIG_DBPASSWORD,0,TYPE_STRING,PARM_OPT,0,0},
		{"DBSocket",&CONFIG_DBSOCKET,0,TYPE_STRING,PARM_OPT,0,0},
		{"DBPort",&CONFIG_DBPORT,0,TYPE_INT,PARM_OPT,1024,65535},
		{"NodeID",&CONFIG_NODEID,0,TYPE_INT,PARM_OPT,0,65535},
		{0}
	};


	if(CONFIG_FILE == NULL)
	{
		CONFIG_FILE=strdup("/etc/zabbix/zabbix_server.conf");
	}

	parse_cfg_file(CONFIG_FILE,cfg);

	if(CONFIG_DBNAME == NULL)
	{
		zabbix_log( LOG_LEVEL_CRIT, "DBName not in config file");
		exit(1);
	}
	if(APP_PID_FILE == NULL)
	{
		APP_PID_FILE=strdup("/tmp/zabbix_server.pid");
	}
	if(CONFIG_ALERT_SCRIPTS_PATH == NULL)
	{
		CONFIG_ALERT_SCRIPTS_PATH=strdup("/home/zabbix/bin");
	}
	if(CONFIG_FPING_LOCATION == NULL)
	{
		CONFIG_FPING_LOCATION=strdup("/usr/sbin/fping");
	}

}

/*
void	trend(void)
{
	char		sql[MAX_STRING_LEN];
 
	DB_RESULT	result;
	DB_RESULT	result2;

	int		i,j;

	result2 = DBselect("select itemid from items");
	for(i=0;i<DBnum_rows(result2);i++)
	{
		result = DBselect("select clock-clock%%3600, count(*),min(value),avg(value),max(value) from history where itemid=%d group by 1",atoi(DBget_field(result2,i,0)));
	
		for(j=0;j<DBnum_rows(result);j++)
		{
			zbx_snprintf(sql,sizeof(sql),"insert into trends (itemid, clock, num, value_min, value_avg, value_max) values (%d,%d,%d,%f,%f,%f)",atoi(DBget_field(result2,i,0)), atoi(DBget_field(result,j,0)),atoi(DBget_field(result,j,1)),atof(DBget_field(result,j,2)),atof(DBget_field(result,j,3)),atof(DBget_field(result,j,4)));
			DBexecute(sql);
		}
		DBfree_result(result);
	}
	DBfree_result(result2);
}
*/

int	tcp_listen(const char *host, int port, socklen_t *addrlenp)
{
	int	sockfd;
	struct	sockaddr_in      serv_addr;
	int	on;
/*	struct linger ling;*/

	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		zabbix_log( LOG_LEVEL_CRIT, "Cannot create socket");
		exit(1);
	}

	/* Enable address reuse */
	/* This is to immediately use the address even if it is in TIME_WAIT state */
	/* http://www-128.ibm.com/developerworks/linux/library/l-sockpit/index.html */
	on = 1;
	if( -1 == setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) ))
	{
		zabbix_log(LOG_LEVEL_WARNING, "Cannot setsockopt SO_REUSEADDR [%s]", strerror(errno));
	}

/*	if(CONFIG_NOTIMEWAIT == 1)
	{
		ling.l_onoff=1;
		ling.l_linger=0;
		if(setsockopt(sockfd,SOL_SOCKET,SO_LINGER,&ling,sizeof(ling))==-1)
		{
			zabbix_log(LOG_LEVEL_WARNING, "Cannot setsockopt SO_LINGER [%s]", strerror(errno));
		}
	}*/

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family      = AF_INET;
	if(CONFIG_LISTEN_IP == NULL)
	{
		serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else
	{
		serv_addr.sin_addr.s_addr = inet_addr(CONFIG_LISTEN_IP);
	}
	serv_addr.sin_port        = htons(port);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		zabbix_log( LOG_LEVEL_CRIT, "Cannot bind to port %d. Another zabbix_server running? Shutting down...", port);
		exit(FAIL);
	}
	
	if(listen(sockfd, LISTENQ) !=0 )
	{
		zabbix_log( LOG_LEVEL_CRIT, "listen() failed");
		exit(FAIL);
	}

	*addrlenp = sizeof(serv_addr);

	return  sockfd;
}

/******************************************************************************
 *                                                                            *
 * Function: test                                                             *
 *                                                                            *
 * Purpose: test a custom developed functions                                 *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Eugene Grigorjev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/

/* #define TEST */

#ifdef TEST

#define ZBX_LIM_PRINT(msg, str_len, str, limit) \
	printf(msg "(%i): %." #limit "s<br/>\n", str_len, str, str_len > limit ? "..." : "")

//#define ZBX_LIM_PRINT(msg, str_len, str, limit) 

size_t HEADERFUNCTION( void *ptr, size_t size, size_t nmemb, void *stream)
{
	ZBX_LIM_PRINT("HEADERFUNCTION", size*nmemb, ptr, 300);

	return size*nmemb;
}

size_t WRITEFUNCTION( void *ptr, size_t size, size_t nmemb, void *stream)
{
	size_t s = size*nmemb + 1;
	char *str_dat = calloc(1, s);

	zbx_snprintf(str_dat,s,ptr);
	ZBX_LIM_PRINT("WRITEFUNCTION", s, str_dat, 65535);

	return size*nmemb;
}

#define XBX_CURL_ERR(msg) \
	printf("ERROR: " msg " [%s]\n", curl_easy_strerror(err))

#define ZBX_CURL_SET_OPT(opt_name, opt_val) \
	if(CURLE_OK != (err = curl_easy_setopt(easyhandle, opt_name, opt_val))) \
		XBX_CURL_ERR("curl_easy_setopt(" #opt_name ")")

typedef struct s_zbx_step
{
	char *url;
	char *post;
} zbx_step;

void test()
{
	int		err = CURLE_OK;
	CURL		*easyhandle = NULL;
	zbx_step	*stp = NULL;
	zbx_step	steps[] = {
		/*	URL						POST 			*/
		{"http://www.zabbix.com/forum/",			NULL},
		{"http://www.zabbix.com/forum/login.php",		"vb_login_username=Eugene"
									"&cookieuser="
									"&vb_login_password="
									"&s="
									"&do=login"
									"&forceredirect=1"
									"&vb_login_md5password=ec5287c45f0e70ec22d52e8bcbeeb640"
									"&vb_login_md5password_utf=ec5287c45f0e70ec22d52e8bcbeeb640"},
		{"http://www.zabbix.com/forum/",			NULL},
		{"http://www.zabbix.com/forum/usercp.php?",		NULL},
		{"http://www.zabbix.com/forum/login.php?do=logout",	NULL},
		{"http://www.zabbix.com/forum/",			NULL},
		{NULL, NULL}
		};


	printf("-= Test Started =-\n");

	if(NULL != (easyhandle = curl_easy_init()))
	{
		ZBX_CURL_SET_OPT(CURLOPT_COOKIEFILE,		"");
		ZBX_CURL_SET_OPT(CURLOPT_FOLLOWLOCATION,	1);
		ZBX_CURL_SET_OPT(CURLOPT_HEADERFUNCTION,	HEADERFUNCTION);
		ZBX_CURL_SET_OPT(CURLOPT_WRITEFUNCTION,		WRITEFUNCTION);

		for(stp=&steps[0];(*stp).url; stp++)
		{
			printf("****************************************************************\n");
			printf("{{ %s }}\n", stp->url);
			printf("****************************************************************\n");

			if(stp->post)	ZBX_CURL_SET_OPT(CURLOPT_POSTFIELDS,	stp->post);

			ZBX_CURL_SET_OPT(CURLOPT_URL,			stp->url);

			if(CURLE_OK != curl_easy_perform(easyhandle)) /* post away! */
				XBX_CURL_ERR("curl_easy_perform()");

			fflush(stdout);
		}
		(void)curl_easy_cleanup(easyhandle);
	}
	else
	{
		XBX_CURL_ERR("curl_easy_init()");
	}

	printf("\n-= Test completed =-\n");
}
#endif /* TEST */

/******************************************************************************
 *                                                                            *
 * Function: main                                                             *
 *                                                                            *
 * Purpose: executes server processes                                         *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
int main(int argc, char **argv)
{
	int	ch;

	int	nodeid;
	zbx_task_t	task  = ZBX_TASK_START;

#ifdef HAVE_ZZZ
	DB_RESULT	result;
	DB_ROW		row;
	const char ** v;
#endif



#ifdef HAVE_ZZZ
	init_config();

	DBconnect();
	result = DBselect("select NULL from history where itemid=20272222");
	row=DBfetch(result);
	if(!row) printf("OK");
	exit(0);
	while((row=DBfetch(result)))
	{
		printf("[%s]\n",row[0]);
	}
	DBfree_result(result);
	DBclose();
	return 0;
#endif
#ifdef HAVE_ZZZ
/* */
	DBconnect();
	result = DBselect("select itemid,key_,description from items");
	while ( SQLO_SUCCESS == sqlo_fetch(result, 1))
	{
		v = sqlo_values(result, NULL, 1);
		printf("%s %s %s\n",v[0],v[1],v[2]);
	}
	DBfree_result(result);
	DBclose();
/* */
	return 0;
#endif

	progname = argv[0];

/* Parse the command-line. */
	while ((ch = getopt_long(argc, argv, "c:n:hv",longopts,NULL)) != EOF)
	switch ((char) ch) {
		case 'c':
			CONFIG_FILE = strdup(optarg);
			break;
		case 'h':
			help();
			exit(-1);
			break;
		case 'n':
			nodeid=0;
			if(optarg)	nodeid = atoi(optarg);
			task = ZBX_TASK_CHANGE_NODEID;
			break;
		case 'v':
			version();
			exit(-1);
			break;
		default:
			usage();
			exit(-1);
			break;
        }

/*	init_metrics();*/

	init_config();

	switch (task) {
		case ZBX_TASK_CHANGE_NODEID:
			change_nodeid(0,nodeid);
			exit(-1);
			break;
		default:
			;
	}

#ifdef TEST
	if(CONFIG_LOG_FILE == NULL)
	{
		zabbix_open_log(LOG_TYPE_SYSLOG,CONFIG_LOG_LEVEL,NULL);
	}
	else
	{
		zabbix_open_log(LOG_TYPE_FILE,CONFIG_LOG_LEVEL,CONFIG_LOG_FILE);
	}

	zabbix_log( LOG_LEVEL_WARNING, "Starting zabbix_server. ZABBIX %s.", ZABBIX_VERSION);
	DBconnect();
	test();
	DBclose();
	return 0;
#endif /* TEST */
	
	return daemon_start(CONFIG_ALLOW_ROOT_PERMISSION);
}

int MAIN_ZABBIX_ENTRY(void)
{
        DB_RESULT       result;
        DB_ROW          row;

	int	i;
	pid_t	pid;

	int		listenfd;
	socklen_t	addrlen;

	char		host[128];
	
	int		server_num = 0;

	if(CONFIG_LOG_FILE == NULL)
	{
		zabbix_open_log(LOG_TYPE_SYSLOG,CONFIG_LOG_LEVEL,NULL);
	}
	else
	{
		zabbix_open_log(LOG_TYPE_FILE,CONFIG_LOG_LEVEL,CONFIG_LOG_FILE);
	}

/*	zabbix_log( LOG_LEVEL_WARNING, "INFO [%s]", ZBX_SQL_MOD(a,%d)); */
	zabbix_log( LOG_LEVEL_WARNING, "Starting zabbix_server. ZABBIX %s.", ZABBIX_VERSION);

	DBconnect();

	result = DBselect("select refresh_unsupported from config where " ZBX_COND_NODEID, LOCAL_NODE("configid"));
	row = DBfetch(result);

	if( (row != NULL) && DBis_null(row[0]) != SUCCEED)
	{
		CONFIG_REFRESH_UNSUPPORTED = atoi(row[0]);
	}
	DBfree_result(result);

	result = DBselect("select masterid from nodes where nodeid=%d", CONFIG_NODEID);
	row = DBfetch(result);

	if( (row != NULL) && DBis_null(row[0]) != SUCCEED)
	{
		CONFIG_MASTER_NODEID = atoi(row[0]);
	}
	DBfree_result(result);

/* Need to set trigger status to UNKNOWN since last run */
/* DBconnect() already made in init_config() */
/*	DBconnect();*/
	DBupdate_triggers_status_after_restart();

/*#define CALC_TREND*/

#ifdef CALC_TREND
	trend();
	return 0;
#endif
	DBclose();
	threads = calloc(1+CONFIG_POLLER_FORKS+CONFIG_TRAPPERD_FORKS+CONFIG_PINGER_FORKS+CONFIG_ALERTER_FORKS
		+CONFIG_HOUSEKEEPER_FORKS+CONFIG_TIMER_FORKS+CONFIG_UNREACHABLE_POLLER_FORKS
		+CONFIG_NODEWATCHER_FORKS,sizeof(pid_t));

	if(CONFIG_TRAPPERD_FORKS > 0)
	{
		listenfd = tcp_listen(host,CONFIG_LISTEN_PORT,&addrlen);
	}

	for(i=1; i<=CONFIG_POLLER_FORKS+CONFIG_TRAPPERD_FORKS+CONFIG_PINGER_FORKS+CONFIG_ALERTER_FORKS+CONFIG_HOUSEKEEPER_FORKS+CONFIG_TIMER_FORKS+CONFIG_UNREACHABLE_POLLER_FORKS+CONFIG_NODEWATCHER_FORKS; i++)
	{
		if((pid = fork()) == 0)
		{
			server_num = i;
			break; 
		}
		else
		{
			threads[i]=pid;
		}
	}

/*	zabbix_log( LOG_LEVEL_WARNING, "zabbix_server #%d started",server_num); */
	/* Main process */
	if(server_num == 0)
	{
		init_main_process();
		zabbix_log( LOG_LEVEL_WARNING, "server #%d started [Main]",server_num);
		for(;;)	zbx_sleep(3600);
	}


	if(server_num <= CONFIG_POLLER_FORKS)
	{
#ifdef HAVE_SNMP
		init_snmp("zabbix_server");
		zabbix_log( LOG_LEVEL_WARNING, "server #%d started [Poller. SNMP:ON]",server_num);
#else
		zabbix_log( LOG_LEVEL_WARNING, "server #%d started [Poller. SNMP:OFF]",server_num);
#endif
		main_poller_loop(ZBX_POLLER_TYPE_NORMAL, server_num);
	}
	else if(server_num <= CONFIG_POLLER_FORKS + CONFIG_TRAPPERD_FORKS)
	{
/* Run trapper processes then do housekeeping */
		if(gethostname(host,127) != 0)
		{
			zabbix_log( LOG_LEVEL_CRIT, "gethostname() failed");
			exit(FAIL);
		}
		child_trapper_main(server_num, listenfd, addrlen);

/*		threads[i] = child_trapper_make(i, listenfd, addrlen); */
/*		child_trapper_make(server_num, listenfd, addrlen); */
	}
	else if(server_num <= CONFIG_POLLER_FORKS + CONFIG_TRAPPERD_FORKS + CONFIG_PINGER_FORKS)
	{
		zabbix_log( LOG_LEVEL_WARNING, "server #%d started [ICMP pinger]",server_num);
		main_pinger_loop(1+server_num-(CONFIG_POLLER_FORKS + CONFIG_TRAPPERD_FORKS + CONFIG_PINGER_FORKS));
	}
	else if(server_num <= CONFIG_POLLER_FORKS + CONFIG_TRAPPERD_FORKS + CONFIG_PINGER_FORKS + CONFIG_ALERTER_FORKS)
	{
		zabbix_log( LOG_LEVEL_WARNING, "server #%d started [Alerter]",server_num);
		main_alerter_loop();
	}
	else if(server_num <= CONFIG_POLLER_FORKS + CONFIG_TRAPPERD_FORKS + CONFIG_PINGER_FORKS + CONFIG_ALERTER_FORKS
			+CONFIG_HOUSEKEEPER_FORKS)
	{
		zabbix_log( LOG_LEVEL_WARNING, "server #%d started [Housekeeper]",server_num);
		main_housekeeper_loop();
	}
	else if(server_num <= CONFIG_POLLER_FORKS + CONFIG_TRAPPERD_FORKS + CONFIG_PINGER_FORKS + CONFIG_ALERTER_FORKS
			+CONFIG_HOUSEKEEPER_FORKS + CONFIG_TIMER_FORKS)
	{
		zabbix_log( LOG_LEVEL_WARNING, "server #%d started [Timer]",server_num);
		main_timer_loop();
	}
	else if(server_num <= CONFIG_POLLER_FORKS + CONFIG_TRAPPERD_FORKS + CONFIG_PINGER_FORKS + CONFIG_ALERTER_FORKS
			+CONFIG_HOUSEKEEPER_FORKS + CONFIG_TIMER_FORKS + CONFIG_UNREACHABLE_POLLER_FORKS)
	{
/*		zabbix_log( LOG_LEVEL_WARNING, "%d<=%d",server_num,  CONFIG_POLLER_FORKS + CONFIG_TRAPPERD_FORKS + CONFIG_PINGER_FORKS + CONFIG_ALERTER_FORKS+CONFIG_HOUSEKEEPER_FORKS + CONFIG_TIMER_FORKS + CONFIG_UNREACHABLE_POLLER_FORKS); */
#ifdef HAVE_SNMP
		init_snmp("zabbix_server");
		zabbix_log( LOG_LEVEL_WARNING, "server #%d started [Poller for unreachable hosts. SNMP:ON]",server_num);
#else
		zabbix_log( LOG_LEVEL_WARNING, "server #%d started [Poller for unreachable hosts. SNMP:OFF]",server_num);
#endif
/*		zabbix_log( LOG_LEVEL_WARNING, "Before main_poller_loop(%d,%d)",ZBX_POLLER_TYPE_UNREACHABLE,server_num - (CONFIG_POLLER_FORKS + CONFIG_TRAPPERD_FORKS + CONFIG_PINGER_FORKS +CONFIG_ALERTER_FORKS+CONFIG_HOUSEKEEPER_FORKS + CONFIG_TIMER_FORKS)); */
		main_poller_loop(ZBX_POLLER_TYPE_UNREACHABLE,
				server_num - (CONFIG_POLLER_FORKS + CONFIG_TRAPPERD_FORKS + CONFIG_PINGER_FORKS + CONFIG_ALERTER_FORKS+CONFIG_HOUSEKEEPER_FORKS + CONFIG_TIMER_FORKS));
	}
	else if(server_num <= CONFIG_POLLER_FORKS + CONFIG_TRAPPERD_FORKS + CONFIG_PINGER_FORKS + CONFIG_ALERTER_FORKS
			+ CONFIG_HOUSEKEEPER_FORKS + CONFIG_TIMER_FORKS + CONFIG_UNREACHABLE_POLLER_FORKS
			+ CONFIG_NODEWATCHER_FORKS)
	{
		zabbix_log( LOG_LEVEL_WARNING, "server #%d started [Node watcher. Node ID:%d]",
				server_num, CONFIG_NODEID);
		main_nodewatcher_loop();
	}

	return SUCCEED;
/*
	if( server_num == 0)
	{
		
		if(gethostname(host,127) != 0)
		{
			zabbix_log( LOG_LEVEL_CRIT, "gethostname() failed");
			exit(FAIL);
		}

		listenfd = tcp_listen(host,CONFIG_LISTEN_PORT,&addrlen);

		for(i = CONFIG_POLLER_FORKS; i < CONFIG_POLLER_FORKS + CONFIG_TRAPPERD_FORKS; i++)
		{
			threads[i] = child_trapper_make(i, listenfd, addrlen);
		}

		zabbix_log( LOG_LEVEL_WARNING, "server #%d started [Housekeeper]",server_num);

		for(i=0; i < CONFIG_POLLER_FORKS + CONFIG_TRAPPERD_FORKS; i++)
		{
				zabbix_log( LOG_LEVEL_DEBUG, "%d. PID=[%d]", i, threads[i]);
		}
		zabbix_log( LOG_LEVEL_CRIT, "ZABBIX server is up.");

		init_main_process();

		main_housekeeper_loop();
	}
	else if(server_num == 1)
	{
		zabbix_log( LOG_LEVEL_WARNING, "server #%d started [Alerter]",server_num);
		main_alerter_loop();
	}
	else if(server_num == 2)
	{
		zabbix_log( LOG_LEVEL_WARNING, "server #%d started [Timer]",server_num);
		main_timer_loop();
	}
	else if(server_num == 3)
	{
		zabbix_log( LOG_LEVEL_WARNING, "server #%d started [ICMP pinger]",server_num);
		main_pinger_loop();
	}
	else if(server_num == 4)
	{
#ifdef HAVE_SNMP
		init_snmp("zabbix_server");
		zabbix_log( LOG_LEVEL_WARNING, "server #%d started [Poller for unreachable hosts. SNMP:ON]",server_num);
#else
		zabbix_log( LOG_LEVEL_WARNING, "server #%d started [Poller for unreachable hosts. SNMP:OFF]",server_num);
#endif
		main_poller_loop(server_num);
	}
	else if(server_num == 5)
	{
		zabbix_log( LOG_LEVEL_WARNING, "server #%d started [Node watcher]",server_num);
		main_nodewatcher_loop();
	}
	else
	{
#ifdef HAVE_SNMP
		init_snmp("zabbix_server");
		zabbix_log( LOG_LEVEL_WARNING, "server #%d started [Poller. SNMP:ON]",server_num);
#else
		zabbix_log( LOG_LEVEL_WARNING, "server #%d started [Poller. SNMP:OFF]",server_num);
#endif

		main_poller_loop(server_num);
	}

	return SUCCEED;*/
}

void	zbx_on_exit()
{
	zabbix_log(LOG_LEVEL_DEBUG, "zbx_on_exit() called.");
	
#if !defined(_WINDOWS)
	
	int i = 0;

	if(threads != NULL)
	{
		for(i = 1; i <= CONFIG_POLLER_FORKS+CONFIG_TRAPPERD_FORKS+CONFIG_PINGER_FORKS+CONFIG_ALERTER_FORKS+CONFIG_HOUSEKEEPER_FORKS+CONFIG_TIMER_FORKS+CONFIG_UNREACHABLE_POLLER_FORKS+CONFIG_NODEWATCHER_FORKS; i++)
		{
			if(threads[i]) {
				kill(threads[i],SIGTERM);
				threads[i] = (ZBX_THREAD_HANDLE)NULL;
			}
		}
	}
	
#endif /* not _WINDOWS */

#ifdef USE_PID_FILE

	daemon_stop();

#endif /* USE_PID_FILE */

	zbx_sleep(2); /* wait for all threads closing */
	
	zabbix_log(LOG_LEVEL_INFORMATION, "ZABBIX Server stopped");
	zabbix_close_log();
	
#ifdef  HAVE_SQLITE3
	zbx_mutex_destroy(&sqlite_access);
#endif /* HAVE_SQLITE3 */

	exit(SUCCEED);
}

