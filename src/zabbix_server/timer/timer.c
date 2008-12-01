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

#include "cfg.h"
#include "pid.h"
#include "db.h"
#include "log.h"
#include "zlog.h"
#include "zbxserver.h"

#include "timer.h"

/******************************************************************************
 *                                                                            *
 * Function: main_timer_loop                                                  *
 *                                                                            *
 * Purpose: periodically updates time-related triggers                        *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments: does update once per 30 seconds (hardcoded)                      *
 *                                                                            *
 ******************************************************************************/
void main_timer_loop()
{
	int	now;

/*	int	itemid,functionid;
	char	*function;
	char	*parameter;*/

	DB_ITEM	item;

	DB_RESULT	result;
	DB_ROW	row;

	for (;;) {
		zbx_setproctitle("updating nodata() functions");

		DBconnect(ZBX_DB_CONNECT_NORMAL);

/*
#ifdef HAVE_POSTGRESQL
		zbx_snprintf(sql,sizeof(sql),"select distinct f.itemid,f.functionid,f.parameter from functions f, items i,hosts h where h.hostid=i.hostid and h.status=%d and i.itemid=f.itemid and f.function in ('nodata','date','dayofweek','time','now') and i.lastclock+f.parameter::text::integer<=%d and i.status=%d", HOST_STATUS_MONITORED, now, ITEM_STATUS_ACTIVE);
#else
		zbx_snprintf(sql,sizeof(sql),"select distinct f.itemid,f.functionid,f.parameter,f.function from functions f, items i,hosts h where h.hostid=i.hostid and h.status=%d and i.itemid=f.itemid and f.function in ('nodata','date','dayofweek','time','now') and i.lastclock+f.parameter<=%d and i.status=%d", HOST_STATUS_MONITORED, now, ITEM_STATUS_ACTIVE);
#endif
	*/

		result = DBselect("select distinct %s, functions f where h.hostid=i.hostid and h.status=%d"
				" and i.status=%d and f.function in ('nodata','date','dayofweek','time','now')"
				" and i.itemid=f.itemid" DB_NODE,
				ZBX_SQL_ITEM_SELECT,
				HOST_STATUS_MONITORED,
				ITEM_STATUS_ACTIVE,
				DBnode_local("h.hostid"));

		while (NULL != (row = DBfetch(result))) {
			DBget_item_from_db(&item,row);

			now = (int)time(NULL);

			DBbegin();
			update_functions(&item, now);
			update_triggers(item.itemid);
			DBcommit();
		}

		DBfree_result(result);
		DBclose();

		zbx_setproctitle("sleeping for 30 sec");

		sleep(30);
	}
}
