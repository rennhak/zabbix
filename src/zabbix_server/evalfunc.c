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
#include "db.h"
#include "log.h"
#include "zlog.h"

#include "evalfunc.h"
#include "iconv.h"

/******************************************************************************
 *                                                                            *
 * Function: evaluate_LOGSOURCE                                               *
 *                                                                            *
 * Purpose: evaluate function 'logsource' for the item                        *
 *                                                                            *
 * Parameters: item - item (performance metric)                               *
 *             parameter - ignored                                            *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, result is stored in 'value' *
 *               FAIL - failed to evaluate function                           *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int evaluate_LOGSOURCE(char *value, DB_ITEM *item, char *parameter)
{
	DB_RESULT	result;
	DB_ROW	row;

	char		sql[MAX_STRING_LEN];
	int		now;
	int		res = SUCCEED;

	if(item->value_type != ITEM_VALUE_TYPE_LOG)
	{
		return	FAIL;
	}

	now=time(NULL);

	zbx_snprintf(sql,sizeof(sql),"select source from history_log where itemid=" ZBX_FS_UI64 " order by clock desc",
		item->itemid);

	result = DBselectN(sql,1);
	row = DBfetch(result);

	if(!row || DBis_null(row[0])==SUCCEED)
	{
		zabbix_log(LOG_LEVEL_DEBUG, "Result for LOGSOURCE is empty" );
		res = FAIL;
	}
	else
	{
		if(strcmp(row[0], parameter) == 0)
		{
			strcpy(value,"1");
		}
		else
		{
			strcpy(value,"0");
		}
	}
	DBfree_result(result);

	return res;
}

/******************************************************************************
 *                                                                            *
 * Function: evaluate_LOGSEVERITY                                             *
 *                                                                            *
 * Purpose: evaluate function 'logseverity' for the item                      *
 *                                                                            *
 * Parameters: item - item (performance metric)                               *
 *             parameter - ignored                                            *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, result is stored in 'value' *
 *               FAIL - failed to evaluate function                           *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int evaluate_LOGSEVERITY(char *value, DB_ITEM *item, char *parameter)
{
	DB_RESULT	result;
	DB_ROW		row;

	char		sql[MAX_STRING_LEN];
	int		now;
	int		res = SUCCEED;

	if(item->value_type != ITEM_VALUE_TYPE_LOG)
	{
		return	FAIL;
	}

	now=time(NULL);

	zbx_snprintf(sql,sizeof(sql),"select severity from history_log where itemid=" ZBX_FS_UI64 " order by clock desc",
		item->itemid);

	result = DBselectN(sql,1);
	row = DBfetch(result);
	if(!row || DBis_null(row[0])==SUCCEED)
	{
		zabbix_log(LOG_LEVEL_DEBUG, "Result for LOGSEVERITY is empty" );
		res = FAIL;
	}
	else
	{
		strcpy(value,row[0]);
	}
	DBfree_result(result);

	return res;
}

/******************************************************************************
 *                                                                            *
 * Function: evaluate_COUNT                                                   *
 *                                                                            *
 * Purpose: evaluate function 'count' for the item                            *
 *                                                                            *
 * Parameters: item - item (performance metric)                               *
 *             parameter - number of seconds                                  *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, result is stored in 'value' *
 *               FAIL - failed to evaluate function                           *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int evaluate_COUNT(char *value, DB_ITEM *item, char *parameter)
{
	DB_RESULT	result;
	DB_ROW	row;

	char		period[MAX_STRING_LEN+1];
	char		op[MAX_STRING_LEN+1];
	char		cmp[MAX_STRING_LEN+1];
	char		cmp_esc[MAX_STRING_LEN+1];

	int		now;
	int		res = SUCCEED;

	char		*table = NULL;
	char		table_ui64[] = "history_uint";
	char		table_float[] = "history";
	char		table_log[] = "history_log";
	char		table_str[] = "history_str";

	zabbix_log( LOG_LEVEL_DEBUG, "In evaluate_COUNT(param:%s)",
		parameter);


	switch(item->value_type)
	{
		case ITEM_VALUE_TYPE_FLOAT:	table = table_float;	break;
		case ITEM_VALUE_TYPE_UINT64:	table = table_ui64;	break;
		case ITEM_VALUE_TYPE_LOG:	table = table_log;	break;
		case ITEM_VALUE_TYPE_STR:	table = table_str;	break;
		default:
			return FAIL;
	}

	now=time(NULL);

	if(get_param(parameter, 1, period, MAX_STRING_LEN) != 0)
	{
		return FAIL;
	}
	if(get_param(parameter, 2, cmp, MAX_STRING_LEN) != 0)
	{
		result = DBselect("select count(value) from %s where clock>%d and itemid=" ZBX_FS_UI64,
			table,
			now-atoi(period),
			item->itemid);
		
	}
	else
	{
		if(get_param(parameter, 3, op, MAX_STRING_LEN) != 0)
		{
			strscpy(op,"eq");
		}
		DBescape_string(cmp, cmp_esc, sizeof(cmp_esc));
		/* ITEM_VALUE_TYPE_UINT64 */
		if( (item->value_type == ITEM_VALUE_TYPE_UINT64) && (strcmp(op,"eq") == 0))
		{
			result = DBselect("select count(value) from history_uint where clock>%d and value=" ZBX_FS_UI64 " and itemid=" ZBX_FS_UI64,
				now-atoi(period),
				zbx_atoui64(cmp_esc),
				item->itemid);
		}
		else if( (item->value_type == ITEM_VALUE_TYPE_UINT64) && (strcmp(op,"ne") == 0))
		{
			result = DBselect("select count(value) from history_uint where clock>%d and value<>" ZBX_FS_UI64 " and itemid=" ZBX_FS_UI64,
				now-atoi(period),
				zbx_atoui64(cmp_esc),
				item->itemid);
		}
		else if( (item->value_type == ITEM_VALUE_TYPE_UINT64) && (strcmp(op,"gt") == 0))
		{
			result = DBselect("select count(value) from history_uint where clock>%d and value>" ZBX_FS_UI64 " and itemid=" ZBX_FS_UI64,
				now-atoi(period),
				zbx_atoui64(cmp_esc),
				item->itemid);
		}
		else if( (item->value_type == ITEM_VALUE_TYPE_UINT64) && (strcmp(op,"lt") == 0))
		{
			result = DBselect("select count(value) from history_uint where clock>%d and value<" ZBX_FS_UI64 " and itemid=" ZBX_FS_UI64,
				now-atoi(period),
				zbx_atoui64(cmp_esc),
				item->itemid);
		}
		else if( (item->value_type == ITEM_VALUE_TYPE_UINT64) && (strcmp(op,"ge") == 0))
		{
			result = DBselect("select count(value) from history_uint where clock>%d and value>=" ZBX_FS_UI64 " and itemid=" ZBX_FS_UI64,
				now-atoi(period),
				zbx_atoui64(cmp_esc),
				item->itemid);
		}
		else if( (item->value_type == ITEM_VALUE_TYPE_UINT64) && (strcmp(op,"le") == 0))
		{
			result = DBselect("select count(value) from history_uint where clock>%d and value<=" ZBX_FS_UI64 " and itemid=" ZBX_FS_UI64,
				now-atoi(period),
				zbx_atoui64(cmp_esc),
				item->itemid);
		}
		/* ITEM_VALUE_TYPE_FLOAT */
		else if( (item->value_type == ITEM_VALUE_TYPE_FLOAT) && (strcmp(op,"eq") == 0))
		{
			result = DBselect("select count(value) from history where clock>%d and value+0.00001>" ZBX_FS_DBL " and value-0.0001<" ZBX_FS_DBL " and itemid=" ZBX_FS_UI64,
				now-atoi(period),
				atof(cmp_esc),
				atof(cmp_esc),
				item->itemid);
		}
		else if( (item->value_type == ITEM_VALUE_TYPE_FLOAT) && (strcmp(op,"ne") == 0))
		{
			result = DBselect("select count(value) from history where clock>%d and ((value+0.00001<" ZBX_FS_DBL ") or (value-0.0001>" ZBX_FS_DBL ")) and itemid=" ZBX_FS_UI64,
				now-atoi(period),
				atof(cmp_esc),
				atof(cmp_esc),
				item->itemid);
		}
		else if( (item->value_type == ITEM_VALUE_TYPE_FLOAT) && (strcmp(op,"gt") == 0))
		{
			result = DBselect("select count(value) from history where clock>%d and value>" ZBX_FS_DBL " and itemid=" ZBX_FS_UI64,
				now-atoi(period),
				atof(cmp_esc),
				item->itemid);
		}
		else if( (item->value_type == ITEM_VALUE_TYPE_FLOAT) && (strcmp(op,"ge") == 0))
		{
			result = DBselect("select count(value) from history where clock>=%d and value>" ZBX_FS_DBL " and itemid=" ZBX_FS_UI64,
				now-atoi(period),
				atof(cmp_esc),
				item->itemid);
		}
		else if( (item->value_type == ITEM_VALUE_TYPE_FLOAT) && (strcmp(op,"lt") == 0))
		{
			result = DBselect("select count(value) from history where clock>%d and value<" ZBX_FS_DBL " and itemid=" ZBX_FS_UI64,
				now-atoi(period),
				atof(cmp_esc),
				item->itemid);
		}
		else if( (item->value_type == ITEM_VALUE_TYPE_FLOAT) && (strcmp(op,"le") == 0))
		{
			result = DBselect("select count(value) from history where clock>%d and value<=" ZBX_FS_DBL " and itemid=" ZBX_FS_UI64,
				now-atoi(period),
				atof(cmp_esc),
				item->itemid);
		}
		else if(item->value_type == ITEM_VALUE_TYPE_LOG)
		{
			result = DBselect("select count(value) from history_log where clock>%d and value like '%s' and itemid=" ZBX_FS_UI64,
				now-atoi(period),
				cmp_esc,
				item->itemid);
		}
		else
		{
			result = DBselect("select count(value) from history_str where clock>%d and value like '%s' and itemid=" ZBX_FS_UI64,
				now-atoi(period),
				cmp_esc,
				item->itemid);
		}
	}


	row = DBfetch(result);

	if(!row || DBis_null(row[0])==SUCCEED)
	{
		zabbix_log(LOG_LEVEL_DEBUG, "Result for COUNT is empty" );
		res = FAIL;
	}
	else
	{
		strcpy(value,row[0]);
	}
	DBfree_result(result);

	zabbix_log( LOG_LEVEL_DEBUG, "End evaluate_COUNT");

	return res;
}

/******************************************************************************
 *                                                                            *
 * Function: evaluate_SUM                                                     *
 *                                                                            *
 * Purpose: evaluate function 'sum' for the item                              *
 *                                                                            *
 * Parameters: item - item (performance metric)                               *
 *             parameter - number of seconds                                  *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, result is stored in 'value' *
 *               FAIL - failed to evaluate function                           *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int evaluate_SUM(char *value, DB_ITEM *item, int parameter, int flag)
{
	DB_RESULT	result;
	DB_ROW	row;

	char		sql[MAX_STRING_LEN];
	int		now;
	int		res = SUCCEED;
	int		rows = 0;
	double		sum=0;
	zbx_uint64_t	sum_uint64=0;
	zbx_uint64_t	value_uint64;

	char		*table = NULL;
	char		table_ui64[] = "history_uint";
	char		table_float[] = "history";


	switch(item->value_type)
	{
		case ITEM_VALUE_TYPE_FLOAT:	table = table_float;	break;
		case ITEM_VALUE_TYPE_UINT64:	table = table_ui64;	break;
		default:
			return FAIL;
	}

	now=time(NULL);

	if(flag == ZBX_FLAG_SEC)
	{
		result = DBselect("select sum(value) from %s where clock>%d and itemid=" ZBX_FS_UI64,
			table,
			now-parameter,
			item->itemid);

		row = DBfetch(result);
		if(!row || DBis_null(row[0])==SUCCEED)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Result for SUM is empty" );
			res = FAIL;
		}
		else
		{
			strcpy(value,row[0]);
		}
	}
	else if(flag == ZBX_FLAG_VALUES)
	{
		zbx_snprintf(sql,sizeof(sql),"select value from %s where itemid=" ZBX_FS_UI64 " order by clock desc",
			table,
			item->itemid);
		result = DBselectN(sql, parameter);
		if(item->value_type == ITEM_VALUE_TYPE_UINT64)
		{
			while((row=DBfetch(result)))
			{
				ZBX_STR2UINT64(value_uint64,row[0]);
				sum_uint64+=value_uint64;
				rows++;
			}
			if(rows>0)	zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_UI64, sum_uint64);
		}
		else
		{
			while((row=DBfetch(result)))
			{
				sum+=atof(row[0]);
				rows++;
			}
			if(rows>0)	zbx_snprintf(value,MAX_STRING_LEN, ZBX_FS_DBL, sum);
		}
		if(0 == rows)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Result for SUM is empty" );
			res = FAIL;
		}
	}
	else
	{
		zabbix_log(LOG_LEVEL_WARNING, "Unknown flag [%d] Expected [%d] or [%d]",
			flag,
			ZBX_FLAG_SEC,
			ZBX_FLAG_VALUES);
		return	FAIL;
	}

	DBfree_result(result);

	return res;
}

/******************************************************************************
 *                                                                            *
 * Function: evaluate_AVG                                                     *
 *                                                                            *
 * Purpose: evaluate function 'avg' for the item                              *
 *                                                                            *
 * Parameters: item - item (performance metric)                               *
 *             parameter - number of seconds                                  *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, result is stored in 'value' *
 *               FAIL - failed to evaluate function                           *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int evaluate_AVG(char *value,DB_ITEM	*item,int parameter,int flag)
{
	DB_RESULT	result;
	DB_ROW		row;

	char		sql[MAX_STRING_LEN];
	int		now;
	int		res = SUCCEED;
	int		rows;
	double		sum=0;

	char		*table = NULL;
	char		table_ui64[] = "history_uint";
	char		table_float[] = "history";


	switch(item->value_type)
	{
		case ITEM_VALUE_TYPE_FLOAT:	table = table_float;	break;
		case ITEM_VALUE_TYPE_UINT64:	table = table_ui64;	break;
		default:
			return FAIL;
	}

	now=time(NULL);

	if(flag == ZBX_FLAG_SEC)
	{
		result = DBselect("select avg(value) from %s where clock>%d and itemid=" ZBX_FS_UI64,
			table,
			now-parameter,
			item->itemid);

		row = DBfetch(result);
		
		if(!row || DBis_null(row[0])==SUCCEED)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Result for AVG is empty" );
			res = FAIL;
		}
		else
		{
			strcpy(value,row[0]);
			del_zeroes(value);
		}
	}
	else if(flag == ZBX_FLAG_VALUES)
	{
		zbx_snprintf(sql,sizeof(sql),"select value from %s where itemid=" ZBX_FS_UI64 " order by clock desc",
			table,
			item->itemid);
		result = DBselectN(sql, parameter);
		rows=0;
		while((row=DBfetch(result)))
		{
			sum+=atof(row[0]);
			rows++;
		}
		if(rows == 0)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Result for AVG is empty" );
			res = FAIL;
		}
		else
		{
			zbx_snprintf(value,MAX_STRING_LEN, ZBX_FS_DBL, sum/(double)rows);
		}
	}
	else
	{
		zabbix_log(LOG_LEVEL_WARNING, "Unknown flag [%d] Expected [%d] or [%d]",
			flag,
			ZBX_FLAG_SEC,
			ZBX_FLAG_VALUES);
		return	FAIL;
	}

	DBfree_result(result);

	return res;
}

/******************************************************************************
 *                                                                            *
 * Function: evaluate_MIN                                                     *
 *                                                                            *
 * Purpose: evaluate function 'min' for the item                              *
 *                                                                            *
 * Parameters: item - item (performance metric)                               *
 *             parameter - number of seconds                                  *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, result is stored in 'value' *
 *               FAIL - failed to evaluate function                           *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int evaluate_MIN(char *value,DB_ITEM	*item,int parameter, int flag)
{
	DB_RESULT	result;
	DB_ROW		row;

	char		sql[MAX_STRING_LEN];
	int		now;
	int		rows;
	int		res = SUCCEED;

	char		*table = NULL;
	char		table_ui64[] = "history_uint";
	char		table_float[] = "history";

	zbx_uint64_t	min_uint64=0;
	zbx_uint64_t	l;

	double		min=0;
	double		f;

	switch(item->value_type)
	{
		case ITEM_VALUE_TYPE_FLOAT:	table = table_float;	break;
		case ITEM_VALUE_TYPE_UINT64:	table = table_ui64;	break;
		default:
			return FAIL;
	}

	now=time(NULL);


	if(flag == ZBX_FLAG_SEC)
	{
		result = DBselect("select min(value) from %s where clock>%d and itemid=" ZBX_FS_UI64,
			table,
			now-parameter,
			item->itemid);
		row = DBfetch(result);
		if(!row || DBis_null(row[0])==SUCCEED)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Result for MIN is empty" );
			res = FAIL;
		}
		else
		{
			strcpy(value,row[0]);
			del_zeroes(value);
		}
	}
	else if(flag == ZBX_FLAG_VALUES)
	{
		zbx_snprintf(sql,sizeof(sql),"select value from %s where itemid=" ZBX_FS_UI64 " order by clock desc",
			table,
			item->itemid);
		result = DBselectN(sql,parameter);

		rows=0;
		while((row=DBfetch(result)))
		{
			if(item->value_type == ITEM_VALUE_TYPE_UINT64)
			{
				ZBX_STR2UINT64(l,row[0]);

				if(rows==0)		min_uint64 = l;
				else if(l<min_uint64)	min_uint64 = l;
			}
			else
			{
				f=atof(row[0]);
				if(rows==0)	min = f;
				else if(f<min)	min = f;
			}
			rows++;
		}

		if(rows==0)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Result for MIN is empty" );
			res = FAIL;
		}
		else
		{
			if(item->value_type == ITEM_VALUE_TYPE_UINT64)
			{
				zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_UI64, min_uint64);
			}
			else
			{
				zbx_snprintf(value,MAX_STRING_LEN, ZBX_FS_DBL, min);
			}
		}
	}
	else
	{
		zabbix_log(LOG_LEVEL_WARNING, "Unknown flag [%d] Expected [%d] or [%d]",
			flag,
			ZBX_FLAG_SEC,
			ZBX_FLAG_VALUES);
		return	FAIL;
	}

	DBfree_result(result);

	return res;
}

/******************************************************************************
 *                                                                            *
 * Function: evaluate_MAX                                                     *
 *                                                                            *
 * Purpose: evaluate function 'max' for the item                              *
 *                                                                            *
 * Parameters: item - item (performance metric)                               *
 *             parameter - number of seconds                                  *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, result is stored in 'value' *
 *               FAIL - failed to evaluate function                           *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int evaluate_MAX(char *value,DB_ITEM *item,int parameter,int flag)
{
	DB_RESULT	result;
	DB_ROW		row;

	char		sql[MAX_STRING_LEN];
	int		now;
	int		res = SUCCEED;
	int		rows;
	double		f;
	double		max = 0;

	char		*table = NULL;
	char		table_ui64[] = "history_uint";
	char		table_float[] = "history";

	zbx_uint64_t	max_uint64=0;
	zbx_uint64_t	l;
	
	zabbix_log( LOG_LEVEL_DEBUG, "In evaluate_MAX()");

	switch(item->value_type)
	{
		case ITEM_VALUE_TYPE_FLOAT:	table = table_float;	break;
		case ITEM_VALUE_TYPE_UINT64:	table = table_ui64;	break;
		default:
			return FAIL;
	}

	now=time(NULL);

	if(flag == ZBX_FLAG_SEC)
	{
		result = DBselect("select max(value) from %s where clock>%d and itemid=" ZBX_FS_UI64,
			table,
			now-parameter,
			item->itemid);

		row = DBfetch(result);

		if(!row || DBis_null(row[0])==SUCCEED)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Result for MAX is empty" );
			res = FAIL;
		}
		else
		{
			strcpy(value,row[0]);
			del_zeroes(value);
		}
	}
	else if(flag == ZBX_FLAG_VALUES)
	{
		zbx_snprintf(sql,sizeof(sql),"select value from %s where itemid=" ZBX_FS_UI64 " order by clock desc",
			table,
			item->itemid);
		result = DBselectN(sql,parameter);
		rows=0;
		while((row=DBfetch(result)))
		{
			if(item->value_type == ITEM_VALUE_TYPE_UINT64)
			{
				ZBX_STR2UINT64(l,row[0]);

				if(rows==0)		max_uint64 = l;
				else if(l>max_uint64)	max_uint64 = l;
			}
			else
			{
				f=atof(row[0]);
				if(rows==0)	max=f;
				else if(f>max)	max=f;
			}
			rows++;
		}
		if(rows == 0)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Result for MAX is empty" );
			res = FAIL;
		}
		else
		{
			if(item->value_type == ITEM_VALUE_TYPE_UINT64)
			{
				zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_UI64, max_uint64);
			}
			else
			{
				zbx_snprintf(value,MAX_STRING_LEN, ZBX_FS_DBL, max);
			}
		}
	}
	else
	{
		zabbix_log(LOG_LEVEL_WARNING, "Unknown flag [%d] Expected [%d] or [%d]",
			flag,
			ZBX_FLAG_SEC,
			ZBX_FLAG_VALUES);
		return	FAIL;
	}

	DBfree_result(result);
	
	zabbix_log( LOG_LEVEL_DEBUG, "End of evaluate_MAX()");

	return res;
}

/******************************************************************************
 *                                                                            *
 * Function: evaluate_DELTA                                                   *
 *                                                                            *
 * Purpose: evaluate function 'delat' for the item                            *
 *                                                                            *
 * Parameters: item - item (performance metric)                               *
 *             parameter - number of seconds                                  *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, result is stored in 'value' *
 *               FAIL - failed to evaluate function                           *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int evaluate_DELTA(char *value,DB_ITEM *item,int parameter, int flag)
{
	DB_RESULT	result;
	DB_ROW		row;

	char		sql[MAX_STRING_LEN];
	int		now;
	int		res = SUCCEED;
	int		rows;
	double		f;
	double		min = 0,max = 0;

	zbx_uint64_t	max_uint64=0,min_uint64=0;
	zbx_uint64_t	l;
	
	char		*table = NULL;
	char		table_ui64[] = "history_uint";
	char		table_float[] = "history";

	zabbix_log( LOG_LEVEL_DEBUG, "In evaluate_DELTA()");

	switch(item->value_type)
	{
		case ITEM_VALUE_TYPE_FLOAT:	table = table_float;	break;
		case ITEM_VALUE_TYPE_UINT64:	table = table_ui64;	break;
		default:
			return FAIL;
	}

	now=time(NULL);

	if(flag == ZBX_FLAG_SEC)
	{
		result = DBselect("select max(value)-min(value) from %s where clock>%d and itemid=" ZBX_FS_UI64,
			table,
			now-parameter,
			item->itemid);

		row = DBfetch(result);
		if(!row || DBis_null(row[0])==SUCCEED)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Result for DELTA is empty" );
			res = FAIL;
		}
		else
		{
			strcpy(value,row[0]);
			del_zeroes(value);
		}
	}
	else if(flag == ZBX_FLAG_VALUES)
	{
		zbx_snprintf(sql,sizeof(sql),"select value from %s where itemid=" ZBX_FS_UI64 " order by clock desc",
			table,
			item->itemid);
		result = DBselectN(sql,parameter);
		rows=0;
		while((row=DBfetch(result)))
		{
			if(item->value_type == ITEM_VALUE_TYPE_UINT64)
			{
				ZBX_STR2UINT64(l,row[0]);

				if(rows==0)
				{
					max_uint64 = l;
					min_uint64 = l;
				}
				else 
				{
					if(l>max_uint64)	max_uint64 = l;
					if(l<min_uint64)	min_uint64 = l;
				}
			}
			else
			{
				f=atof(row[0]);
				if(rows==0)
				{
					min=f;
					max=f;
				}
				else
				{
					if(f>max)	max=f;
					if(f<min)	min=f;
				}
			}
			rows++;
		}
		if(rows==0)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "Result for DELTA is empty" );
			res = FAIL;
		}
		else
		{
			if(item->value_type == ITEM_VALUE_TYPE_UINT64)
			{
				zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_UI64, max_uint64-min_uint64);
			}
			else
			{
				zbx_snprintf(value,MAX_STRING_LEN, ZBX_FS_DBL, max-min);
			}
		}
	}
	else
	{
		zabbix_log(LOG_LEVEL_WARNING, "Unknown flag [%d] Expected [%d] or [%d]",
			flag,
			ZBX_FLAG_SEC,
			ZBX_FLAG_VALUES);
		return	FAIL;
	}

	DBfree_result(result);

	zabbix_log( LOG_LEVEL_DEBUG, "End of evaluate_DELTA()");

	return res;
}

/******************************************************************************
 *                                                                            *
 * Function: evaluate_NODATA                                                  *
 *                                                                            *
 * Purpose: evaluate function 'nodata' for the item                           *
 *                                                                            *
 * Parameters: item - item (performance metric)                               *
 *             parameter - number of seconds                                  *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, result is stored in 'value' *
 *               FAIL - failed to evaluate function                           *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int evaluate_NODATA(char *value,DB_ITEM	*item,int parameter)
{
	int		now;
	int		res = SUCCEED;

	zabbix_log( LOG_LEVEL_DEBUG, "In evaluate_NODATA()");

	now = time(NULL);

	if((CONFIG_SERVER_STARTUP_TIME + parameter > now) || (item->lastclock + parameter > now))
	{
		strcpy(value,"0");
	}
	else
	{
		strcpy(value,"1");
	}

	zabbix_log( LOG_LEVEL_DEBUG, "End of evaluate_NODATA()");

	return res;
}

int convertj(char *inputstr, char *outputstr, size_t outputlength, const char *encoding)
{
#define ZBX_ENCODING struct zbx_encoding_t
ZBX_ENCODING
{
	char	*zbx_encoding;
	char	*iconv_encoding;
};

ZBX_ENCODING e[]=
{
		{"cp932",	"CP932"},
		{"ujis",	"UJIS"},
		{"sjis",	"SHIFT-JIS"},
		{"eucjpms",	"EUC-JP-MS"},
		{NULL}
};
	iconv_t		cd;
	size_t		inputlength, resultlength;
	int		i, res = SUCCEED;
	const char	*from_code = NULL;
	const char	*to_code = {"UTF-8"};

	for (i = 0; e[i].zbx_encoding != NULL; i ++)
		if (0 == strcmp(encoding, e[i].zbx_encoding)) {
			from_code = e[i].iconv_encoding;
			break;
		}
	if (NULL == from_code)
		return FAIL;

	cd = iconv_open(to_code, from_code);
	if (cd == (iconv_t)-1) {
		zabbix_log(LOG_LEVEL_DEBUG, "iconv_open(to_code:%s,from_code:%s) failed",
				to_code,
				from_code);
		return FAIL;
	}
		
	inputlength = strlen(inputstr);

	resultlength = iconv(cd, &inputstr, &inputlength, &outputstr, &outputlength);
	if (resultlength == (size_t)-1) {
		zabbix_log(LOG_LEVEL_DEBUG, "iconv(inputstr:%s,to_code:%s,from_code:%s) failed",
				inputstr,
				to_code,
				from_code);
		res = FAIL;
	}
	iconv_close(cd);

	zabbix_log(LOG_LEVEL_DEBUG, "convertj() [from_code:%s] [to_code:%s] [inputstr:%s] [outputstr:%s]", from_code, to_code, inputstr, outputstr);

	return res;
}

/******************************************************************************
 *                                                                            *
 * Function: evaluate_function                                                *
 *                                                                            *
 * Purpose: evaluate function                                                 *
 *                                                                            *
 * Parameters: item - item to calculate function for                          *
 *             function - function (for example, 'max')                       *
 *             parameter - parameter of the function)                         *
 *             flag - if EVALUATE_FUNCTION_SUFFIX, then include units and     *
 *                    suffix (K,M,G) into result value (for example, 15GB)    *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, value contains its value    *
 *               FAIL - evaluation failed                                     *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
int evaluate_function(char *value,DB_ITEM *item,char *function,char *parameter)
{
	int	ret  = SUCCEED;
	time_t  now;
	struct  tm      *tm;
	
	int	fuzlow, fuzhig;

	int	day;
	int	len;

	char    encoding[MAX_STRING_LEN];
	char    lastvalue_str[MAX_STRING_LEN];

	zabbix_log( LOG_LEVEL_DEBUG, "In evaluate_function(%s)",
		function);

	if(strcmp(function,"last")==0)
	{
		if(item->lastvalue_null==1)
		{
			ret = FAIL;
		}
		else
		{
			switch (item->value_type) {
				case ITEM_VALUE_TYPE_FLOAT:
					zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_DBL,item->lastvalue_dbl);
					del_zeroes(value);
					break;
				case ITEM_VALUE_TYPE_UINT64:
					zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_UI64,item->lastvalue_uint64);
					break;
				default:
					strcpy(value,item->lastvalue_str);
					break;
			}
		}
	}
	else if(strcmp(function,"prev")==0)
	{
		if(item->prevvalue_null==1)
		{
			ret = FAIL;
		}
		else
		{
			switch (item->value_type) {
				case ITEM_VALUE_TYPE_FLOAT:
					zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_DBL,item->prevvalue_dbl);
					del_zeroes(value);
					break;
				case ITEM_VALUE_TYPE_UINT64:
					zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_UI64,item->prevvalue_uint64);
					break;
				default:
					strcpy(value,item->prevvalue_str);
					break;
			}
		}
	}
	else if(strcmp(function,"min")==0)
	{
		if(parameter[0]=='#')
			ret = evaluate_MIN(value,item,atoi(parameter+1),ZBX_FLAG_VALUES);
		else
			ret = evaluate_MIN(value,item,atoi(parameter),ZBX_FLAG_SEC);
	}
	else if(strcmp(function,"max")==0)
	{
		if(parameter[0]=='#')
			ret = evaluate_MAX(value,item,atoi(parameter+1),ZBX_FLAG_VALUES);
		else
			ret = evaluate_MAX(value,item,atoi(parameter),ZBX_FLAG_SEC);
	}
	else if(strcmp(function,"avg")==0)
	{
		if(parameter[0]=='#')
			ret = evaluate_AVG(value,item,atoi(parameter+1),ZBX_FLAG_VALUES);
		else
			ret = evaluate_AVG(value,item,atoi(parameter),ZBX_FLAG_SEC);
	}
	else if(strcmp(function,"sum")==0)
	{
		if(parameter[0]=='#')
			ret = evaluate_SUM(value,item,atoi(parameter+1),ZBX_FLAG_VALUES);
		else
			ret = evaluate_SUM(value,item,atoi(parameter),ZBX_FLAG_SEC);
	}
	else if(strcmp(function,"count")==0)
	{
		ret = evaluate_COUNT(value,item,parameter);
	}
	else if(strcmp(function,"delta")==0)
	{
		if(parameter[0]=='#')
			ret = evaluate_DELTA(value,item,atoi(parameter+1),ZBX_FLAG_VALUES);
		else
			ret = evaluate_DELTA(value,item,atoi(parameter),ZBX_FLAG_SEC);
	}
	else if(strcmp(function,"nodata")==0)
	{
		ret = evaluate_NODATA(value,item,atoi(parameter));
	}
	else if(strcmp(function,"date")==0)
	{
		now=time(NULL);
                tm=localtime(&now);
                zbx_snprintf(value,MAX_STRING_LEN,"%.4d%.2d%.2d",
			tm->tm_year+1900,
			tm->tm_mon+1,
			tm->tm_mday);
	}
	else if(strcmp(function,"dayofweek")==0)
	{
		now=time(NULL);
                tm=localtime(&now);
		/* The number of days since Sunday, in the range 0 to 6. */
		day=tm->tm_wday;
		if(0 == day)	day=7;
                zbx_snprintf(value,MAX_STRING_LEN,"%d",
			day);
	}
	else if(strcmp(function,"time")==0)
	{
		now=time(NULL);
                tm=localtime(&now);
                zbx_snprintf(value,MAX_STRING_LEN,"%.2d%.2d%.2d",
			tm->tm_hour,
			tm->tm_min,
			tm->tm_sec);
	}
	else if(strcmp(function,"abschange")==0)
	{
		if((item->lastvalue_null==1)||(item->prevvalue_null==1))
		{
			ret = FAIL;
		}
		else
		{
			switch (item->value_type) {
				case ITEM_VALUE_TYPE_FLOAT:
					zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_DBL,
						(double)abs(item->lastvalue_dbl-item->prevvalue_dbl));
					del_zeroes(value);
					break;
				case ITEM_VALUE_TYPE_UINT64:
					/* To avoid overflow */
					if(item->lastvalue_uint64>=item->prevvalue_uint64)
					{
						zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_UI64,
							labs(item->lastvalue_uint64-item->prevvalue_uint64));
					}
					else
					{
						zbx_snprintf(value,MAX_STRING_LEN,"-" ZBX_FS_UI64,
							labs(item->prevvalue_uint64 - item->lastvalue_uint64));
					}
					break;
				default:
					if(strcmp(item->lastvalue_str, item->prevvalue_str) == 0)
					{
						strcpy(value,"0");
					}
					else
					{
						strcpy(value,"1");
					}
					break;
			}
		}
	}
	else if(strcmp(function,"change")==0)
	{
		if((item->lastvalue_null==1)||(item->prevvalue_null==1))
		{
			ret = FAIL;
		}
		else
		{
			switch (item->value_type) {
				case ITEM_VALUE_TYPE_FLOAT:
					zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_DBL,
						item->lastvalue_dbl-item->prevvalue_dbl);
					del_zeroes(value);
					break;
				case ITEM_VALUE_TYPE_UINT64:
					/* To avoid overflow */
					if(item->lastvalue_uint64>=item->prevvalue_uint64)
					{
						zbx_snprintf(value,MAX_STRING_LEN,ZBX_FS_UI64,
							item->lastvalue_uint64-item->prevvalue_uint64);
					}
					else
					{
						zbx_snprintf(value,MAX_STRING_LEN,"-" ZBX_FS_UI64,
							item->prevvalue_uint64 - item->lastvalue_uint64);
					}
					break;
				default:
					if(strcmp(item->lastvalue_str, item->prevvalue_str) == 0)
					{
						strcpy(value,"0");
					}
					else
					{
						strcpy(value,"1");
					}
					break;
			}
		}
	}
	else if(strcmp(function,"diff")==0)
	{
		if((item->lastvalue_null==1)||(item->prevvalue_null==1))
		{
			ret = FAIL;
		}
		else
		{
			switch (item->value_type) {
				case ITEM_VALUE_TYPE_FLOAT:
					if(cmp_double(item->lastvalue_dbl, item->prevvalue_dbl) == 0)
					{
						strcpy(value,"0");
					}
					else
					{
						strcpy(value,"1");
					}
					break;
				case ITEM_VALUE_TYPE_UINT64:
					if(item->lastvalue_uint64 == item->prevvalue_uint64)
					{
						strcpy(value,"0");
					}
					else
					{
						strcpy(value,"1");
					}
					break;
				default:
					if(strcmp(item->lastvalue_str, item->prevvalue_str) == 0)
					{
						strcpy(value,"0");
					}
					else
					{
						strcpy(value,"1");
					}
					break;
			}
/*			if( (item->value_type==ITEM_VALUE_TYPE_FLOAT) || (item->value_type==ITEM_VALUE_TYPE_UINT64))
			{
				if(cmp_double(item->lastvalue, item->prevvalue) == 0)
				{
					strcpy(value,"0");
				}
				else
				{
					strcpy(value,"1");
				}
			}
			else
			{
				if(strcmp(item->lastvalue_str, item->prevvalue_str) == 0)
				{
					strcpy(value,"0");
				}
				else
				{
					strcpy(value,"1");
				}
			}*/
		}
	}
	else if(strcmp(function,"str")==0)
	{
		if( (item->value_type==ITEM_VALUE_TYPE_STR) || (item->value_type==ITEM_VALUE_TYPE_LOG))
		{
			if(item->lastvalue_null==0 && strstr(item->lastvalue_str, parameter) != NULL)
			{
				strcpy(value,"1");
			}
			else
			{
				strcpy(value,"0");
			}

		}
		else
		{
			ret = FAIL;
		}
	}
	else if(strcmp(function,"regexp")==0)
	{
		if( (item->value_type==ITEM_VALUE_TYPE_STR) || (item->value_type==ITEM_VALUE_TYPE_LOG))
		{
			if(item->lastvalue_null==0)
			{
				encoding[0]='\0';
				get_key_param(item->key, 3, encoding, sizeof(encoding));

				if (FAIL == convertj(item->lastvalue_str, lastvalue_str, sizeof(lastvalue_str), encoding))
					zbx_strlcpy(lastvalue_str, item->lastvalue_str, sizeof(lastvalue_str));

				//if(zbx_regexp_match(item->lastvalue_str, parameter, &len) != NULL)
				if(zbx_regexp_match(lastvalue_str, parameter, &len) != NULL)
				{
					strcpy(value,"1");
				}
				else
				{
					strcpy(value,"0");
				}
			}
			else
			{
				strcpy(value,"0");
			}
		}
		else
		{
			ret = FAIL;
		}
	}
	else if(strcmp(function,"iregexp")==0)
	{
		if( (item->value_type==ITEM_VALUE_TYPE_STR) || (item->value_type==ITEM_VALUE_TYPE_LOG))
		{
			if(item->lastvalue_null==0)
			{
				encoding[0]='\0';
				get_key_param(item->key, 3, encoding, sizeof(encoding));

				if (FAIL == convertj(item->lastvalue_str, lastvalue_str, sizeof(lastvalue_str), encoding))
					zbx_strlcpy(lastvalue_str, item->lastvalue_str, sizeof(lastvalue_str));

				//if(zbx_iregexp_match(item->lastvalue_str, parameter, &len) != NULL)
				if(zbx_iregexp_match(lastvalue_str, parameter, &len) != NULL)
				{
					strcpy(value,"1");
				}
				else
				{
					strcpy(value,"0");
				}
			}
			else
			{
				strcpy(value,"0");
			}
		}
		else
		{
			ret = FAIL;
		}
	}
	else if(strcmp(function,"now")==0)
	{
		now=time(NULL);
                zbx_snprintf(value,MAX_STRING_LEN,"%d",(int)now);
	}
	else if(strcmp(function,"fuzzytime")==0)
	{
		now=time(NULL);
		fuzlow=(int)(now-atoi(parameter));
		fuzhig=(int)(now+atoi(parameter));

		if(item->lastvalue_null==1)
		{
				ret = FAIL;
		}
		else
		{
			switch (item->value_type) {
				case ITEM_VALUE_TYPE_FLOAT:
					if((item->lastvalue_dbl>=fuzlow)&&(item->lastvalue_dbl<=fuzhig))
					{
						strcpy(value,"1");
					}
					else
					{
						strcpy(value,"0");
					}
					break;
				case ITEM_VALUE_TYPE_UINT64:
					if((item->lastvalue_uint64>=fuzlow)&&(item->lastvalue_uint64<=fuzhig))
					{
						strcpy(value,"1");
					}
					else
					{
						strcpy(value,"0");
					}
					break;
				default:
					ret = FAIL;
					break;
			}
		}
	}
	else if(strcmp(function,"logseverity")==0)
	{
		ret = evaluate_LOGSEVERITY(value,item,parameter);
	}
	else if(strcmp(function,"logsource")==0)
	{
		ret = evaluate_LOGSOURCE(value,item,parameter);
	}
	else
	{
		zabbix_log( LOG_LEVEL_WARNING, "Unsupported function:%s",
			function);
		zabbix_syslog("Unsupported function:%s",
			function);
		ret = FAIL;
	}

	zabbix_log( LOG_LEVEL_DEBUG, "End of evaluate_function(result:%s)",
		value);
	return ret;
}

/******************************************************************************
 *                                                                            *
 * Function: add_value_suffix_uptime                                          *
 *                                                                            *
 * Purpose: Peocess suffix 'uptime'                                           *
 *                                                                            *
 * Parameters: value - value for adjusting                                    *
 *             max_len - max len of the value                                 *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static void	add_value_suffix_uptime(char *value, int max_len)
{
	double	value_double;
	double	days, hours, min;

	zabbix_log( LOG_LEVEL_DEBUG, "In add_value_suffix_uptime(%s)",
		value);

	value_double = atof(value);

	if(value_double <0)	return;

	days=floor(value_double/(24*3600));
	if(cmp_double(days,0) != 0)
	{
		value_double=value_double-days*(24*3600);
	}
	hours=floor(value_double/(3600));
	if(cmp_double(hours,0) != 0)
	{
		value_double=value_double-hours*3600;
	}
	min=floor(value_double/(60));
	if( cmp_double(min,0) !=0)
	{
		value_double=value_double-min*(60);
	}
	if(cmp_double(days,0) == 0)
	{
		zbx_snprintf(value, max_len, "%02d:%02d:%02d",
			(int)hours,
			(int)min,
			(int)value_double);
	}
	else
	{
		zbx_snprintf(value, max_len, "%d days, %02d:%02d:%02d",
			(int)days,
			(int)hours,
			(int)min,
			(int)value_double);
	}
	zabbix_log( LOG_LEVEL_DEBUG, "End of add_value_suffix_uptime(%s)",
		value);
}

/******************************************************************************
 *                                                                            *
 * Function: add_value_suffix_s                                               *
 *                                                                            *
 * Purpose: Peocess suffix 's'                                                *
 *                                                                            *
 * Parameters: value - value for adjusting                                    *
 *             max_len - max len of the value                                 *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static void	add_value_suffix_s(char *value, int max_len)
{
	double	value_double;
	double	t;
	char	tmp[MAX_STRING_LEN];

	zabbix_log( LOG_LEVEL_DEBUG, "In add_value_suffix_s(%s)",
		value);

	value_double = atof(value);
	if(value_double <0)	return;

	value[0]='\0';

	t=floor(value_double/(365*24*3600));
	if(cmp_double(t,0) != 0)
	{
		zbx_snprintf(tmp, sizeof(tmp), "%dy", (int)t);
		zbx_strlcat(value, tmp, max_len);
		value_double = value_double-t*(365*24*3600);
	}

	t=floor(value_double/(30*24*3600));
	if(cmp_double(t,0) != 0)
	{
		zbx_snprintf(tmp, sizeof(tmp), "%dm", (int)t);
		zbx_strlcat(value, tmp, max_len);
		value_double = value_double-t*(30*24*3600);
	}

	t=floor(value_double/(24*3600));
	if(cmp_double(t,0) != 0)
	{
		zbx_snprintf(tmp, sizeof(tmp), "%dd", (int)t);
		zbx_strlcat(value, tmp, max_len);
		value_double = value_double-t*(24*3600);
	}

	t=floor(value_double/(3600));
	if(cmp_double(t,0) != 0)
	{
		zbx_snprintf(tmp, sizeof(tmp), "%dh", (int)t);
		zbx_strlcat(value, tmp, max_len);
		value_double = value_double-t*(3600);
	}

	t=floor(value_double/(60));
	if(cmp_double(t,0) != 0)
	{
		zbx_snprintf(tmp, sizeof(tmp), "%dm", (int)t);
		zbx_strlcat(value, tmp, max_len);
		value_double = value_double-t*(60);
	}

	zbx_snprintf(tmp, sizeof(tmp), "%02.2f", value_double);
	zbx_rtrim(tmp,"0");
	zbx_rtrim(tmp,".");
	zbx_strlcat(tmp, "s", sizeof(tmp));
	zbx_strlcat(value, tmp, max_len);

	zabbix_log( LOG_LEVEL_DEBUG, "End of add_value_suffix_s(%s)",
		value);
}

/******************************************************************************
 *                                                                            *
 * Function: add_value_suffix_normsl                                          *
 *                                                                            *
 * Purpose: Peocess normal values and add K,M,G,T                             *
 *                                                                            *
 * Parameters: value - value for adjusting                                    *
 *             max_len - max len of the value                                 *
 *             units - units (bps, b,B, etc)                                  *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static void	add_value_suffix_normal(char *value, int max_len, char *units)
{
	double	base = 1024;
	char	kmgt[MAX_STRING_LEN];

	zbx_uint64_t	value_uint64;
	double		value_double;

	zabbix_log( LOG_LEVEL_DEBUG, "In add_value_normal(value:%s,units:%s)",
		value,
		units);

	ZBX_STR2UINT64(value_uint64, value);

/*	value_uint64 = llabs(zbx_atoui64(value));*/

	/* SPecial processing for bits */
	if(strcmp(units,"b") == 0 || strcmp(units,"bps") == 0)
	{
		base = 1000;
	}

	if(value_uint64 < base)
	{
		strscpy(kmgt,"");
		value_double = (double)value_uint64;
	}
	else if(value_uint64 < base*base)
	{
		strscpy(kmgt,"K");
		value_double = (double)value_uint64/base;
	}
	else if(value_uint64 < base*base*base)
	{
		strscpy(kmgt,"M");
		value_double = (double)(value_uint64/(base*base));
	}
	else if(value_uint64 < base*base*base*base)
	{
		strscpy(kmgt,"G");
		value_double = (double)value_uint64/(base*base*base);
	}
	else
	{
		strscpy(kmgt,"T");
		value_double = (double)value_uint64/(base*base*base*base);
	}

	if(cmp_double((int)(value_double+0.5), value_double) == 0)
	{
		zbx_snprintf(value, MAX_STRING_LEN, ZBX_FS_DBL_EXT(0) " %s%s",
			value_double,
			kmgt,
			units);
	}
	else
	{
		zbx_snprintf(value, MAX_STRING_LEN, ZBX_FS_DBL_EXT(2) " %s%s",
			value_double,
			kmgt,
			units);
	}

	zabbix_log( LOG_LEVEL_DEBUG, "End of add_value_normal(value:%s)",
		value);
}

/******************************************************************************
 *                                                                            *
 * Function: add_value_suffix                                                 *
 *                                                                            *
 * Purpose: Add suffix for value                                              *
 *                                                                            *
 * Parameters: value - value to replacing                                     *
 *             valuemapid - index of value map                                *
 *                                                                            *
 * Return value: SUCCEED - suffix added succesfully, value contains new value *
 *               FAIL - adding failed, value contains old value               *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/

/* Do not forget to keep it in sync wiht convert_units in config.inc.php */
int	add_value_suffix(char *value, int max_len, char *units, int value_type)
{
	int	ret = FAIL;

        struct  tm *local_time = NULL;
	time_t	time;

	char	tmp[MAX_STRING_LEN];

	zabbix_log( LOG_LEVEL_DEBUG, "In add_value_suffix(value:%s,units:%s)",
		value,
		units);

	switch(value_type)
	{
	case	ITEM_VALUE_TYPE_FLOAT:
		if(strcmp(units,"s") == 0)
		{
			add_value_suffix_s(value, max_len);
			ret = SUCCEED;
		}
		else if(strcmp(units,"uptime") == 0)
		{
			add_value_suffix_uptime(value, max_len);
			ret = SUCCEED;
		}
		else if(strlen(units) != 0)
		{
			add_value_suffix_normal(value, max_len, units);
			ret = SUCCEED;
		}
		else
		{
			/* Do nothing if units not set */
		}
		break;

	case	ITEM_VALUE_TYPE_UINT64:
		if(strcmp(units,"s") == 0)
		{
			add_value_suffix_s(value, max_len);
			ret = SUCCEED;
		}
		else if(strcmp(units,"unixtime") == 0)
		{
			time = (time_t)zbx_atoui64(value);
			local_time = localtime(&time);
			strftime(tmp, MAX_STRING_LEN, "%Y.%m.%d %H:%M:%S",
				local_time);
			zbx_strlcpy(value, tmp, max_len);
			ret = SUCCEED;
		}
		else if(strcmp(units,"uptime") == 0)
                {
			add_value_suffix_uptime(value, max_len);
			ret = SUCCEED;
		}
		else if(strlen(units) != 0)
		{
			add_value_suffix_normal(value, max_len, units);
			ret = SUCCEED;
		}
		else
		{
			/* Do nothing if units not set */
		}
		break;
	default:
		ret = FAIL;
		break;
	}

	zabbix_log(LOG_LEVEL_DEBUG, "End of add_value_suffix(%s)",
		value);

	return ret;
}

/******************************************************************************
 *                                                                            *
 * Function: replace_value_by_map                                             *
 *                                                                            *
 * Purpose: replace value by mapping value                                    *
 *                                                                            *
 * Parameters: value - value to replacing                                     *
 *             valuemapid - index of value map                                *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, value contains new value    *
 *               FAIL - evaluation failed, value contains old value           *
 *                                                                            *
 * Author: Eugene Grigorjev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
int	replace_value_by_map(char *value, zbx_uint64_t valuemapid)
{
	DB_RESULT	result;
	DB_ROW		row;

	char new_value[MAX_STRING_LEN];
	char sql[MAX_STRING_LEN];
	char *or_value;

	zabbix_log(LOG_LEVEL_DEBUG, "In replace_value_by_map()" );
	
	if(valuemapid == 0)	return FAIL;
	
	result = DBselect("select newvalue from mappings where valuemapid=" ZBX_FS_UI64 " and value='%s'",
			valuemapid,
			value);
	row = DBfetch(result);

	if(!row || DBis_null(row[0])==SUCCEED)		return FAIL;

	strcpy(new_value,row[0]);
	DBfree_result(result);

	del_zeroes(new_value);
	or_value = sql;	/* sql variarbvle used as tmp - original value */
	zbx_strlcpy(sql,value,MAX_STRING_LEN);
	
	zbx_snprintf(value, MAX_STRING_LEN, "%s (%s)",
		new_value,
		or_value);

	zabbix_log(LOG_LEVEL_DEBUG, "End replace_value_by_map(result:%s)",
		value);
	return SUCCEED;
}

/******************************************************************************
 *                                                                            *
 * Function: evaluate_function2                                               *
 *                                                                            *
 * Purpose: evaluate function                                                 *
 *                                                                            *
 * Parameters: host - host the key belongs to                                 *
 *             key - item's key (for example, 'max')                          *
 *             function - function (for example, 'max')                       *
 *             parameter - parameter of the function)                         *
 *                                                                            *
 * Return value: SUCCEED - evaluated succesfully, value contains its value    *
 *               FAIL - evaluation failed                                     *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments: Used for evaluation of notification macros                       *
 *                                                                            *
 ******************************************************************************/
int evaluate_function2(char *value,char *host,char *key,char *function,char *parameter)
{
	DB_ITEM	item;
	DB_RESULT result;
	DB_ROW	row;

	char	host_esc[MAX_STRING_LEN];
	char	key_esc[MAX_STRING_LEN];

	int	res;

	zabbix_log(LOG_LEVEL_DEBUG, "In evaluate_function2(%s,%s,%s,%s)",
		host,
		key,
		function,
		parameter);

	DBescape_string(host, host_esc, MAX_STRING_LEN);
	DBescape_string(key, key_esc, MAX_STRING_LEN);

	result = DBselect("select %s where h.host='%s' and h.hostid=i.hostid and i.key_='%s' and" ZBX_COND_NODEID,
		ZBX_SQL_ITEM_SELECT,
		host_esc,
		key_esc,
		LOCAL_NODE("h.hostid"));

	row = DBfetch(result);

	if(!row)
	{
        	DBfree_result(result);
		zabbix_log(LOG_LEVEL_WARNING, "Query returned empty result");
		zabbix_syslog("Query returned empty result");
		return FAIL;
	}

	DBget_item_from_db(&item,row);

	res = evaluate_function(value,&item,function,parameter);

	if(replace_value_by_map(value, item.valuemapid) != SUCCEED)
	{
		add_value_suffix(value, MAX_STRING_LEN, item.units, item.value_type);
	}

/* Cannot call DBfree_result until evaluate_FUNC */
	DBfree_result(result);

	zabbix_log(LOG_LEVEL_DEBUG, "End evaluate_function2(result:%s)",
		value);
	return res;
}
