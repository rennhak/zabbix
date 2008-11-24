/* 
** ZABBIX
** Copyright (C) 2000-2007 SIA Zabbix
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
#include "log.h"
#include "zlog.h"
#include "threads.h"

#include "db.h"
#include "dbcache.h"
#include "mutexs.h"
#include "zbxserver.h"

#define	LOCK_CACHE	zbx_mutex_lock(&cache_lock)
#define	UNLOCK_CACHE	zbx_mutex_unlock(&cache_lock)

#define ZBX_GET_SHM_DBCACHE_KEY(smk_key) 								\
	{if( -1 == (shm_key = ftok(CONFIG_FILE, (int)'c') )) 						\
        { 												\
                zbx_error("Can not create IPC key for path '%s', try to create for path '.' [%s]",	\
				CONFIG_FILE, strerror(errno)); 						\
                if( -1 == (shm_key = ftok(".", (int)'c') )) 						\
                { 											\
                        zbx_error("Can not create IPC key for path '.' [%s]", strerror(errno)); 	\
                        exit(1); 									\
                } 											\
        }}

ZBX_DC_CACHE		*cache = NULL;
static ZBX_MUTEX	cache_lock;

static char		*sql = NULL;
static int		sql_allocated = 65536;

zbx_process_t		zbx_process;

extern int		CONFIG_DBSYNCER_FREQUENCY;

/******************************************************************************
 *                                                                            *
 * Function: DCget_trend_nearestindex                                         *
 *                                                                            *
 * Purpose: find nearest index by itemid in array of ZBX_DC_TREND             *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Alekasander Vladishev                                              *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int	DCget_trend_nearestindex(zbx_uint64_t itemid)
{
	int	first_index, last_index, index;

	if (cache->trends_num == 0)
		return 0;

	first_index = 0;
	last_index = cache->trends_num - 1;
	while (1)
	{
		index = first_index + (last_index - first_index) / 2;

		if (cache->trends[index].itemid == itemid)
			return index;
		else if (last_index == first_index)
		{
			if (cache->trends[index].itemid < itemid)
				index++;
			return index;
		}
		else if (cache->trends[index].itemid < itemid)
			first_index = index + 1;
		else
			last_index = index;
	}
}

/******************************************************************************
 *                                                                            *
 * Function: DCget_trend                                                      *
 *                                                                            *
 * Purpose: find existing or add new structure and return pointer             *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value: pointer to a new structure or NULL if array is full          *
 *                                                                            *
 * Author: Alekasander Vladishev                                              *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static ZBX_DC_TREND	*DCget_trend(zbx_uint64_t itemid)
{
	int	index;

	index = DCget_trend_nearestindex(itemid);
	if (index < cache->trends_num && cache->trends[index].itemid == itemid)
		return &cache->trends[index];

	if (cache->trends_num == ZBX_TREND_SIZE)
		return NULL;

	memmove(&cache->trends[index + 1], &cache->trends[index], sizeof(ZBX_DC_TREND) * (cache->trends_num - index));
	memset(&cache->trends[index], 0, sizeof(ZBX_DC_TREND));
	cache->trends[index].itemid = itemid;
	cache->trends_num++;

	return &cache->trends[index];
}

/******************************************************************************
 *                                                                            *
 * Function: DCflush_trend                                                    *
 *                                                                            *
 * Purpose: flush trend to the database                                       *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Alekasander Vladishev                                              *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static void	DCflush_trend(ZBX_DC_TREND *trend, int *sql_offset)
{
	DB_RESULT	result;
	DB_ROW		row;
	int		num;
	history_value_t	value_min, value_avg, value_max;

	switch (trend->value_type)
	{
		case ITEM_VALUE_TYPE_FLOAT:
			result = DBselect("select num,value_min,value_avg,value_max from trends"
					" where itemid=" ZBX_FS_UI64 " and clock=%d",
					trend->itemid,
					trend->clock);
			break;
		case ITEM_VALUE_TYPE_UINT64:
			result = DBselect("select num,value_min,value_avg,value_max from trends_uint"
					" where itemid=" ZBX_FS_UI64 " and clock=%d",
					trend->itemid,
					trend->clock);
			break;
		default:
			zabbix_log(LOG_LEVEL_CRIT, "Invalid value type for trends.");
			exit(-1);
	}

	if (NULL != (row = DBfetch(result)))
	{
		num = atoi(row[0]);
		switch (trend->value_type)
		{
			case ITEM_VALUE_TYPE_FLOAT:
				value_min.value_float = atof(row[1]);
				value_avg.value_float = atof(row[2]);
				value_max.value_float = atof(row[3]);

				if (value_min.value_float < trend->value_min.value_float)
					trend->value_min.value_float = value_min.value_float;
				if (value_max.value_float > trend->value_max.value_float)
					trend->value_max.value_float = value_max.value_float;
				trend->value_avg.value_float = (trend->num * trend->value_avg.value_float
						+ num * value_avg.value_float) / (trend->num + num);
				trend->num += num;

				zbx_snprintf_alloc(&sql, &sql_allocated, sql_offset, 512,
						"update trends set num=%d,value_min=" ZBX_FS_DBL ",value_avg=" ZBX_FS_DBL
						",value_max=" ZBX_FS_DBL " where itemid=" ZBX_FS_UI64 " and clock=%d;\n",
						trend->num,
						trend->value_min.value_float,
						trend->value_avg.value_float,
						trend->value_max.value_float,
						trend->itemid,
						trend->clock);
				break;
			case ITEM_VALUE_TYPE_UINT64:
				value_min.value_uint64 = zbx_atoui64(row[1]);
				value_avg.value_uint64 = zbx_atoui64(row[2]);
				value_max.value_uint64 = zbx_atoui64(row[3]);

				if (value_min.value_uint64 < trend->value_min.value_uint64)
					trend->value_min.value_uint64 = value_min.value_uint64;
				if (value_max.value_uint64 > trend->value_max.value_uint64)
					trend->value_max.value_uint64 = value_max.value_uint64;
				trend->value_avg.value_uint64 = (trend->num * trend->value_avg.value_uint64
						+ num * value_avg.value_uint64) / (trend->num + num);
				trend->num += num;
				
				zbx_snprintf_alloc(&sql, &sql_allocated, sql_offset, 512,
						"update trends_uint set num=%d,value_min=" ZBX_FS_UI64 ",value_avg=" ZBX_FS_UI64
						",value_max=" ZBX_FS_UI64 " where itemid=" ZBX_FS_UI64 " and clock=%d;\n",
						trend->num,
						trend->value_min.value_uint64,
						trend->value_avg.value_uint64,
						trend->value_max.value_uint64,
						trend->itemid,
						trend->clock);
				break;
		}
	}
	else
	{
		switch (trend->value_type)
		{
			case ITEM_VALUE_TYPE_FLOAT:
				zbx_snprintf_alloc(&sql, &sql_allocated, sql_offset, 512,
						"insert into trends (itemid,clock,num,value_min,value_avg,value_max)"
						" values (" ZBX_FS_UI64 ",%d,%d," ZBX_FS_DBL "," ZBX_FS_DBL "," ZBX_FS_DBL ");\n",
						trend->itemid,
						trend->clock,
						trend->num,
						trend->value_min.value_float,
						trend->value_avg.value_float,
						trend->value_max.value_float);
				break;
			case ITEM_VALUE_TYPE_UINT64:
				zbx_snprintf_alloc(&sql, &sql_allocated, sql_offset, 512,
						"insert into trends_uint (itemid,clock,num,value_min,value_avg,value_max)"
						" values (" ZBX_FS_UI64 ",%d,%d," ZBX_FS_UI64 "," ZBX_FS_UI64 "," ZBX_FS_UI64 ");\n",
						trend->itemid,
						trend->clock,
						trend->num,
						trend->value_min.value_uint64,
						trend->value_avg.value_uint64,
						trend->value_max.value_uint64);
				break;
		}
	}
	DBfree_result(result);

	trend->clock = 0;
	trend->num = 0;
	memset(&trend->value_min, 0, sizeof(history_value_t));
	memset(&trend->value_avg, 0, sizeof(history_value_t));
	memset(&trend->value_max, 0, sizeof(history_value_t));
}

/******************************************************************************
 *                                                                            *
 * Function: DCadd_trend                                                      *
 *                                                                            *
 * Purpose: add new value to the trends                                       *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Alekasander Vladishev                                              *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static void	DCadd_trend(ZBX_DC_HISTORY *history, int *sql_offset)
{
	ZBX_DC_TREND	*trend = NULL, trend_static;
	int		hour;

	hour = history->clock - history->clock % 3600;
	
	if (NULL != (trend = DCget_trend(history->itemid)))
	{
		if (trend->num > 0 && (trend->clock != hour || trend->value_type != history->value_type))
			DCflush_trend(trend, sql_offset);

		trend->value_type	= history->value_type;
		trend->clock		= hour;

		switch (trend->value_type)
		{
			case ITEM_VALUE_TYPE_FLOAT:
				if (trend->num == 0 || history->value.value_float < trend->value_min.value_float)
					trend->value_min.value_float = history->value.value_float;
				if (trend->num == 0 || history->value.value_float > trend->value_max.value_float)
					trend->value_max.value_float = history->value.value_float;
				trend->value_avg.value_float = (trend->num * trend->value_avg.value_float
						+ history->value.value_float) / (trend->num + 1);
				trend->num++;
				break;
			case ITEM_VALUE_TYPE_UINT64:
				if (trend->num == 0 || history->value.value_uint64 < trend->value_min.value_uint64)
					trend->value_min.value_uint64 = history->value.value_uint64;
				if (trend->num == 0 || history->value.value_uint64 > trend->value_max.value_uint64)
					trend->value_max.value_uint64 = history->value.value_uint64;
				trend->value_avg.value_uint64 = (trend->num * trend->value_avg.value_uint64
						+ history->value.value_uint64) / (trend->num + 1);
				trend->num++;
				break;
		}
	}
	else
	{
		zabbix_log(LOG_LEVEL_WARNING, "Insufficient space for trends. Flushing to disk.");

		trend_static.itemid = history->itemid;
		trend_static.clock = hour;
		trend_static.value_type = history->value_type;
		trend_static.num = 1;
		switch (trend_static.value_type)
		{
			case ITEM_VALUE_TYPE_FLOAT:
				trend_static.value_min.value_float = history->value.value_float;
				trend_static.value_avg.value_float = history->value.value_float;
				trend_static.value_max.value_float = history->value.value_float;
				break;
			case ITEM_VALUE_TYPE_UINT64:
				trend_static.value_min.value_uint64 = history->value.value_uint64;
				trend_static.value_avg.value_uint64 = history->value.value_uint64;
				trend_static.value_max.value_uint64 = history->value.value_uint64;
				break;
		}

		DCflush_trend(trend, sql_offset);
	}
}

/******************************************************************************
 *                                                                            *
 * Function: DCmass_update_trends                                             *
 *                                                                            *
 * Purpose:                                                                   *
 *                                                                            *
 * Parameters: history - array of history data                                *
 *             history_num - number of history structures                     *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Aleksander Vladishev                                               *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static void	DCmass_update_trends(ZBX_DC_HISTORY *history, int history_num)
{
	int	sql_offset = 0, i;

	zabbix_log(LOG_LEVEL_DEBUG, "In DCmass_update_trends()");

#ifdef HAVE_ORACLE
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 8, "begin\n");
#endif

	for (i = 0; i < history_num; i++)
	{
		if (0 == history[i].keep_trends)
			continue;

		if (history[i].value_type != ITEM_VALUE_TYPE_FLOAT && history[i].value_type != ITEM_VALUE_TYPE_UINT64)
			continue;

		if (0 != history[i].value_null)
			continue;

		DCadd_trend(&history[i], &sql_offset);
	}

#ifdef HAVE_ORACLE
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 8, "end;\n");
#endif

	if (sql_offset > 16) /* In ORACLE always present begin..end; */
		DBexecute("%s", sql);
}

/******************************************************************************
 *                                                                            *
 * Function: DCsync_trends                                                    *
 *                                                                            *
 * Purpose: flush all trends to the database                                  *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Alekasander Vladishev                                              *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
void	DCsync_trends()
{
	int	sql_offset = 0, i;

	zabbix_log(LOG_LEVEL_DEBUG, "In DCsync_trends(trends_num: %d)",
			cache->trends_num);
	
#ifdef HAVE_ORACLE
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 8, "begin\n");
#endif

	for (i = 0; i < cache->trends_num; i ++)
		DCflush_trend(&cache->trends[i], &sql_offset);

#ifdef HAVE_ORACLE
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 8, "end;\n");
#endif

	if (sql_offset > 16) /* In ORACLE always present begin..end; */
	{
		DBbegin();
		DBexecute("%s", sql);
		DBcommit();
	}

	cache->trends_num = 0;

	zabbix_log(LOG_LEVEL_DEBUG, "End of DCsync_trends()");
}

/******************************************************************************
 *                                                                            *
 * Function: DCmass_update_triggers                                           *
 *                                                                            *
 * Purpose: re-calculate and updates values of triggers related to the items  *
 *                                                                            *
 * Parameters: history - array of history data                                *
 *             history_num - number of history structures                     *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Alexei Vladishev, Aleksander Vladishev                             *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static void	DCmass_update_triggers(ZBX_DC_HISTORY *history, int history_num)
{
	char		*exp;
	char		error[MAX_STRING_LEN];
	int		exp_value;
	DB_TRIGGER	trigger;
	DB_RESULT	result;
	DB_ROW		row;
	int		sql_offset = 0, i;
	ZBX_DC_HISTORY	*h;
	zbx_uint64_t	itemid;

	zabbix_log(LOG_LEVEL_DEBUG, "In DCmass_update_triggers()");

	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 1024,
			"select distinct t.triggerid,t.expression,t.description,t.url,t.comments,t.status,t.value,t.priority"
			",t.type,f.itemid from triggers t,functions f,items i where i.status not in (%d) and i.itemid=f.itemid"
			" and t.status=%d and f.triggerid=t.triggerid and f.itemid in (",
			ITEM_STATUS_NOTSUPPORTED,
			TRIGGER_STATUS_ENABLED);

	for (i = 0; i < history_num; i++)
		if (0 == history[i].value_null)
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 32, ZBX_FS_UI64 ",",
					history[i].itemid);

	if (sql[sql_offset - 1] == ',')
	{
		sql_offset--;
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 32, ")");
	}

	result = DBselect("%s", sql);

	sql_offset = 0;

	while (NULL != (row = DBfetch(result)))
	{
		trigger.triggerid	= zbx_atoui64(row[0]);
		strscpy(trigger.expression, row[1]);
		strscpy(trigger.description, row[2]);
		trigger.url		= row[3];
		trigger.comments	= row[4];
		trigger.status		= atoi(row[5]);
		trigger.value		= atoi(row[6]);
		trigger.priority	= atoi(row[7]);
		trigger.type		= atoi(row[8]);
		itemid			= zbx_atoui64(row[9]);

		h = NULL;

		for (i = 0; i < history_num; i++)
		{
			if (itemid == history[i].itemid)
			{
				h = &history[i];
				break;
			}
		}

		if (NULL == h)
			continue;

		exp = strdup(trigger.expression);

		if (evaluate_expression(&exp_value, &exp, &trigger, error, sizeof(error)) != 0)
		{
			zabbix_log(LOG_LEVEL_WARNING, "Expression [%s] cannot be evaluated [%s]",
					trigger.expression,
					error);
			zabbix_syslog("Expression [%s] cannot be evaluated [%s]",
					trigger.expression,
					error);
/*			We shouldn't update triggervalue if expressions failed */
/*			DBupdate_trigger_value(&trigger, exp_value, time(NULL), error);*/
		}
		else
			DBupdate_trigger_value(&trigger, exp_value, h->clock, NULL);

		zbx_free(exp);
	}
	DBfree_result(result);
}

/******************************************************************************
 *                                                                            *
 * Function: DCmass_update_item                                               *
 *                                                                            *
 * Purpose: update items info after new values is received                    *
 *                                                                            *
 * Parameters: history - array of history data                                *
 *             history_num - number of history structures                     *
 *                                                                            *
 * Author: Alexei Vladishev, Eugene Grigorjev, Aleksander Vladishev           *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static void	DCmass_update_item(ZBX_DC_HISTORY *history, int history_num)
{
	DB_RESULT	result;
	DB_ROW		row;
	DB_ITEM		item;
	char		value_esc[ITEM_LASTVALUE_LEN_MAX];
	int		sql_offset = 0, i;
	ZBX_DC_HISTORY	*h;

	zabbix_log( LOG_LEVEL_DEBUG, "In DCmass_update_item()");

	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 1024,
			"select %s where h.hostid = i.hostid and i.itemid in (",
			ZBX_SQL_ITEM_SELECT,
			TRIGGER_STATUS_ENABLED);

	for (i = 0; i < history_num; i++)
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 32, ZBX_FS_UI64 ",",
				history[i].itemid);

	if (sql[sql_offset - 1] == ',')
	{
		sql_offset--;
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 32, ")");
	}

	result = DBselect("%s", sql);

	sql_offset = 0;

#ifdef HAVE_ORACLE
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 8, "begin\n");
#endif

	while (NULL != (row = DBfetch(result)))
	{
		DBget_item_from_db(&item, row);

		h = NULL;

		for (i = 0; i < history_num; i++)
		{
			if (item.itemid == history[i].itemid)
			{
				h = &history[i];
				break;
			}
		}

		if (NULL == h)
			continue;

		if (zbx_process == ZBX_PROCESS_PROXY)
		{
			item.delta = ITEM_STORE_AS_IS;
			h->keep_history = 1;
			h->keep_trends = 0;
		}
		else
		{
			h->keep_history = item.history;
			h->keep_trends = item.trends;
		}

		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 128, "update items set lastclock=%d",
				h->clock);

		switch (h->value_type) {
		case ITEM_VALUE_TYPE_FLOAT:
			switch (item.delta) {
			case ITEM_STORE_AS_IS:
				h->value.value_float = h->value_orig.value_float;
				zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
						",prevvalue=lastvalue,lastvalue='" ZBX_FS_DBL "'",
						h->value_orig.value_float);
				break;
			case ITEM_STORE_SPEED_PER_SECOND:
				if (item.prevorgvalue_null == 0 && item.prevorgvalue_dbl <= h->value_orig.value_float && item.lastclock < h->clock)
				{
					h->value.value_float = (h->value_orig.value_float - item.prevorgvalue_dbl) / (h->clock - item.lastclock);
					zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
							",prevvalue=lastvalue,prevorgvalue='" ZBX_FS_DBL "'"
							",lastvalue='" ZBX_FS_DBL "'",
							h->value_orig.value_float,
							h->value.value_float);
				}
				else
				{
					zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
							",prevorgvalue='" ZBX_FS_DBL "'",
							h->value_orig.value_float);
					h->value_null = 1;
				}
				break;
			case ITEM_STORE_SIMPLE_CHANGE:
				if (item.prevorgvalue_null == 0 && item.prevorgvalue_dbl <= h->value_orig.value_float)
				{
					h->value.value_float = h->value_orig.value_float - item.prevorgvalue_dbl;
					zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
							",prevvalue=lastvalue,prevorgvalue='" ZBX_FS_DBL "'"
							",lastvalue='" ZBX_FS_DBL "'",
							h->value_orig.value_float,
							h->value.value_float);
				}
				else
				{
					zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
							",prevorgvalue='" ZBX_FS_DBL "'",
							h->value_orig.value_float);
					h->value_null = 1;
				}
				break;
			}
			break;
		case ITEM_VALUE_TYPE_UINT64:
			switch (item.delta) {
			case ITEM_STORE_AS_IS:
				h->value.value_uint64 = h->value_orig.value_uint64;
				zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
						",prevvalue=lastvalue,lastvalue='" ZBX_FS_UI64 "'",
						h->value_orig.value_uint64);
				break;
			case ITEM_STORE_SPEED_PER_SECOND:
				if (item.prevorgvalue_null == 0 && item.prevorgvalue_uint64 <= h->value_orig.value_uint64 && item.lastclock < h->clock)
				{
					h->value.value_uint64 = (h->value_orig.value_uint64 - item.prevorgvalue_uint64) / (h->clock - item.lastclock);
					zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
							",prevvalue=lastvalue,prevorgvalue='" ZBX_FS_UI64 "'"
							",lastvalue='" ZBX_FS_UI64 "'",
							h->value_orig.value_uint64,
							h->value.value_uint64);
				}
				else
				{
					zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
							",prevorgvalue='" ZBX_FS_DBL "'",
							h->value_orig.value_uint64);
					h->value_null = 1;
				}
				break;
			case ITEM_STORE_SIMPLE_CHANGE:
				if (item.prevorgvalue_null == 0 && item.prevorgvalue_uint64 <= h->value_orig.value_uint64)
				{
					h->value.value_uint64 = h->value_orig.value_uint64 - item.prevorgvalue_uint64;
					zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
							",prevvalue=lastvalue,prevorgvalue='" ZBX_FS_UI64 "'"
							",lastvalue='" ZBX_FS_UI64 "'",
							h->value_orig.value_uint64,
							h->value.value_uint64);
				}
				else
				{
					zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
							",prevorgvalue='" ZBX_FS_UI64 "'",
							h->value_orig.value_uint64);
					h->value_null = 1;
				}
				break;
			}
			break;
		case ITEM_VALUE_TYPE_STR:
		case ITEM_VALUE_TYPE_TEXT:
			DBescape_string(h->value_orig.value_str, value_esc, sizeof(value_esc));
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
					",prevvalue=lastvalue,lastvalue='%s'",
					value_esc);
			break;
		case ITEM_VALUE_TYPE_LOG:
			DBescape_string(h->value_orig.value_str, value_esc, sizeof(value_esc));
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
					",prevvalue=lastvalue,lastvalue='%s',lastlogsize=%d",
					value_esc,
					h->lastlogsize);
			break;
		}

		/* Update item status if required */
		if (item.status == ITEM_STATUS_NOTSUPPORTED)
		{
			zabbix_log(LOG_LEVEL_WARNING, "Parameter [%s] became supported by agent on host [%s]",
					item.key,
					item.host_name);
			zabbix_syslog("Parameter [%s] became supported by agent on host [%s]",
					item.key,
					item.host_name);

			item.status = ITEM_STATUS_ACTIVE;
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 32, ",status=%d,error=''",
					item.status);
		}

		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 128, " where itemid=" ZBX_FS_UI64 ";\n",
				item.itemid);
	}
	DBfree_result(result);

#ifdef HAVE_ORACLE
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 8, "end;\n");
#endif

	if (sql_offset > 16) /* In ORACLE always present begin..end; */
		DBexecute("%s", sql);
}

/******************************************************************************
 *                                                                            *
 * Function: DCmass_proxy_update_item                                         *
 *                                                                            *
 * Purpose: update items info after new values is received                    *
 *                                                                            *
 * Parameters: history - array of history data                                *
 *             history_num - number of history structures                     *
 *                                                                            *
 * Author: Alexei Vladishev, Eugene Grigorjev, Aleksander Vladishev           *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static void	DCmass_proxy_update_item(ZBX_DC_HISTORY *history, int history_num)
{
	int	sql_offset = 0, i;

	zabbix_log( LOG_LEVEL_DEBUG, "In DCmass_proxy_update_item()");

#ifdef HAVE_ORACLE
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 8, "begin\n");
#endif

	for (i = 0; i < history_num; i++)
	{
		if (history[i].value_type == ITEM_VALUE_TYPE_LOG)
		{
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 128,
					"update items set lastlogsize=%d where itemid=" ZBX_FS_UI64 ";\n",
					history[i].lastlogsize,
					history[i].itemid);
		}
	}

#ifdef HAVE_ORACLE
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 8, "end;\n");
#endif

	if (sql_offset > 16) /* In ORACLE always present begin..end; */
		DBexecute("%s", sql);
}

/******************************************************************************
 *                                                                            *
 * Function: DCmass_function_update                                           *
 *                                                                            *
 * Purpose: update functions lastvalue after new values is received           *
 *                                                                            *
 * Parameters: history - array of history data                                *
 *             history_num - number of history structures                     *
 *                                                                            *
 * Author: Alexei Vladishev, Aleksander Vladishev                             *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static void DCmass_function_update(ZBX_DC_HISTORY *history, int history_num)
{
	DB_RESULT	result;
	DB_ROW		row;
	DB_FUNCTION	function;
	DB_ITEM		item;
	char		*lastvalue;
	char		value[MAX_STRING_LEN], *value_esc, *parameter_esc;
	int		sql_offset = 0, i;

	zabbix_log(LOG_LEVEL_DEBUG, "In DCmass_function_update()");

	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 1024,
			"select distinct %s,f.function,f.parameter,f.itemid,f.lastvalue from %s,functions f,triggers t"
			" where f.itemid=i.itemid and h.hostid=i.hostid and f.triggerid=t.triggerid and t.status in (%d)"
			" and f.itemid in (",
			ZBX_SQL_ITEM_FIELDS,
			ZBX_SQL_ITEM_TABLES,
			TRIGGER_STATUS_ENABLED);

	for (i = 0; i < history_num; i++)
		if (0 == history[i].value_null)
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 32, ZBX_FS_UI64 ",",
					history[i].itemid);

	if (sql[sql_offset - 1] == ',')
	{
		sql_offset--;
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 32, ")");
	}

	result = DBselect("%s", sql);

	sql_offset = 0;

#ifdef HAVE_ORACLE
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 8, "begin\n");
#endif

	while (NULL != (row = DBfetch(result)))
	{
		DBget_item_from_db(&item, row);

		function.function	= row[ZBX_SQL_ITEM_FIELDS_NUM];
		function.parameter	= row[ZBX_SQL_ITEM_FIELDS_NUM + 1];
		function.itemid		= zbx_atoui64(row[ZBX_SQL_ITEM_FIELDS_NUM + 2]);
/*		It is not required to check lastvalue for NULL here */
		lastvalue		= row[ZBX_SQL_ITEM_FIELDS_NUM + 3];

		if (FAIL == evaluate_function(value, &item, function.function, function.parameter))
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Evaluation failed for function:%s",
					function.function);
			continue;
		}

		/* Update only if lastvalue differs from new one */
		if (DBis_null(lastvalue) == SUCCEED || strcmp(lastvalue, value) != 0)
		{
			value_esc = DBdyn_escape_string(value);
			parameter_esc = DBdyn_escape_string(function.parameter);

			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 1024,
					"update functions set lastvalue='%s' where itemid=" ZBX_FS_UI64
					" and function='%s' and parameter='%s';\n",
					value_esc,
					function.itemid,
					function.function,
					parameter_esc);

			zbx_free(parameter_esc);
			zbx_free(value_esc);
		}
	}
	DBfree_result(result);

#ifdef HAVE_ORACLE
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 8, "end;\n");
#endif

	if (sql_offset > 16) /* In ORACLE always present begin..end; */
		DBexecute("%s", sql);
}

/******************************************************************************
 *                                                                            *
 * Function: DCmass_add_history                                               *
 *                                                                            *
 * Purpose: inserting new history data after new values is received           *
 *                                                                            *
 * Parameters: history - array of history data                                *
 *             history_num - number of history structures                     *
 *                                                                            *
 * Author: Aleksander Vladishev                                               *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static void	DCmass_add_history(ZBX_DC_HISTORY *history, int history_num)
{
	int		sql_offset = 0, i;
	char		value_esc[MAX_STRING_LEN], *value_esc_dyn;
	int		history_text_num, history_log_num;
	zbx_uint64_t	id;
#ifdef HAVE_MYSQL
	int		tmp_offset;
#endif

	zabbix_log(LOG_LEVEL_DEBUG, "In DCmass_add_history()");

#ifdef HAVE_ORACLE
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 8, "begin\n");
#endif

/*
 * history
 */
#ifdef HAVE_MYSQL
	tmp_offset = sql_offset;
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
			"insert into history (itemid,clock,value) values ");
#endif

	for (i = 0; i < history_num; i++)
	{
		if (0 == history[i].keep_history)
			continue;

		if (history[i].value_type != ITEM_VALUE_TYPE_FLOAT)
			continue;

		if (0 != history[i].value_null)
			continue;

#ifdef HAVE_MYSQL
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
				"(" ZBX_FS_UI64 ",%d," ZBX_FS_DBL "),",
				history[i].itemid,
				history[i].clock,
				history[i].value.value_float);
#else
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
				"insert into history (itemid,clock,value) values "
				"(" ZBX_FS_UI64 ",%d," ZBX_FS_DBL ");\n",
				history[i].itemid,
				history[i].clock,
				history[i].value.value_float);
#endif
	}

#ifdef HAVE_MYSQL
	if (sql[sql_offset - 1] == ',')
	{
		sql_offset--;
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 4, ";\n");
	}
	else
		sql_offset = tmp_offset;
#endif

	if (CONFIG_NODE_NOHISTORY == 0 && CONFIG_MASTER_NODEID > 0)
	{
#ifdef HAVE_MYSQL
		tmp_offset = sql_offset;
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
				"insert into history_sync (nodeid,itemid,clock,value) values ");
#endif

		for (i = 0; i < history_num; i++)
		{
			if (0 == history[i].keep_history)
				continue;

			if (history[i].value_type != ITEM_VALUE_TYPE_FLOAT)
				continue;

			if (0 != history[i].value_null)
				continue;

#ifdef HAVE_MYSQL
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
					"(%d," ZBX_FS_UI64 ",%d," ZBX_FS_DBL "),",
					get_nodeid_by_id(history[i].itemid),
					history[i].itemid,
					history[i].clock,
					history[i].value.value_float);
#else
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
					"insert into history_sync (nodeid,itemid,clock,value) values "
					"(%d," ZBX_FS_UI64 ",%d," ZBX_FS_DBL ");\n",
					get_nodeid_by_id(history[i].itemid),
					history[i].itemid,
					history[i].clock,
					history[i].value.value_float);
#endif
		}

#ifdef HAVE_MYSQL
		if (sql[sql_offset - 1] == ',')
		{
			sql_offset--;
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 4, ";\n");
		}
		else
			sql_offset = tmp_offset;
#endif
	}

/*
 * history_uint
 */
#ifdef HAVE_MYSQL
	tmp_offset = sql_offset;
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
			"insert into history_uint (itemid,clock,value) values ");
#endif

	for (i = 0; i < history_num; i++)
	{
		if (0 == history[i].keep_history)
			continue;

		if (history[i].value_type != ITEM_VALUE_TYPE_UINT64)
			continue;

		if (0 != history[i].value_null)
			continue;

#ifdef HAVE_MYSQL
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
				"(" ZBX_FS_UI64 ",%d," ZBX_FS_UI64 "),",
				history[i].itemid,
				history[i].clock,
				history[i].value.value_uint64);
#else
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
				"insert into history_uint (itemid,clock,value) values "
				"(" ZBX_FS_UI64 ",%d," ZBX_FS_UI64 ");\n",
				history[i].itemid,
				history[i].clock,
				history[i].value.value_uint64);
#endif
	}

#ifdef HAVE_MYSQL
	if (sql[sql_offset - 1] == ',')
	{
		sql_offset--;
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 4, ";\n");
	}
	else
		sql_offset = tmp_offset;
#endif

	if (CONFIG_NODE_NOHISTORY == 0 && CONFIG_MASTER_NODEID > 0)
	{
#ifdef HAVE_MYSQL
		tmp_offset = sql_offset;
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
				"insert into history_uint_sync (nodeid,itemid,clock,value) values ");
#endif

		for (i = 0; i < history_num; i++)
		{
			if (0 == history[i].keep_history)
				continue;

			if (history[i].value_type != ITEM_VALUE_TYPE_UINT64)
				continue;

			if (0 != history[i].value_null)
				continue;

#ifdef HAVE_MYSQL
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
					"(%d," ZBX_FS_UI64 ",%d," ZBX_FS_UI64 "),",
					get_nodeid_by_id(history[i].itemid),
					history[i].itemid,
					history[i].clock,
					history[i].value.value_uint64);
#else
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
					"insert into history_uint_sync (nodeid,itemid,clock,value) values "
					"(%d," ZBX_FS_UI64 ",%d," ZBX_FS_UI64 ");\n",
					get_nodeid_by_id(history[i].itemid),
					history[i].itemid,
					history[i].clock,
					history[i].value.value_uint64);
#endif
		}

#ifdef HAVE_MYSQL
		if (sql[sql_offset - 1] == ',')
		{
			sql_offset--;
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 4, ";\n");
		}
		else
			sql_offset = tmp_offset;
#endif
	}

/*
 * history_str
 */
#ifdef HAVE_MYSQL
	tmp_offset = sql_offset;
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
			"insert into history_str (itemid,clock,value) values ");
#endif

	for (i = 0; i < history_num; i++)
	{
		if (0 == history[i].keep_history)
			continue;

		if (history[i].value_type != ITEM_VALUE_TYPE_STR)
			continue;

		if (0 != history[i].value_null)
			continue;

		DBescape_string(history[i].value_orig.value_str, value_esc, sizeof(value_esc));
#ifdef HAVE_MYSQL
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
				"(" ZBX_FS_UI64 ",%d,'%s'),",
				history[i].itemid,
				history[i].clock,
				value_esc);
#else
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
				"insert into history_str (itemid,clock,value) values "
				"(" ZBX_FS_UI64 ",%d,'%s');\n",
				history[i].itemid,
				history[i].clock,
				value_esc);
#endif
	}

#ifdef HAVE_MYSQL
	if (sql[sql_offset - 1] == ',')
	{
		sql_offset--;
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 4, ";\n");
	}
	else
		sql_offset = tmp_offset;
#endif

	if (CONFIG_NODE_NOHISTORY == 0 && CONFIG_MASTER_NODEID > 0)
	{
#ifdef HAVE_MYSQL
		tmp_offset = sql_offset;
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
				"insert into history_str_sync (nodeid,itemid,clock,value) values ");
#endif

		for (i = 0; i < history_num; i++)
		{
			if (0 == history[i].keep_history)
				continue;

			if (history[i].value_type != ITEM_VALUE_TYPE_STR)
				continue;

			if (0 != history[i].value_null)
				continue;

			DBescape_string(history[i].value_orig.value_str, value_esc, sizeof(value_esc));
#ifdef HAVE_MYSQL
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
					"(%d," ZBX_FS_UI64 ",%d,'%s'),",
					get_nodeid_by_id(history[i].itemid),
					history[i].itemid,
					history[i].clock,
					value_esc);
#else
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
					"insert into history_str_sync (nodeid,itemid,clock,value) values "
					"(%d," ZBX_FS_UI64 ",%d,'%s');\n",
					get_nodeid_by_id(history[i].itemid),
					history[i].itemid,
					history[i].clock,
					value_esc);
#endif
		}

#ifdef HAVE_MYSQL
		if (sql[sql_offset - 1] == ',')
		{
			sql_offset--;
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 4, ";\n");
		}
		else
			sql_offset = tmp_offset;
#endif
	}

	history_text_num = 0;
	history_log_num = 0;

	for (i = 0; i < history_num; i++)
		if (history[i].value_type == ITEM_VALUE_TYPE_TEXT)
			history_text_num++;
		else if (history[i].value_type == ITEM_VALUE_TYPE_LOG)
			history_log_num++;

/*
 * history_text
 */
	if (history_text_num > 0)
	{
		id = DBget_maxid_num("history_text", "id", history_text_num);

#ifdef HAVE_MYSQL
		tmp_offset = sql_offset;
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
				"insert into history_text (id,itemid,clock,value) values ");
#endif

		for (i = 0; i < history_num; i++)
		{
			if (0 == history[i].keep_history)
				continue;

			if (history[i].value_type != ITEM_VALUE_TYPE_TEXT)
				continue;

			if (0 != history[i].value_null)
				continue;

			value_esc_dyn = DBdyn_escape_string(history[i].value_orig.value_str);
#ifdef HAVE_MYSQL
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512 + strlen(value_esc_dyn),
					"(" ZBX_FS_UI64 "," ZBX_FS_UI64 ",%d,'%s'),",
					id,
					history[i].itemid,
					history[i].clock,
					value_esc_dyn);
#else
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512 + strlen(value_esc_dyn),
					"insert into history_text (id,itemid,clock,value) values "
					"(" ZBX_FS_UI64 "," ZBX_FS_UI64 ",%d,'%s');\n",
					id,
					history[i].itemid,
					history[i].clock,
					value_esc_dyn);
#endif
			zbx_free(value_esc_dyn);
			id++;
		}

#ifdef HAVE_MYSQL
		if (sql[sql_offset - 1] == ',')
		{
			sql_offset--;
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 4, ";\n");
		}
		else
			sql_offset = tmp_offset;
#endif
	}

/*
 * history_log
 */
	if (history_log_num > 0)
	{
		id = DBget_maxid_num("history_log", "id", history_log_num);

#ifdef HAVE_MYSQL
		tmp_offset = sql_offset;
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
				"insert into history_log (id,itemid,clock,timestamp,source,severity,value) values ");
#endif

		for (i = 0; i < history_num; i++)
		{
			if (0 == history[i].keep_history)
				continue;

			if (history[i].value_type != ITEM_VALUE_TYPE_LOG)
				continue;

			if (0 != history[i].value_null)
				continue;

			DBescape_string(history[i].source, value_esc, sizeof(value_esc));
			value_esc_dyn = DBdyn_escape_string(history[i].value_orig.value_str);
#ifdef HAVE_MYSQL
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512 + strlen(value_esc_dyn),
					"(" ZBX_FS_UI64 "," ZBX_FS_UI64 ",%d,%d,'%s',%d,'%s'),",
					id,
					history[i].itemid,
					history[i].clock,
					history[i].timestamp,
					value_esc,
					history[i].severity,
					value_esc_dyn);
#else
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512 + strlen(value_esc_dyn),
					"insert into history_log (id,itemid,clock,timestamp,source,severity,value) values "
					"(" ZBX_FS_UI64 "," ZBX_FS_UI64 ",%d,%d,'%s',%d,'%s');\n",
					id,
					history[i].itemid,
					history[i].clock,
					history[i].timestamp,
					value_esc,
					history[i].severity,
					value_esc_dyn);
#endif
			zbx_free(value_esc_dyn);
			id++;
		}

#ifdef HAVE_MYSQL
		if (sql[sql_offset - 1] == ',')
		{
			sql_offset--;
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 4, ";\n");
		}
		else
			sql_offset = tmp_offset;
#endif
	}

#ifdef HAVE_MYSQL
	sql[sql_offset] = '\0';
#endif

#ifdef HAVE_ORACLE
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 8, "end;\n");
#endif

	if (sql_offset > 16) /* In ORACLE always present begin..end; */
		DBexecute("%s", sql);
}

/******************************************************************************
 *                                                                            *
 * Function: DCmass_proxy_add_history                                         *
 *                                                                            *
 * Purpose: inserting new history data after new values is received           *
 *                                                                            *
 * Parameters: history - array of history data                                *
 *             history_num - number of history structures                     *
 *                                                                            *
 * Author: Aleksander Vladishev                                               *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static void	DCmass_proxy_add_history(ZBX_DC_HISTORY *history, int history_num)
{
	int		sql_offset = 0, i;
	char		value_esc[MAX_STRING_LEN], *value_esc_dyn;
#ifdef HAVE_MYSQL
	int		tmp_offset;
#endif

	zabbix_log(LOG_LEVEL_DEBUG, "In DCmass_proxy_add_history()");

#ifdef HAVE_ORACLE
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 8, "begin\n");
#endif

#ifdef HAVE_MYSQL
	tmp_offset = sql_offset;
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
			"insert into proxy_history (itemid,clock,value) values ");
#endif

	for (i = 0; i < history_num; i++)
	{
		if (history[i].value_type == ITEM_VALUE_TYPE_FLOAT)
		{
#ifdef HAVE_MYSQL
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
					"(" ZBX_FS_UI64 ",%d,'" ZBX_FS_DBL "'),",
					history[i].itemid,
					history[i].clock,
					history[i].value_orig.value_float);
#else
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
					"insert into proxy_history (itemid,clock,value) values "
					"(" ZBX_FS_UI64 ",%d,'" ZBX_FS_DBL "');\n",
					history[i].itemid,
					history[i].clock,
					history[i].value_orig.value_float);
#endif
		}
	}

#ifdef HAVE_MYSQL
	if (sql[sql_offset - 1] == ',')
	{
		sql_offset--;
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 4, ";\n");
	}
	else
		sql_offset = tmp_offset;
#endif

#ifdef HAVE_MYSQL
	tmp_offset = sql_offset;
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
			"insert into proxy_history (itemid,clock,value) values ");
#endif

	for (i = 0; i < history_num; i++)
	{
		if (history[i].value_type == ITEM_VALUE_TYPE_UINT64)
		{
#ifdef HAVE_MYSQL
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
					"(" ZBX_FS_UI64 ",%d,'" ZBX_FS_UI64 "'),",
					history[i].itemid,
					history[i].clock,
					history[i].value_orig.value_uint64);
#else
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
					"insert into proxy_history (itemid,clock,value) values "
					"(" ZBX_FS_UI64 ",%d,'" ZBX_FS_UI64 "');\n",
					history[i].itemid,
					history[i].clock,
					history[i].value_orig.value_uint64);
#endif
		}
	}

#ifdef HAVE_MYSQL
	if (sql[sql_offset - 1] == ',')
	{
		sql_offset--;
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 4, ";\n");
	}
	else
		sql_offset = tmp_offset;
#endif

#ifdef HAVE_MYSQL
	tmp_offset = sql_offset;
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
			"insert into proxy_history (itemid,clock,value) values ");
#endif

	for (i = 0; i < history_num; i++)
	{
		if (history[i].value_type == ITEM_VALUE_TYPE_STR)
		{
			DBescape_string(history[i].value_orig.value_str, value_esc, sizeof(value_esc));
#ifdef HAVE_MYSQL
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
					"(" ZBX_FS_UI64 ",%d,'%s'),",
					history[i].itemid,
					history[i].clock,
					value_esc);
#else
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
					"insert into proxy_history (itemid,clock,value) values " 
					"(" ZBX_FS_UI64 ",%d,'%s');\n",
					history[i].itemid,
					history[i].clock,
					value_esc);
#endif
		}
	}

#ifdef HAVE_MYSQL
	if (sql[sql_offset - 1] == ',')
	{
		sql_offset--;
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 4, ";\n");
	}
	else
		sql_offset = tmp_offset;
#endif

#ifdef HAVE_MYSQL
	tmp_offset = sql_offset;
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
			"insert into proxy_history (itemid,clock,value) values ");
#endif

	for (i = 0; i < history_num; i++)
	{
		if (history[i].value_type == ITEM_VALUE_TYPE_TEXT)
		{
			value_esc_dyn = DBdyn_escape_string(history[i].value_orig.value_str);
#ifdef HAVE_MYSQL
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512 + strlen(value_esc_dyn),
					"(" ZBX_FS_UI64 ",%d,'%s'),",
					history[i].itemid,
					history[i].clock,
					value_esc_dyn);
#else
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512 + strlen(value_esc_dyn),
					"insert into proxy_history (itemid,clock,value) values "
					"(" ZBX_FS_UI64 ",%d,'%s');\n",
					history[i].itemid,
					history[i].clock,
					value_esc_dyn);
#endif
			zbx_free(value_esc_dyn);
		}
	}

#ifdef HAVE_MYSQL
	if (sql[sql_offset - 1] == ',')
	{
		sql_offset--;
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 4, ";\n");
	}
	else
		sql_offset = tmp_offset;
#endif

#ifdef HAVE_MYSQL
	tmp_offset = sql_offset;
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
				"insert into proxy_history (itemid,clock,timestamp,source,severity,value) values ");
#endif

	for (i = 0; i < history_num; i++)
	{
		if (history[i].value_type == ITEM_VALUE_TYPE_LOG)
		{
			DBescape_string(history[i].source, value_esc, sizeof(value_esc));
			value_esc_dyn = DBdyn_escape_string(history[i].value_orig.value_str);
#ifdef HAVE_MYSQL
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512 + strlen(value_esc_dyn),
					"(" ZBX_FS_UI64 ",%d,%d,'%s',%d,'%s'),",
					history[i].itemid,
					history[i].clock,
					history[i].timestamp,
					value_esc,
					history[i].severity,
					value_esc_dyn);
#else
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512 + strlen(value_esc_dyn),
					"insert into proxy_history (itemid,clock,timestamp,source,severity,value) values "
					"(" ZBX_FS_UI64 ",%d,%d,'%s',%d,'%s');\n",
					history[i].itemid,
					history[i].clock,
					history[i].timestamp,
					value_esc,
					history[i].severity,
					value_esc_dyn);
#endif
			zbx_free(value_esc_dyn);
		}
	}

#ifdef HAVE_MYSQL
	if (sql[sql_offset - 1] == ',')
	{
		sql_offset--;
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 4, ";\n");
	}
	else
		sql_offset = tmp_offset;
#endif

#ifdef HAVE_MYSQL
	sql[sql_offset] = '\0';
#endif

#ifdef HAVE_ORACLE
	zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 8, "end;\n");
#endif

	if (sql_offset > 16) /* In ORACLE always present begin..end; */
		DBexecute("%s", sql);
}

static int DCitem_already_exists(ZBX_DC_HISTORY *history, int history_num, zbx_uint64_t itemid)
{
	int	i;

	for (i = 0; i < history_num; i++)
		if (itemid == history[i].itemid)
			return SUCCEED;

	return FAIL;
}

/******************************************************************************
 *                                                                            *
 * Function: DCsync                                                           *
 *                                                                            *
 * Purpose: writes updates and new data from pool to database                 *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value: number of synced values                                      *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
int	DCsync_history(int sync_type)
{
	static ZBX_DC_HISTORY	history[ZBX_SYNC_MAX];
	int			i, j, history_num, n, f;
	int			syncs;
	int			total_num = 0;
	int			skipped_clock, max_delay;

	zabbix_log(LOG_LEVEL_DEBUG, "In DCsync_history(history_first:%d history_num:%d)",
			cache->history_first,
			cache->history_num);

	if (0 == cache->history_num)
		return 0;

	syncs = cache->history_num / ZBX_SYNC_MAX;
	max_delay = (int)time(NULL) - CONFIG_DBSYNCER_FREQUENCY;

	do
	{
		LOCK_CACHE;

		history_num = 0;
		n = cache->history_num;
		f = cache->history_first;
		skipped_clock = 0;

		while (n > 0 && history_num < ZBX_SYNC_MAX)
		{
			if (zbx_process == ZBX_PROCESS_PROXY || FAIL == DCitem_already_exists(history, history_num, cache->history[f].itemid))
			{
				memcpy(&history[history_num], &cache->history[f], sizeof(ZBX_DC_HISTORY));
				if (history[history_num].value_type == ITEM_VALUE_TYPE_STR
						|| history[history_num].value_type == ITEM_VALUE_TYPE_TEXT
						|| history[history_num].value_type == ITEM_VALUE_TYPE_LOG)
				{
					history[history_num].value_orig.value_str = strdup(cache->history[f].value_orig.value_str);

					if (history[history_num].value_type == ITEM_VALUE_TYPE_LOG)
					{
						if (NULL != cache->history[f].source)
							history[history_num].source = strdup(cache->history[f].source);
						else
							history[history_num].source = NULL;
					}
				}

				for (j = f; j != cache->history_first; j = (j == 0 ? ZBX_HISTORY_SIZE : j) - 1)
				{
					i = (j == 0 ? ZBX_HISTORY_SIZE : j) - 1;
					memcpy(&cache->history[j], &cache->history[i], sizeof(ZBX_DC_HISTORY));
				}

				cache->history_num--;
				cache->history_first++;
				cache->history_first = cache->history_first % ZBX_HISTORY_SIZE;

				history_num++;
			}
			else if (skipped_clock == 0)
				skipped_clock = cache->history[f].clock;

			n--;
			f++;
			f = f % ZBX_HISTORY_SIZE;
		}

		UNLOCK_CACHE;

		if (0 == history_num)
			break;

		DBbegin();

		if (zbx_process == ZBX_PROCESS_SERVER)
		{
			DCmass_update_item(history, history_num);
			DCmass_add_history(history, history_num);
			DCmass_function_update(history, history_num);
			DCmass_update_triggers(history, history_num);
			DCmass_update_trends(history, history_num);
		}
		else
		{
			DCmass_proxy_add_history(history, history_num);
			DCmass_proxy_update_item(history, history_num);
		}

		DBcommit();

		for (i = 0; i < history_num; i ++)
		{
			if (history[i].value_type == ITEM_VALUE_TYPE_STR
					|| history[i].value_type == ITEM_VALUE_TYPE_TEXT
					|| history[i].value_type == ITEM_VALUE_TYPE_LOG)
			{
				zbx_free(history[i].value_orig.value_str);

				if (history[i].value_type == ITEM_VALUE_TYPE_LOG && NULL != history[i].source)
					zbx_free(history[i].source);
			}
		}
		total_num += history_num;
	} while (--syncs > 0 || sync_type == ZBX_SYNC_FULL || (skipped_clock != 0 && skipped_clock < max_delay));

	return total_num;
}

/******************************************************************************
 *                                                                            *
 * Function: DCvacuum_text                                                    *
 *                                                                            *
 * Purpose:                                                                   *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Alekasander Vladishev                                              *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static void DCvacuum_text()
{
	char	*first_text;
	int	i, index;
	size_t	offset;

	zabbix_log(LOG_LEVEL_DEBUG, "In DCvacuum_text()");

	/* vacuumng text buffer */
	first_text = NULL;
	for (i = 0; i < cache->history_num; i++)
	{
		index = (cache->history_first + i) % ZBX_HISTORY_SIZE;
		if (cache->history[index].value_type == ITEM_VALUE_TYPE_STR
				|| cache->history[index].value_type == ITEM_VALUE_TYPE_TEXT
				|| cache->history[index].value_type == ITEM_VALUE_TYPE_LOG)
		{
			first_text = cache->history[index].value_orig.value_str;
			break;
		}
	}

	if (NULL != first_text)
	{
		offset = first_text - cache->text;
		for (i = 0; i < cache->history_num; i++)
		{
			index = (cache->history_first + i) % ZBX_HISTORY_SIZE;
			if (cache->history[index].value_type == ITEM_VALUE_TYPE_STR
					|| cache->history[index].value_type == ITEM_VALUE_TYPE_TEXT
					|| cache->history[index].value_type == ITEM_VALUE_TYPE_LOG)
			{
				cache->history[index].value_orig.value_str -= offset;

				if (cache->history[index].value_type == ITEM_VALUE_TYPE_LOG && NULL != cache->history[index].source)
					cache->history[index].source -= offset;
			}
		}
		cache->last_text -= offset;
	} else
		cache->last_text = cache->text;
}

/******************************************************************************
 *                                                                            *
 * Function: DCget_history_ptr                                                *
 *                                                                            *
 * Purpose:                                                                   *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Alekasander Vladishev                                              *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static ZBX_DC_HISTORY *DCget_history_ptr(zbx_uint64_t itemid, size_t text_len)
{
	ZBX_DC_HISTORY	*history;
	int		index;
	size_t		free_len;

retry:
	if (cache->history_num >= ZBX_HISTORY_SIZE)
	{
		UNLOCK_CACHE;

		zabbix_log(LOG_LEVEL_DEBUG, "History buffer is full. Sleeping for 1 second.");
		sleep(1);

		LOCK_CACHE;

		goto retry;
	}

	if (text_len > sizeof(cache->text))
	{
		zabbix_log(LOG_LEVEL_ERR, "Insufficient shared memory");
		exit(-1);
	}

	free_len = sizeof(cache->text) - (cache->last_text - cache->text);

	if (text_len > free_len)
	{
		DCvacuum_text();

		free_len = sizeof(cache->text) - (cache->last_text - cache->text);

		if (text_len > free_len)
		{
			UNLOCK_CACHE;

			zabbix_log(LOG_LEVEL_DEBUG, "History text buffer is full. Sleeping for 1 second.");
			sleep(1);

			LOCK_CACHE;

			goto retry;
		}
	}

	index = (cache->history_first + cache->history_num) % ZBX_HISTORY_SIZE;
	history = &cache->history[index];

	cache->history_num++;

	return history;
}

/******************************************************************************
 *                                                                            *
 * Function: DCadd_history                                                    *
 *                                                                            *
 * Purpose:                                                                   *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Alekasander Vladishev                                              *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
void	DCadd_history(zbx_uint64_t itemid, double value_orig, int clock)
{
	ZBX_DC_HISTORY	*history;

	LOCK_CACHE;

	history = DCget_history_ptr(itemid, 0);

	history->itemid			= itemid;
	history->clock			= clock;
	history->value_type		= ITEM_VALUE_TYPE_FLOAT;
	history->value_orig.value_float	= value_orig;
	history->value.value_float	= 0;
	history->value_null		= 0;
	history->keep_history		= 0;
	history->keep_trends		= 0;

	UNLOCK_CACHE;
}

/******************************************************************************
 *                                                                            *
 * Function: DCadd_history_uint                                               *
 *                                                                            *
 * Purpose:                                                                   *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Alekasander Vladishev                                              *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
void	DCadd_history_uint(zbx_uint64_t itemid, zbx_uint64_t value_orig, int clock)
{
	ZBX_DC_HISTORY	*history;

	LOCK_CACHE;

	history = DCget_history_ptr(itemid, 0);

	history->itemid				= itemid;
	history->clock				= clock;
	history->value_type			= ITEM_VALUE_TYPE_UINT64;
	history->value_orig.value_uint64	= value_orig;
	history->value.value_uint64		= 0;
	history->value_null			= 0;
	history->keep_history			= 0;
	history->keep_trends			= 0;

	UNLOCK_CACHE;
}

/******************************************************************************
 *                                                                            *
 * Function: DCadd_history_str                                                *
 *                                                                            *
 * Purpose:                                                                   *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Alekasander Vladishev                                              *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
void	DCadd_history_str(zbx_uint64_t itemid, char *value_orig, int clock)
{
	ZBX_DC_HISTORY	*history;
	size_t		len;

	LOCK_CACHE;

	len = strlen(value_orig) + 1;
	history = DCget_history_ptr(itemid, len);

	history->itemid			= itemid;
	history->clock			= clock;
	history->value_type		= ITEM_VALUE_TYPE_STR;
	history->value_orig.value_str	= cache->last_text;
	history->value.value_str	= cache->last_text;
	zbx_strlcpy(cache->last_text, value_orig, len);
	history->value_null		= 0;
	cache->last_text		+= len;
	history->keep_history		= 0;
	history->keep_trends		= 0;

	UNLOCK_CACHE;
}

/******************************************************************************
 *                                                                            *
 * Function: DCadd_history_text                                               *
 *                                                                            *
 * Purpose:                                                                   *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Alekasander Vladishev                                              *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
void	DCadd_history_text(zbx_uint64_t itemid, char *value_orig, int clock)
{
	ZBX_DC_HISTORY	*history;
	size_t		len;

	LOCK_CACHE;

	len = strlen(value_orig) + 1;
	history = DCget_history_ptr(itemid, len);

	history->itemid			= itemid;
	history->clock			= clock;
	history->value_type		= ITEM_VALUE_TYPE_TEXT;
	history->value_orig.value_str	= cache->last_text;
	history->value.value_str	= cache->last_text;
	zbx_strlcpy(cache->last_text, value_orig, len);
	history->value_null		= 0;
	cache->last_text		+= len;
	history->keep_history		= 0;
	history->keep_trends		= 0;

	UNLOCK_CACHE;
}

/******************************************************************************
 *                                                                            *
 * Function: DCadd_history_log                                                *
 *                                                                            *
 * Purpose:                                                                   *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Alekasander Vladishev                                              *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
void	DCadd_history_log(zbx_uint64_t itemid, char *value_orig, int clock, int timestamp, char *source, int severity, int lastlogsize)
{
	ZBX_DC_HISTORY	*history;
	size_t		len1, len2;

	LOCK_CACHE;

	len1 = strlen(value_orig) + 1;
	len2 = (NULL != source && *source != '\0') ? strlen(source) + 1 : 0;
	history = DCget_history_ptr(itemid, len1 + len2);

	history->itemid			= itemid;
	history->clock			= clock;
	history->value_type		= ITEM_VALUE_TYPE_LOG;
	history->value_orig.value_str	= cache->last_text;
	history->value.value_str	= cache->last_text;
	zbx_strlcpy(cache->last_text, value_orig, len1);
	history->value_null		= 0;
	cache->last_text		+= len1;
	history->timestamp		= timestamp;

	if (NULL != source && *source != '\0') {
		history->source		= cache->last_text;
		zbx_strlcpy(cache->last_text, source, len2);
		cache->last_text	+= len2;
	}
	else
		history->source		= NULL;

	history->severity		= severity;
	history->lastlogsize		= lastlogsize;
	history->keep_history		= 0;
	history->keep_trends		= 0;

	UNLOCK_CACHE;
}

/******************************************************************************
 *                                                                            *
 * Function: init_database_cache                                              *
 *                                                                            *
 * Purpose: Allocate shared memory for database cache                         *
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
void	init_database_cache(zbx_process_t p)
{
#define ZBX_MAX_ATTEMPTS 10
	int	attempts = 0;

	key_t	shm_key;
	int	shm_id;

	zbx_process = p;

	ZBX_GET_SHM_DBCACHE_KEY(shm_key);

lbl_create:
	if ( -1 == (shm_id = shmget(shm_key, sizeof(ZBX_DC_CACHE), IPC_CREAT | IPC_EXCL | 0666 /* 0022 */)) )
	{
		if( EEXIST == errno )
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Shared memory already exists for database cache, trying to recreate.");

			shm_id = shmget(shm_key, 0 /* get reference */, 0666 /* 0022 */);

			shmctl(shm_id, IPC_RMID, 0);
			if ( ++attempts > ZBX_MAX_ATTEMPTS )
			{
				zabbix_log(LOG_LEVEL_CRIT, "Can't recreate shared memory for database cache. [too many attempts]");
				exit(1);
			}
			if ( attempts > (ZBX_MAX_ATTEMPTS / 2) )
			{
				zabbix_log(LOG_LEVEL_DEBUG, "Wait 1 sec for next attemt of database cache memory allocation.");
				sleep(1);
			}
			goto lbl_create;
		}
		else
		{
			zabbix_log(LOG_LEVEL_CRIT, "Can't allocate shared memory for database cache. [%s]",strerror(errno));
			exit(1);
		}
	}
	
	cache = shmat(shm_id, 0, 0);

	if ((void*)(-1) == cache)
	{
		zabbix_log(LOG_LEVEL_CRIT, "Can't attach shared memory for database cache. [%s]",strerror(errno));
		exit(FAIL);
	}

	if(ZBX_MUTEX_ERROR == zbx_mutex_create_force(&cache_lock, ZBX_MUTEX_CACHE))
	{
		zbx_error("Unable to create mutex for database cache");
		exit(FAIL);
	}

	cache->last_text = cache->text;

	if (NULL == sql)
		sql = zbx_malloc(sql, sql_allocated);

}

/******************************************************************************
 *                                                                            *
 * Function: DCsync_all                                                       *
 *                                                                            *
 * Purpose: writes updates and new data from pool and cache data to database  *
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
static void	DCsync_all()
{
	zabbix_log(LOG_LEVEL_DEBUG,"In DCsync_all()");

	DCsync_history(ZBX_SYNC_FULL);
	DCsync_trends();

	zabbix_log(LOG_LEVEL_DEBUG,"End of DCsync_all()");
}

/******************************************************************************
 *                                                                            *
 * Function: free_database_cache                                              *
 *                                                                            *
 * Purpose: Free memory aloccated for database cache                          *
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
void	free_database_cache()
{

	key_t	shm_key;
	int	shm_id;

	zabbix_log(LOG_LEVEL_DEBUG, "In free_database_cache()");

	if (NULL == cache)
		return;

	DCsync_all();

	LOCK_CACHE;
	
	ZBX_GET_SHM_DBCACHE_KEY(shm_key);

	shm_id = shmget(shm_key, sizeof(ZBX_DC_CACHE), 0);

	if (-1 == shm_id)
	{
		zabbix_log(LOG_LEVEL_ERR, "Can't find shared memory for database cache. [%s]",strerror(errno));
		exit(1);
	}

	shmctl(shm_id, IPC_RMID, 0);

	cache = NULL;

	UNLOCK_CACHE;

	zbx_mutex_destroy(&cache_lock);

	zabbix_log(LOG_LEVEL_DEBUG,"End of free_database_cache()");
}
