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
#include "db.h"
#include "log.h"
#include "daemon.h"
#include "zbxjson.h"

#include "proxyconfig.h"
#include "../servercomms.h"

#define CONFIG_PROXYCONFIG_RETRY 120 /* seconds */

/******************************************************************************
 *                                                                            *
 * Function: process_proxyconfig_table                                        *
 *                                                                            *
 * Purpose: update configuration table                                        *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value: SUCCESS - processed succesfully                              * 
 *               FAIL - an error occured                                      *
 *                                                                            *
 * Author: Aleksander Vladishev                                               *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int	process_proxyconfig_table(struct zbx_json_parse *jp, const char *tablename, const char *p)
{
	int			t, f, field_count, insert, offset, execute;
	ZBX_TABLE		*table = NULL;
	ZBX_FIELD		*fields[ZBX_MAX_FIELDS];
	struct zbx_json_parse	jp_obj, jp_data, jp_row;
	char			buf[MAX_STRING_LEN],
				sql[MAX_STRING_LEN],
				esc[MAX_STRING_LEN],
				recid[21]; /* strlen("18446744073709551615") == 20 */
	const char		*pf;
	DB_RESULT		result;

	zabbix_log(LOG_LEVEL_DEBUG, "In process_proxyconfig_table() [tablename:%s]",
			tablename);

	for (t = 0; tables[t].table != NULL; t++ )
		if (0 == strcmp(tables[t].table, tablename))
			table = &tables[t];

	if (NULL == table) {
		zabbix_log(LOG_LEVEL_WARNING, "Invalid table name \"%s\"",
				tablename);
		return FAIL;
	}

	if (ZBX_DB_OK > DBexecute("create table %1$s_%2$s_tmp (%2$s "ZBX_DBTYPE_INT64")",
			table->table,
			table->recid))
		goto db_error;

/* {"hosts":{"fields":["hostid","host",...],"data":[[1,"zbx01",...],[2,"zbx02",...],...]},"items":{...},...} 
 *          ^---------------------------------------------------------------------------^
 */	if (FAIL == zbx_json_brackets_open(p, &jp_obj))
		goto json_error;

/* {"hosts":{"fields":["hostid","host",...],"data":[[1,"zbx01",...],[2,"zbx02",...],...]},"items":{...},...} 
 *                    ^
 */	if (NULL == (p = zbx_json_pair_by_name(&jp_obj, "fields"))) {
		zabbix_log(LOG_LEVEL_WARNING, "Can't find \"fields\" pair");
		return FAIL;
	}

/* {"hosts":{"fields":["hostid","host",...],"data":[[1,"zbx01",...],[2,"zbx02",...],...]},"items":{...},...} 
 *                    ^-------------------^
 */	if (FAIL == zbx_json_brackets_open(p, &jp_data))
		goto json_error;

	p = NULL;
	field_count = 0;
	while (NULL != (p = zbx_json_next(&jp_data, p))) {
		if (NULL == (p = zbx_json_decodevalue(p, buf, sizeof(buf))))
			goto json_error;

		fields[field_count] = NULL;
		for(f = 0; table->fields[f].name != NULL; f++)
			if (0 == strcmp(table->fields[f].name, buf)) {
				fields[field_count] = &table->fields[f];
				break;
			}

		if (NULL == fields[field_count]) {
			zabbix_log(LOG_LEVEL_WARNING, "Invalid field name \"%s\"",
					buf);
			return FAIL;
		}
		field_count++;
	}

/* {"hosts":{"fields":["hostid","host",...],"data":[[1,"zbx01",...],[2,"zbx02",...],...]},"items":{...},...} 
 *                                                 ^
 */	if (NULL == (p = zbx_json_pair_by_name(&jp_obj, "data"))) {
		zabbix_log(LOG_LEVEL_WARNING, "Can't find \"data\" pair");
		return FAIL;
	}

/* {"hosts":{"fields":["hostid","host",...],"data":[[1,"zbx01",...],[2,"zbx02",...],...]},"items":{...},...} 
 *                                                 ^-----------------------------------^
 */	if (FAIL == zbx_json_brackets_open(p, &jp_data))
		goto json_error;

/* {"hosts":{"fields":["hostid","host",...],"data":[[1,"zbx01",...],[2,"zbx02",...],...]},"items":{...},...} 
 *                                                  ^
 */	p = NULL;
	while (NULL != (p = zbx_json_next(&jp_data, p))) {
/* {"hosts":{"fields":["hostid","host",...],"data":[[1,"zbx01",...],[2,"zbx02",...],...]},"items":{...},...} 
 *                                                  ^-------------^
 */		if (FAIL == zbx_json_brackets_open(p, &jp_row))
			goto json_error;

		pf = NULL;
		if (NULL == (pf = zbx_json_next_value(&jp_row, pf, recid, sizeof(recid))))
			goto json_error;

		result = DBselect("select 0 from %s where %s=%s",
				table->table,
				table->recid,
				recid);
		insert = (NULL == DBfetch(result));
		DBfree_result(result);

		offset = 0;
		if (insert) {
			offset += zbx_snprintf(sql + offset, sizeof(sql) - offset, "insert into %s (",
					table->table);
			for (f = 0; f < field_count; f ++)
				offset += zbx_snprintf(sql + offset, sizeof(sql) - offset, "%s,", fields[f]->name);
			offset--;
			offset += zbx_snprintf(sql + offset, sizeof(sql) - offset, ") values (%s,",
					recid);
		} else
			offset += zbx_snprintf(sql + offset, sizeof(sql) - offset, "update %s set ",
					table->table);

		execute = 0;

/* {"hosts":{"fields":["hostid","host",...],"data":[[1,"zbx01",...],[2,"zbx02",...],...]},"items":{...},...} 
 *                                                   ^
 */		f = 1;
		while (NULL != (pf = zbx_json_next_value(&jp_row, pf, buf, sizeof(buf)))) {
			execute = 1;

			if (f == field_count) {
				zabbix_log(LOG_LEVEL_WARNING, "Invalid number of fields \"%.*s\"",
						jp_row.end - jp_row.start + 1,
						jp_row.start);
				return FAIL;
			}

			if (fields[f]->type == ZBX_TYPE_INT || fields[f]->type == ZBX_TYPE_UINT || fields[f]->type == ZBX_TYPE_ID || fields[f]->type == ZBX_TYPE_FLOAT) {
				if (0 == insert)
					offset += zbx_snprintf(sql + offset, sizeof(sql) - offset, "%s=", fields[f]->name);
				offset += zbx_snprintf(sql + offset, sizeof(sql) - offset, "%s,", buf);
			} else {
				DBescape_string(buf, esc, sizeof(esc));
				if (0 == insert)
					offset += zbx_snprintf(sql + offset, sizeof(sql) - offset, "%s=", fields[f]->name);
				offset += zbx_snprintf(sql + offset, sizeof(sql) - offset, "'%s',", esc);
			}
			f++;
		}

		if (f != field_count) {
			zabbix_log(LOG_LEVEL_WARNING, "Invalid number of fields \"%.*s\"",
					jp_row.end - jp_row.start + 1,
					jp_row.start);
			return FAIL;
		}

		offset--;
		if (insert)
			offset += zbx_snprintf(sql + offset, sizeof(sql) - offset, ")");
		else
			offset += zbx_snprintf(sql + offset, sizeof(sql) - offset, " where %s=%s",
					table->recid,
					recid);

		if ((insert || execute) && ZBX_DB_OK > DBexecute("%s", sql))
			goto db_error;
		if (ZBX_DB_OK > DBexecute("insert into %1$s_%2$s_tmp (%2$s) values (%3$s)",
				table->table,
				table->recid,
				recid))
			goto db_error;
	}

	if (ZBX_DB_OK > DBexecute("delete from %1$s where not %2$s in (select %2$s from %1$s_%2$s_tmp)",
			table->table,
			table->recid))
		goto db_error;

	if (ZBX_DB_OK > DBexecute("drop table %s_%s_tmp",
			table->table,
			table->recid))
		goto db_error;

	zabbix_log(LOG_LEVEL_DEBUG, "End process_proxyconfig_table()");

	return SUCCEED;
json_error:
	zabbix_log(LOG_LEVEL_DEBUG, "Can't proceed table \"%s\". %s",
			tablename,
			zbx_json_strerror());
db_error:
	return FAIL;
}

/******************************************************************************
 *                                                                            *
 * Function: process_proxyconfig                                              *
 *                                                                            *
 * Purpose: update configuration                                              *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value: SUCCESS - processed succesfully                              * 
 *               FAIL - an error occured                                      *
 *                                                                            *
 * Author: Aleksander Vladishev                                               *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static void	process_proxyconfig(struct zbx_json_parse *jp)
{
	char		buf[MAX_STRING_LEN];
	size_t		len = sizeof(buf);
	const char	*p = NULL;
	int		res = SUCCEED;

	zabbix_log(LOG_LEVEL_DEBUG, "In process_proxyconfig()");

	DBbegin();
/*
 * {"hosts":{"fields":["hostid","host",...],"data":[[1,"zbx01",...],[2,"zbx02",...],...]},"items":{...},...} 
 *          ^
 */	while (NULL != (p = zbx_json_pair_next(jp, p, buf, len)) && res == SUCCEED) {
		if (ZBX_JSON_TYPE_OBJECT != zbx_json_type(p)) {
			zabbix_log(LOG_LEVEL_WARNING, "Invalid type of data for table \"%s\" \"%.40s...\"",
					buf,
					p);
			res = FAIL;
			break;
		}

		res = process_proxyconfig_table(jp, buf, p);
	}
	if (res == SUCCEED)
		DBcommit();
	else
		DBrollback();
}

/******************************************************************************
 *                                                                            *
 * Function: process_configuration_sync                                       *
 *                                                                            *
 * Purpose: calculates checks sum of config data                              *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              * 
 *                                                                            *
 * Author: Aleksander Vladishev                                               *
 *                                                                            *
 * Comments: never returns                                                    *
 *                                                                            *
 ******************************************************************************/
static void	process_configuration_sync()
{
	zbx_sock_t	sock;
	char		*data;
	struct		zbx_json_parse jp;
	
	zabbix_log(LOG_LEVEL_DEBUG, "In process_configuration_sync()");

	while (FAIL == connect_to_server(&sock, 600)) { /* alarm */
		zabbix_log(LOG_LEVEL_WARNING, "Connect to the server failed. Retry after %d seconds",
				CONFIG_PROXYCONFIG_RETRY);

		sleep(CONFIG_PROXYCONFIG_RETRY);
	}

	if (FAIL == get_data_from_server(&sock, ZBX_PROTO_VALUE_PROXY_CONFIG, &data))
		goto exit;

	if (FAIL == zbx_json_open(data, &jp))
		goto exit;

	process_proxyconfig(&jp);
exit:
	disconnect_server(&sock);
}

/******************************************************************************
 *                                                                            *
 * Function: main_proxyconfig_loop                                            *
 *                                                                            *
 * Purpose: periodically request config data                                  *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              * 
 *                                                                            *
 * Author: Aleksander Vladishev                                               *
 *                                                                            *
 * Comments: never returns                                                    *
 *                                                                            *
 ******************************************************************************/
void	main_proxyconfig_loop()
{
	struct	sigaction phan;
	int	start, sleeptime;

	zabbix_log(LOG_LEVEL_DEBUG, "In main_proxyconfig_loop()");

	phan.sa_handler = child_signal_handler;
	sigemptyset(&phan.sa_mask);
	phan.sa_flags = 0;
	sigaction(SIGALRM, &phan, NULL);

	for (;;) {
		start = time(NULL);

		zbx_setproctitle("configuration syncer [connecting to the database]]");

		DBconnect(ZBX_DB_CONNECT_NORMAL);

		zbx_setproctitle("configuration syncer [load configuration]");

		process_configuration_sync();

		DBclose();

		sleeptime = CONFIG_PROXYCONFIG_FREQUENCY - (time(NULL) - start);

		if (sleeptime > 0) {
			zbx_setproctitle("configuration syncer [sleeping for %d seconds]",
					sleeptime);
			zabbix_log(LOG_LEVEL_DEBUG, "Sleeping for %d seconds",
					sleeptime);
			sleep(sleeptime);
		}
	}
}