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
#include "zbxconf.h"

#include "cfg.h"
#include "log.h"
#include "alias.h"
#include "zbxplugin.h"
#include "sysinfo.h"


#ifdef WIN32

static char	DEFAULT_CONFIG_FILE[]	= "C:\\zabbix_agentd.conf";
static char	*DEFAULT_PID_FILE	= NULL;
static char	DEFAULT_STAT_FILE[]	= "C:\\zabbix_agentd.stt";

#else /* not WIN32 */

static char	DEFAULT_CONFIG_FILE[]	= "/etc/zabbix/zabbix_agentd.conf";
static char	DEFAULT_PID_FILE[]	= "/tmp/zabbix_agentd.pid";
static char	DEFAULT_STAT_FILE[]	= "/tmp/zabbix_agentd.stt";

#endif /* WIN32 */


char	*CONFIG_HOSTS_ALLOWED		= NULL;
char	*CONFIG_HOSTNAME		= NULL;
char	*CONFIG_STAT_FILE		= NULL;
/* int	CONFIG_NOTIMEWAIT		= 0; */
int	CONFIG_DISABLE_ACTIVE		= 0;
int	CONFIG_ENABLE_REMOTE_COMMANDS	= 0;
int	CONFIG_LISTEN_PORT	= 10050;
int	CONFIG_SERVER_PORT	= 10051;
int	CONFIG_REFRESH_ACTIVE_CHECKS	= 120;
char	*CONFIG_LISTEN_IP		= NULL;
int	CONFIG_LOG_LEVEL		= LOG_LEVEL_INFORMATION;
char	CONFIG_LOG_UNRES_SYMB		= 0;


void    load_config(void)
{
	struct cfg_line cfg[]=
	{
/*               PARAMETER      ,VAR    ,FUNC,  TYPE(0i,1s), MANDATORY, MIN, MAX
*/
		{"Server",		&CONFIG_HOSTS_ALLOWED,	0,TYPE_STRING,	PARM_MAND,	0,0},
		{"Hostname",		&CONFIG_HOSTNAME,	0,TYPE_STRING,	PARM_OPT,	0,0},

#ifdef USE_PID_FILE
		{"PidFile",		&CONFIG_PID_FILE,	0,TYPE_STRING,	PARM_OPT,	0,0},
#endif /* USE_PID_FILE */

		{"LogFile",		&CONFIG_LOG_FILE,	0,TYPE_STRING,	PARM_OPT,	0,0},
		{"StatFile",		&CONFIG_STAT_FILE,	0,TYPE_STRING,	PARM_OPT,	0,0},
		{"DisableActive",	&CONFIG_DISABLE_ACTIVE,	0,TYPE_INT,	PARM_OPT,	0,1},
		{"Timeout",		&CONFIG_TIMEOUT,	0,TYPE_INT,	PARM_OPT,	1,30},
/*		{"NoTimeWait",		&CONFIG_NOTIMEWAIT,	0,TYPE_INT,	PARM_OPT,	0,1},*/
		{"ListenPort",		&CONFIG_LISTEN_PORT,	0,TYPE_INT,	PARM_OPT,	1024,32767},
		{"ServerPort",		&CONFIG_SERVER_PORT,	0,TYPE_INT,	PARM_OPT,	1024,32767},
		{"ListenIP",		&CONFIG_LISTEN_IP,	0,TYPE_STRING,	PARM_OPT,	0,0},

		{"DebugLevel",		&CONFIG_LOG_LEVEL,	0,TYPE_INT,	PARM_OPT,	0,5},

		{"StartAgents",		&CONFIG_AGENTD_FORKS,		0,TYPE_INT,	PARM_OPT,	1,16},
		{"RefreshActiveChecks",	&CONFIG_REFRESH_ACTIVE_CHECKS,	0,TYPE_INT,	PARM_OPT,60,3600},
		{"EnableRemoteCommands",&CONFIG_ENABLE_REMOTE_COMMANDS,	0,TYPE_INT,	PARM_OPT,0,1},
		{"AllowRootPermission",	&CONFIG_ALLOW_ROOT_PERMISSION,	0,TYPE_INT,	PARM_OPT,0,1},
		
//		{"PerfCounter",		&CONFIG_PERF_COUNTER,		0,	TYPE_STRING,PARM_OPT,0,0},
//		{"CollectorTimeout",	&CONFIG_COLLECTOR_TIMEOUT,	0,	TYPE_STRING,PARM_OPT,0,0},
		{"LogUnresolvedSymbols",&CONFIG_LOG_UNRES_SYMB,			0,	TYPE_STRING,PARM_OPT,0,1},

		{"Alias",		0,	&add_alias_from_config,	TYPE_STRING,PARM_OPT,0,0},
		
#if defined(ZABBIX_DAEMON)
		{"Plugin",		0,	&add_plugin,	TYPE_STRING,PARM_OPT,0,0},
#endif /* ZABBIX_DAEMON */
		
		{0}
	};

	AGENT_RESULT	result;	

	memset(&result, 0, sizeof(AGENT_RESULT));
	
	if(CONFIG_FILE == NULL)
	{
		CONFIG_FILE = DEFAULT_CONFIG_FILE;
	}

	parse_cfg_file(CONFIG_FILE,cfg);

#ifdef USE_PID_FILE
	if(CONFIG_PID_FILE == NULL)
	{
		CONFIG_PID_FILE = DEFAULT_PID_FILE;
	}
#endif /* USE_PID_FILE */
	
	if(CONFIG_STAT_FILE == NULL)
	{
		CONFIG_STAT_FILE = DEFAULT_STAT_FILE;
	}

	if(CONFIG_HOSTNAME == NULL)
	{
	  	if(SUCCEED == process("system.hostname", 0, &result))
		{
	        	if(result.type & AR_STRING)
			{
				CONFIG_HOSTNAME=strdup(result.str);
			}
		}
	        free_result(&result);

		if(CONFIG_HOSTNAME == NULL)
		{
			zabbix_log( LOG_LEVEL_CRIT, "Hostname is not defined");
			exit(1);
		}
	}

/*	if(CONFIG_STAT_FILE == NULL)
	{
		CONFIG_STAT_FILE=strdup("/tmp/zabbix_agentd.tmp");
	}*/
}



static int     add_parameter(char *key)
{
	char    *command;

	command = strstr(key,",");
	if(NULL == command)
	{
		return  FAIL;
	}
	command[0]=0;
	command++;
	add_user_parameter(key, command);
	return  SUCCEED;
}

void    load_user_parameters(void)
{
	struct cfg_line cfg[]=
	{
/*               PARAMETER,		VAR,	FUNC,		TYPE(0i,1s), MANDATORY,MIN,MAX
*/
		{"UserParameter",	0,	&add_parameter,	0,	0,	0,	0},
		{0}
	};
	
	parse_cfg_file(CONFIG_FILE,cfg);
}
