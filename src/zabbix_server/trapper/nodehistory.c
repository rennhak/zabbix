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


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>

#include <string.h>

#include <time.h>

#include <sys/socket.h>
#include <errno.h>

/* Functions: pow(), round() */
#include <math.h>

#include "common.h"
#include "db.h"
#include "log.h"
#include "zlog.h"

#include "nodeevents.h"

extern int     process_event(DB_EVENT *event);

/******************************************************************************
 *                                                                            *
 * Function: process_record                                                   *
 *                                                                            *
 * Purpose: process record update                                             *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:  SUCCEED - processed successfully                            *
 *                FAIL - an error occured                                     *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int	process_record(int nodeid, char *record)
{
	char	tmp[MAX_STRING_LEN];
	int		table;
	zbx_uint64_t	itemid;
	int		timestamp;
	double		value;
	zbx_uint64_t	value_uint;
	int		res = FAIL;

	zabbix_log( LOG_LEVEL_DEBUG, "In process_record [%s]", record);

	zbx_get_field(record,tmp,0,'|');
	table=atoi(tmp);
	if(table == ZBX_TABLE_HISTORY)
	{
		zbx_get_field(record,tmp,1,'|');
		sscanf(tmp,ZBX_FS_UI64,&itemid);
		zbx_get_field(record,tmp,2,'|');
		timestamp=atoi(tmp);
		zbx_get_field(record,tmp,3,'|');
		value=atof(tmp);

		res =  DBadd_history(itemid, value, timestamp);
	}
	else if(table == ZBX_TABLE_HISTORY_UINT)
	{
		zbx_get_field(record,tmp,1,'|');
		sscanf(tmp,ZBX_FS_UI64,&itemid);
		zbx_get_field(record,tmp,2,'|');
		timestamp=atoi(tmp);
		zbx_get_field(record,tmp,3,'|');
		sscanf(tmp,ZBX_FS_UI64,&value_uint);

		res =  DBadd_history_uint(itemid, value_uint, timestamp);
	}
	else if(table == ZBX_TABLE_HISTORY_STR)
	{
		zbx_get_field(record,tmp,1,'|');
		sscanf(tmp,ZBX_FS_UI64,&itemid);
		zbx_get_field(record,tmp,2,'|');
		timestamp=atoi(tmp);
		zbx_get_field(record,tmp,3,'|');

		res =  DBadd_history_str(itemid, tmp, timestamp);
	}

	return res;
}

/******************************************************************************
 *                                                                            *
 * Function: node_events                                                      *
 *                                                                            *
 * Purpose: process new events received from a salve node                     *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:  SUCCEED - processed successfully                            *
 *                FAIL - an error occured                                     *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
int	node_history(char *data)
{
	char	*s;
	int	firstline=1;
	int	nodeid=0;
	int	sender_nodeid=0;
	char	tmp[MAX_STRING_LEN];

//	zabbix_log( LOG_LEVEL_WARNING, "In node_history(len:%d)", strlen(data));

       	s=(char *)strtok(data,"\n");
	while(s!=NULL)
	{
		if(firstline == 1)
		{
//			zabbix_log( LOG_LEVEL_WARNING, "First line [%s]", s);
			zbx_get_field(s,tmp,1,'|');
			sender_nodeid=atoi(tmp);
			zbx_get_field(s,tmp,2,'|');
			nodeid=atoi(tmp);
			firstline=0;
			zabbix_log( LOG_LEVEL_WARNING, "NODE %d: Received history from node %d for node %d", CONFIG_NODEID, sender_nodeid, nodeid);
		}
		else
		{
//			zabbix_log( LOG_LEVEL_WARNING, "Got line [%s]", s);
			process_record(nodeid, s);
		}

       		s=(char *)strtok(NULL,"\n");
	}
	return SUCCEED;
}
