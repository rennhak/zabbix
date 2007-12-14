/* 
** ZABBIX
** Copyright (C) 2000-2006 SIA Zabbix
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

#include "cfg.h"
#include "db.h"
#include "log.h"
#include "zlog.h"

#include "nodecomms.h"
#include "nodesender.h"
#include "events.h"

/******************************************************************************
 *                                                                            *
 * Function: process_node                                                     *
 *                                                                            *
 * Purpose: select all related nodes and send config changes                  *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value: SUCCESS - processed succesfully                              * 
 *               FAIL - an error occured                                      *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int process_node(int nodeid, int master_nodeid, zbx_uint64_t event_lastid)
{
	DB_RESULT	result;
	DB_ROW		row;
	char		*data = NULL;
	int		found = 0;

	int		offset = 0;
	int		allocated = 1024;

	zbx_uint64_t	eventid;

	zabbix_log( LOG_LEVEL_DEBUG, "In process_node(local:%d, event_lastid:" ZBX_FS_UI64 ")",
		nodeid,
		event_lastid);
	/* Begin work */

	data = zbx_malloc(data, allocated);
	memset(data,0,allocated);

	zbx_snprintf_alloc(&data, &allocated, &offset, 128, "Events%c%d%c%d",
		ZBX_DM_DELIMITER,
		CONFIG_NODEID,
		ZBX_DM_DELIMITER,
		nodeid);

	result = DBselect("select eventid,source,object,objectid,clock,ms,value,acknowledged from events where eventid>" ZBX_FS_UI64 " and " ZBX_COND_NODEID " order by eventid",
		event_lastid,
		ZBX_NODE("eventid", nodeid));
	while((row=DBfetch(result)))
	{
		ZBX_STR2UINT64(eventid,row[0])
		found = 1;
		zbx_snprintf_alloc(&data, &allocated, &offset, 1024, "\n%s%c%s%c%s%c%s%c%s%c%s%c%s%c%s",
				row[0], ZBX_DM_DELIMITER,	/* eventid */
				row[1], ZBX_DM_DELIMITER,	/* source */
				row[2], ZBX_DM_DELIMITER,	/* object */
				row[3], ZBX_DM_DELIMITER,	/* objectid */
				row[4], ZBX_DM_DELIMITER,	/* clock */
				row[5], ZBX_DM_DELIMITER,	/* ms */
				row[6], ZBX_DM_DELIMITER,	/* value */
				row[7]);			/* acknowledged */
	}
	if(found == 1)
	{
		zabbix_log( LOG_LEVEL_DEBUG, "Sending [%s]",
			data);
		/* Do not send events for current node if CONFIG_NODE_NOEVENTS is set */
		if( ((CONFIG_NODE_NOEVENTS !=0) && (CONFIG_NODEID == nodeid)) ||
			send_to_node("new events", master_nodeid, nodeid, data) == SUCCEED)
		{
			zabbix_log( LOG_LEVEL_DEBUG, "Updating nodes.event_lastid");
			DBexecute("update nodes set event_lastid=" ZBX_FS_UI64 " where nodeid=%d",
				eventid,
				nodeid);
		}
		else
		{
			zabbix_log( LOG_LEVEL_DEBUG, "Not updating nodes.event_lastid");
		}
	}
	DBfree_result(result);
	zbx_free(data);

	return SUCCEED;
}

/******************************************************************************
 *                                                                            *
 * Function: main_eventsender                                                 *
 *                                                                            *
 * Purpose: periodically sends new events to master node                      *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              * 
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments: never returns                                                    *
 *                                                                            *
 ******************************************************************************/
void main_eventsender()
{
	DB_RESULT	result;
	DB_ROW		row;
	zbx_uint64_t	lastid;
	int		nodeid;
	int		master_nodeid;

	zabbix_log( LOG_LEVEL_DEBUG, "In main_eventsender()");

	DBbegin();

	master_nodeid = CONFIG_MASTER_NODEID;

	if(master_nodeid != 0)
	{
		result = DBselect("select nodeid,event_lastid from nodes");

		while((row = DBfetch(result)))
		{
			nodeid=atoi(row[0]);
			ZBX_STR2UINT64(lastid,row[1])

			process_node(nodeid, master_nodeid, lastid);
		}
		DBfree_result(result);
	}

	DBcommit();
}
