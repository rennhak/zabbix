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
#include "security.h"
#include "zabbix_agent.h"

#include "pid.h"
#include "log.h"
#include "zbxconf.h"

#include "stats.h"
#include "active.h"

#define	LISTENQ 1024

char *progname = NULL;
char title_message[] = "ZABBIX Agent (daemon)";
char usage_message[] = "[-vhp] [-c <file>] [-t <metric>]";

#ifndef HAVE_GETOPT_LONG

	char *help_message[] = {
		"Options:",
		"  -c <file>    Specify configuration file",
		"  -h           give this help",
		"  -v           display version number",
		"  -p           print supported metrics and exit",
		"  -t <metric>  test specified metric and exit",
		0 /* end of text */
	};

#else /* not HAVE_GETOPT_LONG */

	char *help_message[] = {
		"Options:",
		"  -c --config <file>  Specify configuration file",
		"  -h --help           give this help",
		"  -v --version        display version number",
		"  -p --print          print supported metrics and exit",
		"  -t --test <metric>  test specified metric and exit",
		0 /* end of text */
	};

#endif /* HAVE_GETOPT_LONG */

struct option longopts[] =
{
	{"config",	1,	0,	'c'},
	{"help",	0,	0,	'h'},
	{"version",	0,	0,	'v'},
	{"print",	0,	0,	'p'},
	{"test",	1,	0,	't'},
	{0,0,0,0}
};

static char	*TEST_METRIC = NULL;

static int parse_commandline(int argc, char **argv)
{
	int	task	= ZBX_TASK_START;
	char	ch	= '\0';

	/* Parse the command-line. */
	while ((ch = getopt_long(argc, argv, "c:hvpt:", longopts, NULL)) != EOF)
		switch ((char) ch) {
		case 'c':
			CONFIG_FILE = optarg;
			break;
		case 'h':
			help();
			exit(-1);
			break;
		case 'v':
			version();
			exit(-1);
			break;
		case 'p':
			if(task == ZBX_TASK_START)
				task = ZBX_TASK_PRINT_SUPPORTED;
			break;
		case 't':
			if(task == ZBX_TASK_START) 
			{
				task = ZBX_TASK_TEST_METRIC;
				TEST_METRIC = optarg;
			}
			break;
		default:
			task = ZBX_TASK_SHOW_USAGE;
			break;
	}

	return task;
}

void	init_log(void)
{
	if(CONFIG_LOG_FILE == NULL)
		zabbix_open_log(LOG_TYPE_SYSLOG,CONFIG_LOG_LEVEL,NULL);
	else
		zabbix_open_log(LOG_TYPE_FILE,CONFIG_LOG_LEVEL,CONFIG_LOG_FILE);

}

static ZBX_SOCKET connect_to_server(void)
{
	ZBX_SOCKET sock;
	zbx_sockaddr serv_addr;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		zabbix_log( LOG_LEVEL_CRIT, "Unable to create socket");
//		WriteLog(MSG_SOCKET_ERROR,EVENTLOG_ERROR_TYPE,"e",WSAGetLastError());
		LOG_DEBUG_INFO("s", "End of ListenerThread() Error: 1");
		exit(1);
	}

	// Create socket
	// Fill in local address structure
	memset(&serv_addr, 0, sizeof(zbx_sockaddr));

	serv_addr.sin_family		= AF_INET;
	serv_addr.sin_addr.s_addr	= CONFIG_LISTEN_IP ? inet_addr(CONFIG_LISTEN_IP) : htonl(INADDR_ANY);
	serv_addr.sin_port		= htons(CONFIG_LISTEN_PORT);

	// Bind socket
	if (bind(sock,&serv_addr,sizeof(zbx_sockaddr)) == SOCKET_ERROR)
	{
		zabbix_log(LOG_LEVEL_CRIT, "Cannot bind to port %d. Error [%s]. Another zabbix_agentd already running ?",
				port, strerror(errno));
//		WriteLog(MSG_BIND_ERROR,EVENTLOG_ERROR_TYPE,"e",WSAGetLastError());
		exit(1);
	}

	if(listen(sock, 
#if defined(WIN32)
		LISTENQ
#else /* WIN32 */
		SOMAXCONN
#endif
		) == SOCKET_ERROR)
	{
		zabbix_log( LOG_LEVEL_CRIT, "Listen failed");
//		WriteLog(MSG_LISTEN_ERROR,EVENTLOG_ERROR_TYPE,"e",WSAGetLastError());
		exit(1);
	}

	return sock;
}

void MAIN_ZABBIX_EVENT_LOOP(void)
{
	ZBX_THREAD_HANDLE		*threads;
	ZBX_THREAD_ACTIVECHK_ARGS	activechk_args;

	int	i = 0;

	ZBX_SOCKET	sock;

	init_log();	

	zabbix_log( LOG_LEVEL_WARNING, "zabbix_agentd started. ZABBIX %s.", ZABBIX_VERSION);

	sock = connect_to_server();

	/* --- START THREADS ---*/
	threads = calloc(CONFIG_AGENTD_FORKS, sizeof(ZBX_THREAD_HANDLE));

	/* start collector */
	SemColectorStarted = zbx_semaphore_create();

	threads[i] = zbx_thread_start(CollectorThread, &SemColectorStarted);

	zbx_semaphore_wait(&SemColectorStarted);
	zbx_semaphore_destr(&SemColectorStarted);

	/* start listeners */
	for(; i < CONFIG_AGENTD_FORKS-1; i++)
	{
		threads[i] = zbx_thread_start(ListenerThread, &sock);
	}

	/* start active chack */
	if(CONFIG_DISABLE_ACTIVE==0)
	{
		activechk_args.host = CONFIG_HOSTS_ALLOWED;
		activechk_args.port = CONFIG_SERVER_PORT;

		threads[i] = zbx_thread_start(ActiveChecksThread, &activechk_args);
	}


	/* wait for exit */
	for(i = 0; i < CONFIG_AGENTD_FORKS; i++)
	{
		if(zbx_thread_wait(threads[i]))
			WriteLog(MSG_INFORMATION,EVENTLOG_INFORMATION_TYPE,"ds", tid[i], ": Listen thread is Terminated.");
	}
}

int	main(int argc, char **argv)
{
	ZBX_SEM_HANDLE SemColectorStarted;
	int	task = ZBX_TASK_START;

	progname = argv[0];

	task = parse_commandline(argc, argv);

	init_metrics(); // Must be before init_config() !!!

	load_config();

	load_user_parameters();
	
	switch(task)
	{
		case ZBX_TASK_PRINT_SUPPORTED:
			test_parameters();
			exit(SUCCEED);
			break;
		case ZBX_TASK_TEST_METRIC:
			test_parameter(TEST_METRIC);
			exit(SUCCEED);
			break;
		case ZBX_TASK_SHOW_USAGE:
			usage();
			exit(FAIL);
			break;
	}

#if defined(WIN32)
	init_service();
#else /* not WIN32 */
	init_daemon();
#endif /* WIN32 */

	//WriteLog(MSG_AGENT_SHUTDOWN, EVENTLOG_INFORMATION_TYPE,NULL);

	return SUCCEED;
}
