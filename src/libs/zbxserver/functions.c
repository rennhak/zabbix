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

#include "comms.h"
#include "db.h"
#include "log.h"
#include "zlog.h"

#include "zbxserver.h"
#include "evalfunc.h"
#include "expression.h"

/******************************************************************************
 *                                                                            *
 * Function: update_functions                                                 *
 *                                                                            *
 * Purpose: re-calculate and updates values of functions related to the item  *
 *                                                                            *
 * Parameters: item - item to update functions for                            *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
void	update_functions(DB_ITEM *item)
{
	DB_FUNCTION	function;
	DB_RESULT	result;
	DB_ROW		row;
	char		value[MAX_STRING_LEN];
	char		value_esc[MAX_STRING_LEN];
	char		*lastvalue;
	int		ret=SUCCEED;

	zabbix_log( LOG_LEVEL_DEBUG, "In update_functions(" ZBX_FS_UI64 ")",
		item->itemid);

/* Oracle does'n support this */
/*	zbx_snprintf(sql,sizeof(sql),"select function,parameter,itemid,lastvalue from functions where itemid=%d group by function,parameter,itemid order by function,parameter,itemid",item->itemid);*/
	result = DBselect("select distinct function,parameter,itemid,lastvalue from functions where itemid=" ZBX_FS_UI64,
		item->itemid);

	while((row=DBfetch(result)))
	{
		function.function=row[0];
		function.parameter=row[1];
		ZBX_STR2UINT64(function.itemid,row[2]);
/*		function.itemid=atoi(row[2]); */
/*		It is not required to check lastvalue for NULL here */
		lastvalue=row[3];

		zabbix_log( LOG_LEVEL_DEBUG, "ItemId:" ZBX_FS_UI64 " Evaluating %s(%s)",
			function.itemid,
			function.function,
			function.parameter);

		ret = evaluate_function(value,item,function.function,function.parameter);
		if( FAIL == ret)	
		{
			zabbix_log( LOG_LEVEL_DEBUG, "Evaluation failed for function:%s",
				function.function);
			continue;
		}
		if (ret == SUCCEED)
		{
			/* Update only if lastvalue differs from new one */
			if( DBis_null(lastvalue)==SUCCEED || (strcmp(lastvalue,value) != 0))
			{
				DBescape_string(value,value_esc,MAX_STRING_LEN);
				DBexecute("update functions set lastvalue='%s' where itemid=" ZBX_FS_UI64 " and function='%s' and parameter='%s'",
					value_esc,
					function.itemid,
					function.function,
					function.parameter );
			}
			else
			{
				zabbix_log( LOG_LEVEL_DEBUG, "Do not update functions, same value");
			}
		}
	}

	DBfree_result(result);

	zabbix_log( LOG_LEVEL_DEBUG, "End update_functions()");
}

/******************************************************************************
 *                                                                            *
 * Function: update_triggers                                                  *
 *                                                                            *
 * Purpose: re-calculate and updates values of triggers related to the item   *
 *                                                                            *
 * Parameters: itemid - item to update trigger values for                     *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
void	update_triggers(zbx_uint64_t itemid)
{
	char	*exp;
	char	error[MAX_STRING_LEN];
	int	exp_value;
	DB_TRIGGER	trigger;
	DB_RESULT	result;
	DB_ROW		row;

	zabbix_log( LOG_LEVEL_DEBUG, "In update_triggers [itemid:" ZBX_FS_UI64 "]",
		itemid);

	result = DBselect("select distinct t.triggerid,t.expression,t.description,t.url,t.comments,t.status,t.value,t.priority,t.type from triggers t,functions f,items i where i.status<>%d and i.itemid=f.itemid and t.status=%d and f.triggerid=t.triggerid and f.itemid=" ZBX_FS_UI64,
		ITEM_STATUS_NOTSUPPORTED,
		TRIGGER_STATUS_ENABLED,
		itemid);

	while((row=DBfetch(result)))
	{
		ZBX_STR2UINT64(trigger.triggerid,row[0]);
		strscpy(trigger.expression,row[1]);
		strscpy(trigger.description,row[2]);
		trigger.url		= row[3];
		trigger.comments	= row[4];
		trigger.status		= atoi(row[5]);
		trigger.value		= atoi(row[6]);
		trigger.priority	= atoi(row[7]);
		trigger.type		= atoi(row[8]);

		exp = strdup(trigger.expression);
		if( evaluate_expression(&exp_value, &exp, trigger.value, error, sizeof(error)) != 0 )
		{
			zabbix_log( LOG_LEVEL_WARNING, "Expression [%s] cannot be evaluated [%s]",
				trigger.expression,
				error);
			zabbix_syslog("Expression [%s] cannot be evaluated [%s]",
				trigger.expression,
				error);
/*			DBupdate_trigger_value(&trigger, exp_value, time(NULL), error);*//* We shouldn't update triggervalue if expressions failed */
		}
		else
		{
			DBupdate_trigger_value(&trigger, exp_value, time(NULL), NULL);
		}
		zbx_free(exp);
	}
	DBfree_result(result);
	zabbix_log( LOG_LEVEL_DEBUG, "End update_triggers [" ZBX_FS_UI64 "]",
		itemid);
}

/******************************************************************************
 *                                                                            *
 * Function: add_history                                                      *
 *                                                                            *
 * Purpose: add new value to history                                          *
 *                                                                            *
 * Parameters: item - item data                                               *
 *             value - new value of the item                                  *
 *             now   - new value of the item                                  *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int	add_history(DB_ITEM *item, AGENT_RESULT *value, int now)
{
	int ret = SUCCEED;

	zabbix_log( LOG_LEVEL_DEBUG, "In add_history(key:%s,value_type:%X,type:%X)",
		item->key,
		item->value_type,
		value->type);

	if (value->type & AR_UINT64)
		zabbix_log( LOG_LEVEL_DEBUG, "In add_history(itemid:"ZBX_FS_UI64",UINT64:"ZBX_FS_UI64")",
			item->itemid,
			value->ui64);
	if (value->type & AR_STRING)
		zabbix_log( LOG_LEVEL_DEBUG, "In add_history(itemid:"ZBX_FS_UI64",STRING:%s)",
			item->itemid,
			value->str);
	if (value->type & AR_DOUBLE)
		zabbix_log( LOG_LEVEL_DEBUG, "In add_history(itemid:"ZBX_FS_UI64",DOUBLE:"ZBX_FS_DBL")",
			item->itemid,
			value->dbl);
	if (value->type & AR_TEXT)
		zabbix_log( LOG_LEVEL_DEBUG, "In add_history(itemid:"ZBX_FS_UI64",TEXT:[%s])",
			item->itemid,
			value->text);

	if(item->history>0)
	{
		if( (item->value_type==ITEM_VALUE_TYPE_FLOAT) || (item->value_type==ITEM_VALUE_TYPE_UINT64))
		{
			/* Should we store delta or original value? */
			if(item->delta == ITEM_STORE_AS_IS)
			{
				if(item->value_type==ITEM_VALUE_TYPE_UINT64)
				{
					if(GET_UI64_RESULT(value))
						DBadd_history_uint(item->itemid,value->ui64,now);
				}
				else if(item->value_type==ITEM_VALUE_TYPE_FLOAT)
				{
					if(GET_DBL_RESULT(value))
						DBadd_history(item->itemid,value->dbl,now);
				}
			}
			/* Delta as speed of change */
			else if(item->delta == ITEM_STORE_SPEED_PER_SECOND)
			{
				/* Save delta */
				if( ITEM_VALUE_TYPE_FLOAT == item->value_type )
				{
					if(GET_DBL_RESULT(value) && (item->prevorgvalue_null == 0) && (item->prevorgvalue_dbl <= value->dbl) && (now != item->lastclock))
					{
						DBadd_history(
							item->itemid,
							(value->dbl - item->prevorgvalue_dbl)/(now-item->lastclock),
							now);
					}
				}
				else if( ITEM_VALUE_TYPE_UINT64 == item->value_type )
				{
					if(GET_UI64_RESULT(value) && (item->prevorgvalue_null == 0) && (item->prevorgvalue_uint64 <= value->ui64) && (now != item->lastclock))
					{
						DBadd_history_uint(
							item->itemid,
							(zbx_uint64_t)(value->ui64 - item->prevorgvalue_uint64)/(now-item->lastclock),
							now);
					}
				}
			}
			/* Real delta: simple difference between values */
			else if(item->delta == ITEM_STORE_SIMPLE_CHANGE)
			{
				/* Save delta */
				if( ITEM_VALUE_TYPE_FLOAT == item->value_type )
				{
					if(GET_DBL_RESULT(value) && (item->prevorgvalue_null == 0) && (item->prevorgvalue_dbl <= value->dbl) )
					{
						DBadd_history(item->itemid, (value->dbl - item->prevorgvalue_dbl), now);
					}
				}
				else if(item->value_type==ITEM_VALUE_TYPE_UINT64)
				{
					if(GET_UI64_RESULT(value) && (item->prevorgvalue_null == 0) && (item->prevorgvalue_uint64 <= value->ui64) )
					{
						DBadd_history_uint(item->itemid, value->ui64 - item->prevorgvalue_uint64, now);
					}
				}
			}
			else
			{
				zabbix_log(LOG_LEVEL_ERR, "Value not stored for itemid [%d]. Unknown delta [%d]",
					item->itemid,
					item->delta);
				zabbix_syslog("Value not stored for itemid [%d]. Unknown delta [%d]",
					item->itemid,
					item->delta);
				ret = FAIL;
			}
		}
		else if(item->value_type==ITEM_VALUE_TYPE_STR)
		{
			if(GET_STR_RESULT(value))
				DBadd_history_str(item->itemid,value->str,now);
		}
		else if(item->value_type==ITEM_VALUE_TYPE_LOG)
		{
			if(GET_STR_RESULT(value))
				DBadd_history_log(0, item->itemid,value->str,now,item->timestamp,item->eventlog_source,item->eventlog_severity);
		}
		else if(item->value_type==ITEM_VALUE_TYPE_TEXT)
		{
			if(GET_TEXT_RESULT(value))
				DBadd_history_text(item->itemid,value->text,now);
		}
		else
		{
			zabbix_log(LOG_LEVEL_ERR, "Unknown value type [%d] for itemid [" ZBX_FS_UI64 "]",
				item->value_type,
				item->itemid);
		}
	}

	zabbix_log( LOG_LEVEL_DEBUG, "End of add_history");

	return ret;
}

/******************************************************************************
 *                                                                            *
 * Function: update_item                                                      *
 *                                                                            *
 * Purpose: update item info after new value is received                      *
 *                                                                            *
 * Parameters: item - item data                                               *
 *             value - new value of the item                                  *
 *             now   - current timestamp                                      * 
 *                                                                            *
 * Author: Alexei Vladishev, Eugene Grigorjev                                 *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static void	update_item(DB_ITEM *item, AGENT_RESULT *value, time_t now)
{
	char	value_esc[MAX_STRING_LEN];

	zabbix_log( LOG_LEVEL_DEBUG, "In update_item()");

	value_esc[0]	= '\0';
	
	if(item->delta == ITEM_STORE_AS_IS)
	{
		if(GET_STR_RESULT(value))
		{
			DBescape_string(value->str, value_esc, sizeof(value_esc));
		}

		if (item->value_type == ITEM_VALUE_TYPE_LOG) {
			DBexecute("update items set nextcheck=%d,prevvalue=lastvalue,lastvalue='%s',lastclock=%d,lastlogsize=%d where itemid=" ZBX_FS_UI64,
				calculate_item_nextcheck(item->itemid, item->type, item->delay, item->delay_flex, now),
				value_esc,
				(int)now,
				item->lastlogsize,
				item->itemid);
		} else {
			DBexecute("update items set nextcheck=%d,prevvalue=lastvalue,lastvalue='%s',lastclock=%d where itemid=" ZBX_FS_UI64,
				calculate_item_nextcheck(item->itemid, item->type, item->delay, item->delay_flex, now),
				value_esc,
				(int)now,
				item->itemid);
		}
	}
	/* Logic for delta as speed of change */
	else if(item->delta == ITEM_STORE_SPEED_PER_SECOND)
	{
		if(item->value_type == ITEM_VALUE_TYPE_FLOAT)
		{
			if(GET_DBL_RESULT(value))
			{
				if((item->prevorgvalue_null == 0) && (item->prevorgvalue_dbl <= value->dbl) )
				{
					/* In order to continue normal processing, we assume difference 1 second
					   Otherwise function update_functions and update_triggers won't work correctly*/
					if(now != item->lastclock)
					{
						DBexecute("update items set nextcheck=%d,prevvalue=lastvalue,prevorgvalue='" ZBX_FS_DBL "',"
						"lastvalue='" ZBX_FS_DBL "',lastclock=%d where itemid=" ZBX_FS_UI64,
							calculate_item_nextcheck(item->itemid, item->type, item->delay,item->delay_flex,now),
							value->dbl,
							(value->dbl - item->prevorgvalue_dbl)/(now-item->lastclock),
							(int)now,
							item->itemid);
						SET_DBL_RESULT(value, (double)(value->dbl - item->prevorgvalue_dbl)/(now-item->lastclock));
					}
					else
					{
						DBexecute("update items set nextcheck=%d,prevvalue=lastvalue,prevorgvalue='" ZBX_FS_DBL "',"
						"lastvalue='" ZBX_FS_DBL "',lastclock=%d where itemid=" ZBX_FS_UI64,
							calculate_item_nextcheck(item->itemid, item->type, item->delay,item->delay_flex,now),
							value->dbl,
							value->dbl - item->prevorgvalue_dbl,
							(int)now,
							item->itemid);
						SET_DBL_RESULT(value, (double)(value->dbl - item->prevorgvalue_dbl));
					}
				}
				else
				{
					DBexecute("update items set nextcheck=%d,prevorgvalue='" ZBX_FS_DBL "',lastclock=%d where itemid=" ZBX_FS_UI64,
						calculate_item_nextcheck(item->itemid, item->type, item->delay,item->delay_flex,now),
						value->dbl,
						(int)now,
						item->itemid);
				}
			}
		}
		else if(item->value_type == ITEM_VALUE_TYPE_UINT64)
		{
			if(GET_UI64_RESULT(value))
			{
				if((item->prevorgvalue_null == 0) && (item->prevorgvalue_uint64 <= value->ui64) )
				{
					if(now != item->lastclock)
					{
						DBexecute("update items set nextcheck=%d,prevvalue=lastvalue,prevorgvalue='" ZBX_FS_UI64 "',"
						"lastvalue='" ZBX_FS_UI64 "',lastclock=%d where itemid=" ZBX_FS_UI64,
							calculate_item_nextcheck(item->itemid, item->type, item->delay,item->delay_flex,now),
							value->ui64,
							((zbx_uint64_t)(value->ui64 - item->prevorgvalue_uint64))/(now-item->lastclock),
							(int)now,
							item->itemid);
						SET_UI64_RESULT(value, (zbx_uint64_t)(value->ui64 - item->prevorgvalue_uint64)/(now-item->lastclock));
					}
					else
					{
						DBexecute("update items set nextcheck=%d,prevvalue=lastvalue,prevorgvalue='" ZBX_FS_UI64 "',"
						"lastvalue='" ZBX_FS_DBL "',lastclock=%d where itemid=" ZBX_FS_UI64,
							calculate_item_nextcheck(item->itemid, item->type, item->delay,item->delay_flex,now),
							value->ui64,
							(double)(value->ui64 - item->prevorgvalue_uint64),
							(int)now,
							item->itemid);
						SET_UI64_RESULT(value, (zbx_uint64_t)(value->ui64 - item->prevorgvalue_uint64));
					}
				}
				else
				{
					DBexecute("update items set nextcheck=%d,prevorgvalue='" ZBX_FS_UI64 "',lastclock=%d where itemid=" ZBX_FS_UI64,
						calculate_item_nextcheck(item->itemid, item->type, item->delay,item->delay_flex,now),
						value->ui64,
						(int)now,
						item->itemid);
				}
			}
		}
	}
	/* Real delta: simple difference between values */
	else if(item->delta == ITEM_STORE_SIMPLE_CHANGE)
	{
		if(item->value_type == ITEM_VALUE_TYPE_FLOAT)
		{
			if(GET_DBL_RESULT(value))
			{
				if((item->prevorgvalue_null == 0) && (item->prevorgvalue_dbl <= value->dbl))
				{
					DBexecute("update items set nextcheck=%d,prevvalue=lastvalue,prevorgvalue='" ZBX_FS_DBL "',"
					"lastvalue='" ZBX_FS_DBL "',lastclock=%d where itemid=" ZBX_FS_UI64,
						calculate_item_nextcheck(item->itemid, item->type, item->delay,item->delay_flex,now),
						value->dbl,
						(value->dbl - item->prevorgvalue_dbl),
						(int)now,
						item->itemid);
					SET_DBL_RESULT(value, (double)(value->dbl - item->prevorgvalue_dbl));
				}
				else
				{
					DBexecute("update items set nextcheck=%d,prevorgvalue='" ZBX_FS_DBL "',lastclock=%d where itemid=" ZBX_FS_UI64,
						calculate_item_nextcheck(item->itemid, item->type, item->delay,item->delay_flex, now),
						value->dbl,
						(int)now,
						item->itemid);
				}
			}
		}
		else if(item->value_type == ITEM_VALUE_TYPE_UINT64)
		{
			if(GET_UI64_RESULT(value))
			{
				if((item->prevorgvalue_null == 0) && (item->prevorgvalue_uint64 <= value->ui64))
				{
					DBexecute("update items set nextcheck=%d,prevvalue=lastvalue,prevorgvalue='" ZBX_FS_UI64 "',"
					"lastvalue='" ZBX_FS_UI64 "',lastclock=%d where itemid=" ZBX_FS_UI64,
						calculate_item_nextcheck(item->itemid, item->type, item->delay,item->delay_flex,now),
						value->ui64,
						(value->ui64 - item->prevorgvalue_uint64),
						(int)now,
						item->itemid);
					SET_UI64_RESULT(value, (zbx_uint64_t)(value->ui64 - item->prevorgvalue_uint64));
				}
				else
				{
					DBexecute("update items set nextcheck=%d,prevorgvalue='" ZBX_FS_UI64 "',lastclock=%d where itemid=" ZBX_FS_UI64,
						calculate_item_nextcheck(item->itemid, item->type, item->delay,item->delay_flex, now),
						value->ui64,
						(int)now,
						item->itemid);
				}
			}
		}
	}

	item->prevvalue_str	= item->lastvalue_str;
	item->prevvalue_dbl	= item->lastvalue_dbl;
	item->prevvalue_uint64	= item->lastvalue_uint64;
	item->prevvalue_null	= item->lastvalue_null;

	item->lastvalue_uint64	= value->ui64;
	item->lastvalue_dbl	= value->dbl;
	item->lastvalue_str	= value->str;
	item->lastvalue_null	= 0;

/* Update item status if required */
	if(item->status == ITEM_STATUS_NOTSUPPORTED)
	{
		zabbix_log( LOG_LEVEL_WARNING, "Parameter [%s] became supported by agent on host [%s]",
			item->key,
			item->host_name);
		zabbix_syslog("Parameter [%s] became supported by agent on host [%s]",
			item->key,
			item->host_name);
		item->status = ITEM_STATUS_ACTIVE;
		DBexecute("update items set status=%d,error='' where itemid=" ZBX_FS_UI64,
			ITEM_STATUS_ACTIVE,
			item->itemid);
	}

	/* Required for nodata() */
	item->lastclock = now;

	zabbix_log( LOG_LEVEL_DEBUG, "End update_item()");
}

/******************************************************************************
 *                                                                            *
 * Function: process_new_value                                                *
 *                                                                            *
 * Purpose: process new item value                                            *
 *                                                                            *
 * Parameters: item - item data                                               *
 *             value - new value of the item                                  *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments: for trapper poller process                                       *
 *                                                                            *
 ******************************************************************************/
void	process_new_value(DB_ITEM *item, AGENT_RESULT *value, time_t now)
{
	zabbix_log( LOG_LEVEL_DEBUG, "In process_new_value(%s)",
		item->key);

	if( ITEM_MULTIPLIER_USE == item->multiplier )
	{
		if( ITEM_VALUE_TYPE_FLOAT == item->value_type )
		{
			if(GET_DBL_RESULT(value))
			{
				UNSET_RESULT_EXCLUDING(value, AR_DOUBLE);
				SET_DBL_RESULT(value, value->dbl * strtod(item->formula, NULL));
			}
		}
		else if( ITEM_VALUE_TYPE_UINT64 == item->value_type )
		{
			if(GET_UI64_RESULT(value))
			{
				UNSET_RESULT_EXCLUDING(value, AR_UINT64);
				if(is_uint(item->formula) == SUCCEED)
				{
					SET_UI64_RESULT(value, value->ui64 * zbx_atoui64((item->formula)));
				}
				else
				{
					SET_UI64_RESULT(value, (zbx_uint64_t)((double)value->ui64 * strtod(item->formula, NULL)));
				}
			}
		}
	}

	add_history(item, value, now);
	update_item(item, value, now);
	update_functions(item);
}

/******************************************************************************
 *                                                                            *
 * Function: proxy_add_history                                                *
 *                                                                            *
 * Purpose: add new value to history                                          *
 *                                                                            *
 * Parameters: item - item data                                               *
 *             value - new value of the item                                  *
 *             now   - new value of the item                                  *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int	proxy_add_history(DB_ITEM *item, AGENT_RESULT *value, int now)
{
	int ret = SUCCEED;

	zabbix_log( LOG_LEVEL_DEBUG, "In proxy_add_history(key:%s,value_type:%X,type:%X)",
		item->key,
		item->value_type,
		value->type);

	if (value->type & AR_UINT64)
		zabbix_log( LOG_LEVEL_DEBUG, "In proxy_add_history(itemid:" ZBX_FS_UI64 ",UINT64:" ZBX_FS_UI64 ")",
			item->itemid,
			value->ui64);
	if (value->type & AR_STRING)
		zabbix_log( LOG_LEVEL_DEBUG, "In proxy_add_history(itemid:" ZBX_FS_UI64 ",STRING:%s)",
			item->itemid,
			value->str);
	if (value->type & AR_DOUBLE)
		zabbix_log( LOG_LEVEL_DEBUG, "In proxy_add_history(itemid:" ZBX_FS_UI64 ",DOUBLE:" ZBX_FS_DBL ")",
			item->itemid,
			value->dbl);
	if (value->type & AR_TEXT)
		zabbix_log( LOG_LEVEL_DEBUG, "In proxy_add_history(itemid:" ZBX_FS_UI64 ",TEXT:[%s])",
			item->itemid,
			value->text);

	if(item->value_type==ITEM_VALUE_TYPE_UINT64)
	{
		if(GET_UI64_RESULT(value))
			DBproxy_add_history_uint(item->host_name, item->key, now, value->ui64);
	}
	else if(item->value_type==ITEM_VALUE_TYPE_FLOAT)
	{
		if(GET_DBL_RESULT(value))
			DBproxy_add_history(item->host_name, item->key, now, value->dbl);
	}
	else if(item->value_type==ITEM_VALUE_TYPE_STR)
	{
		if(GET_STR_RESULT(value))
			DBproxy_add_history_str(item->host_name, item->key, now, value->str);
	}
	else if(item->value_type==ITEM_VALUE_TYPE_LOG)
	{
		if(GET_STR_RESULT(value))
			DBproxy_add_history_log(item->host_name, item->key, now, item->timestamp, item->eventlog_source, item->eventlog_severity, value->str);
	}
	else if(item->value_type==ITEM_VALUE_TYPE_TEXT)
	{
		if(GET_TEXT_RESULT(value))
			DBproxy_add_history_text(item->host_name, item->key, now, value->text);
	}
	else
	{
		zabbix_log(LOG_LEVEL_ERR, "Unknown value type [%d] for itemid [" ZBX_FS_UI64 "]",
			item->value_type,
			item->itemid);
	}

	zabbix_log( LOG_LEVEL_DEBUG, "End of proxy_add_history");

	return ret;
}

/******************************************************************************
 *                                                                            *
 * Function: proxy_update_item                                                *
 *                                                                            *
 * Purpose: update item info after new value is received                      *
 *                                                                            *
 * Parameters: item - item data                                               *
 *             value - new value of the item                                  *
 *             now   - current timestamp                                      * 
 *                                                                            *
 * Author: Alexei Vladishev, Eugene Grigorjev                                 *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static void	proxy_update_item(DB_ITEM *item, AGENT_RESULT *value, time_t now)
{
	zabbix_log( LOG_LEVEL_DEBUG, "In proxy_update_item()");

	if (item->value_type == ITEM_VALUE_TYPE_LOG) {
		DBexecute("update items set nextcheck=%d,lastlogsize=%d where itemid=" ZBX_FS_UI64,
			calculate_item_nextcheck(item->itemid, item->type, item->delay, item->delay_flex, now),
			item->lastlogsize,
			item->itemid);
	} else {
		DBexecute("update items set nextcheck=%d where itemid=" ZBX_FS_UI64,
			calculate_item_nextcheck(item->itemid, item->type, item->delay, item->delay_flex, now),
			item->itemid);
	}

	item->prevvalue_str	= item->lastvalue_str;
	item->prevvalue_dbl	= item->lastvalue_dbl;
	item->prevvalue_uint64	= item->lastvalue_uint64;
	item->prevvalue_null	= item->lastvalue_null;

	item->lastvalue_uint64	= value->ui64;
	item->lastvalue_dbl	= value->dbl;
	item->lastvalue_str	= value->str;
	item->lastvalue_null	= 0;

	if (item->status == ITEM_STATUS_NOTSUPPORTED) {
		zabbix_log( LOG_LEVEL_WARNING, "Parameter [%s] became supported by agent on host [%s]",
			item->key,
			item->host_name);
		zabbix_syslog("Parameter [%s] became supported by agent on host [%s]",
			item->key,
			item->host_name);
		item->status = ITEM_STATUS_ACTIVE;
		DBexecute("update items set status=%d where itemid=" ZBX_FS_UI64,
			ITEM_STATUS_ACTIVE,
			item->itemid);
	}

	/* Required for nodata() */
	item->lastclock = now;

	zabbix_log( LOG_LEVEL_DEBUG, "End proxy_update_item()");
}

/******************************************************************************
 *                                                                            *
 * Function: proxy_process_new_value                                          *
 *                                                                            *
 * Purpose: process new item value                                            *
 *                                                                            *
 * Parameters: item - item data                                               *
 *             value - new value of the item                                  *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments: for trapper poller process                                       *
 *                                                                            *
 ******************************************************************************/
void	proxy_process_new_value(DB_ITEM *item, AGENT_RESULT *value, time_t now)
{
	zabbix_log( LOG_LEVEL_DEBUG, "In proxy_process_new_value(%s)",
		item->key);

	proxy_add_history(item, value, now);
	proxy_update_item(item, value, now);
}