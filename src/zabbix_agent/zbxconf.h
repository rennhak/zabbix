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

#ifndef ZABBIX_ZBXCONF_H
#define ZABBIX_ZBXCONF_H

extern char	*CONFIG_HOSTS_ALLOWED;
extern char	*CONFIG_HOSTNAME;
extern char	*CONFIG_FILE;
extern char	*CONFIG_PID_FILE;
extern char	*CONFIG_LOG_FILE;
extern char	*CONFIG_STAT_FILE;
extern char	*CONFIG_STAT_FILE_TMP;
extern int	CONFIG_AGENTD_FORKS;
/* extern int	CONFIG_NOTIMEWAIT;		*/
extern int	CONFIG_DISABLE_ACTIVE;
extern int	CONFIG_ENABLE_REMOTE_COMMANDS;
extern int	CONFIG_TIMEOUT;
extern int	CONFIG_LISTEN_PORT;
extern int	CONFIG_SERVER_PORT;
extern int	CONFIG_REFRESH_ACTIVE_CHECKS;
extern char	*CONFIG_LISTEN_IP;
extern int	CONFIG_LOG_LEVEL;

void    load_config(void);
void    load_user_parameters(void);

#endif /* ZABBIX_ZBXCONF_H */
