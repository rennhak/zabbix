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

#include "sysinfo.h"
#include "zabbix_agent.h"

#include "cfg.h"
#include "log.h"
#include "zbxconf.h"
#include "zbxgetopt.h"
#include "comms.h"
#include "mutexs.h"
#include "alias.h"

#include "stats.h"
#include "perfstat.h"
#include "active.h"
#include "listener.h"

#include "symbols.h"

#if defined(ZABBIX_SERVICE)
#	include "service.h"
#elif defined(ZABBIX_DAEMON) /* ZABBIX_SERVICE */
#	include "daemon.h"
#endif /* ZABBIX_DAEMON */


char *progname = NULL;

/* Default config file location */
#ifdef _WINDOWS
	static char	DEFAULT_CONFIG_FILE[]	= "C:\\zabbix_agentd.conf";
#else /* not _WINDOWS */
	static char	DEFAULT_CONFIG_FILE[]	= "/etc/zabbix/zabbix_agentd.conf";
#endif /* _WINDOWS */

/* application TITLE */

char title_message[] = APPLICATION_NAME
#if defined(_WIN64)		
				" Win64"
#elif defined(WIN32)		
				" Win32"
#endif /* WIN32 */
#if defined(ZABBIX_SERVICE)	
				" (service)"
#elif defined(ZABBIX_DAEMON)	
				" (daemon)"
#endif /* ZABBIX_SERVICE */
	;
/* end of application TITLE */


/* application USAGE message */

char usage_message[] = 
	"[-Vhp]"
#if defined(_WINDOWS)
	" [-idsx] [-m]"
#endif /* _WINDOWS */
	" [-c <file>] [-t <metric>]";

/*end of application USAGE message */



/* application HELP message */

char *help_message[] = {
	"Options:",
	"",
	"  -c --config <file>    Specify configuration file",
	"  -h --help             give this help",
	"  -V --version          display version number",
	"  -p --print            print supported metrics and exit",
	"  -t --test <metric>    test specified metric and exit",
/*	"  -u --usage <metric> test specified metric and exit",	*/ /* !!! TODO - print metric usage !!! */

#if defined (_WINDOWS)

	"",
	"Functions:",
	"",
	"  -i --install          install ZABICOM agent as service",
	"  -d --uninstall        uninstall ZABICOM agent from service",

	"  -s --start            start ZABICOM agent service",
	"  -x --stop             stop ZABICOM agent service",

	"  -m --multiple-agents  service name will include hostname",

#endif /* _WINDOWS */

	0 /* end of text */
};

/* end of application HELP message */



/* COMMAND LINE OPTIONS */

/* long options */

static struct zbx_option longopts[] =
{
	{"config",		1,	0,	'c'},
	{"help",		0,	0,	'h'},
	{"version",		0,	0,	'V'},
	{"print",		0,	0,	'p'},
	{"test",		1,	0,	't'},

#if defined (_WINDOWS)

	{"install",		0,	0,	'i'},
	{"uninstall",		0,	0,	'd'},

	{"start",		0,	0,	's'},
	{"stop",		0,	0,	'x'},

	{"multiple-agents",	0,	0,	'm'},

#endif /* _WINDOWS */

	{0,0,0,0}
};

/* short options */

static char	shortopts[] = 
	"c:hVpt:"
#if defined (_WINDOWS)
	"idsxm"
#endif /* _WINDOWS */
	;

/* end of COMMAND LINE OPTIONS*/



static char	*TEST_METRIC = NULL;

static ZBX_THREAD_HANDLE	*threads = NULL;

static void parse_commandline(int argc, char **argv, ZBX_TASK_EX *t)
{
	char	ch	= '\0';

	t->task = ZBX_TASK_START;
	
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
		case 'p':
			if(t->task == ZBX_TASK_START)
				t->task = ZBX_TASK_PRINT_SUPPORTED;
			break;
		case 't':
			if(t->task == ZBX_TASK_START) 
			{
				t->task = ZBX_TASK_TEST_METRIC;
				TEST_METRIC = strdup(zbx_optarg);
			}
			break;
#if defined (_WINDOWS)
		case 'i':
			t->task = ZBX_TASK_INSTALL_SERVICE;
			break;
		case 'd':
			t->task = ZBX_TASK_UNINSTALL_SERVICE;
			break;
		case 's':
			t->task = ZBX_TASK_START_SERVICE;
			break;
		case 'x':
			t->task = ZBX_TASK_STOP_SERVICE;
			break;
		case 'm':
			t->flags = ZBX_TASK_FLAG_MULTIPLE_AGENTS;
			break;
#endif /* _WINDOWS */
		default:
			t->task = ZBX_TASK_SHOW_USAGE;
			break;
		}

	if(CONFIG_FILE == NULL)
	{
		CONFIG_FILE = DEFAULT_CONFIG_FILE;
	}
}

int MAIN_ZABBIX_ENTRY(void)
{
	ZBX_THREAD_ACTIVECHK_ARGS	activechk_args;

	int	i = 0;

	zbx_sock_t	listen_sock;

	zabbix_open_log(
#if ON	/* !!! normal case must be ON !!! */
		LOG_TYPE_FILE
#elif OFF	/* !!! normal case must be OFF !!! */
		LOG_TYPE_SYSLOG
#else	/* !!! for debug only, print log with zbx_error !!! */ 
		LOG_TYPE_UNDEFINED
#endif
		,
		CONFIG_LOG_LEVEL,
		CONFIG_LOG_FILE
		);

	zabbix_log(LOG_LEVEL_INFORMATION, "zabbix_agentd started. ZABICOM %s.", ZABBIX_VERSION);
	zabbix_log( LOG_LEVEL_WARNING, "© 2001-2008 by ZABBIX SIA");

	if( FAIL == zbx_tcp_listen(&listen_sock, CONFIG_LISTEN_IP, (unsigned short)CONFIG_LISTEN_PORT) )
	{
		zabbix_log(LOG_LEVEL_CRIT, "Listener failed with error: %s.", zbx_tcp_strerror());
		exit(1);
	}

	init_collector_data();

	/* --- START THREADS ---*/

	threads = calloc(CONFIG_ZABBIX_FORKS, sizeof(ZBX_THREAD_HANDLE));

	threads[i=0] = zbx_thread_start(collector_thread, NULL);

	/* start listeners */
	for(i++; i < CONFIG_ZABBIX_FORKS - ((0 == CONFIG_DISABLE_ACTIVE) ? 1 : 0); i++)
	{
		threads[i] = zbx_thread_start(listener_thread, &listen_sock);
	}

	/* start active chack */
	if(0 == CONFIG_DISABLE_ACTIVE)
	{
		activechk_args.host = CONFIG_HOSTS_ALLOWED;
		activechk_args.port = (unsigned short)CONFIG_SERVER_PORT;

		threads[i] = zbx_thread_start(active_checks_thread, &activechk_args);
	}

	/* Must be called after all child processes loading. */
	init_main_process();

	/* wait for all threads exiting */
	for(i = 0; i < CONFIG_ZABBIX_FORKS; i++)
	{
		if(threads && threads[i])
		{
			zbx_thread_wait(threads[i]);

			if(threads)
				zabbix_log( LOG_LEVEL_DEBUG, "thread [%i] is terminated", i);

			ZBX_DO_EXIT();
		}
	}

	zbx_on_exit();

	return SUCCEED;
}

void	zbx_on_exit()
{

	int i = 0;

	ZBX_DO_EXIT();

	if(threads != NULL)
	{
		for(i = 0; i<CONFIG_ZABBIX_FORKS ; i++)
		{
			if(threads[i]) {
				zbx_thread_kill(threads[i]);
				threads[i] = (ZBX_THREAD_HANDLE)NULL;
			}
		}
	}
	
	zbx_free(threads);
	
	zabbix_log(LOG_LEVEL_DEBUG, "zbx_on_exit() called.");

#ifdef USE_PID_FILE

	daemon_stop();

#endif /* USE_PID_FILE */

	free_metrics();
	free_collector_data();
	alias_list_free();
	perfs_list_free();

	zbx_sleep(2); /* wait for all threads closing */

	zabbix_log(LOG_LEVEL_INFORMATION, "ZABICOM Agent stopped");

	zabbix_close_log();

	exit(SUCCEED);
}

#ifndef ZABBIX_TEST

int	main(int argc, char **argv)
{
	ZBX_TASK_EX	t;

	memset(&t, 0, sizeof(t));
	t.task = ZBX_TASK_START;

	progname = get_programm_name(argv[0]);

	parse_commandline(argc, argv, &t);

	import_symbols();

	init_metrics(); /* Must be before load_config().  load_config - use metrics!!! */

	if (ZBX_TASK_START == t.task || ZBX_TASK_INSTALL_SERVICE == t.task || ZBX_TASK_UNINSTALL_SERVICE == t.task || ZBX_TASK_START_SERVICE == t.task || ZBX_TASK_STOP_SERVICE == t.task)
		load_config();

#if defined (_WINDOWS)
	if (t.flags & ZBX_TASK_FLAG_MULTIPLE_AGENTS) {
		zbx_snprintf(ZABBIX_SERVICE_NAME, sizeof(ZABBIX_SERVICE_NAME), "%s [%s]", APPLICATION_NAME, CONFIG_HOSTNAME);
		zbx_snprintf(ZABBIX_EVENT_SOURCE, sizeof(ZABBIX_EVENT_SOURCE), "%s [%s]", APPLICATION_NAME, CONFIG_HOSTNAME);
	}
#endif /* _WINDOWS */

	load_user_parameters();

	switch (t.task) {
#if defined (_WINDOWS)
		case ZBX_TASK_INSTALL_SERVICE:
			exit(ZabbixCreateService(argv[0]));
			break;
		case ZBX_TASK_UNINSTALL_SERVICE:
			exit(ZabbixRemoveService());
			break;
		case ZBX_TASK_START_SERVICE:
			exit(ZabbixStartService());
			break;
		case ZBX_TASK_STOP_SERVICE:
			exit(ZabbixStopService());
			break;
#endif /* _WINDOWS */
		case ZBX_TASK_PRINT_SUPPORTED:
			test_parameters();
			free_metrics();
			exit(SUCCEED);
			break;
		case ZBX_TASK_TEST_METRIC:
			test_parameter(TEST_METRIC, PROCESS_TEST);
			exit(SUCCEED);
			break;
		case ZBX_TASK_SHOW_USAGE:
			usage();
			exit(FAIL);
			break;
		default:
			/* do nothing */
			break;
	}

	START_MAIN_ZABBIX_ENTRY(CONFIG_ALLOW_ROOT);

	exit(SUCCEED);
}

#else /* ZABBIX_TEST */
/* #	define ENABLE_CHECK_MEMOTY 1 */

#if defined(_WINDOWS)
#	include "messages.h"
#endif /* _WINDOWS */

int main()
{
#if OFF
	int res, val;

	if(FAIL == zbx_sock_init())
	{
		return 1;
	}


	res = check_ntp("142.3.100.15",123,&val);

	zbx_error("check_ntp result '%i' value '%i'", res, val);

#elif OFF

	zbx_error("%s",strerror_from_module(MSG_ZABBIX_MESSAGE, NULL));

#elif OFF
	
	char buffer[100*1024];

	get_http_page("www.zabbix.com", "", 80, buffer, 100*1024);

	printf("Back [%d] [%s]\n", strlen(buffer), buffer);
	
#elif OFF

	char s[] = "ABCDEFGH";
	char p[] = "D(.){0,}E";
	int len=2;

	printf("String: \t %s\n", s);
	printf("Pattern:\t %s\n", p);
	printf("Result: \t [%s] [%d]\n", zbx_regexp_match(s, p, &len), len);
/*
#elif OFF or ON

Place your test code HERE!!!
*/

#endif /* 0 */

	return 0;
}

#endif /* not ZABBIX_TEST */

