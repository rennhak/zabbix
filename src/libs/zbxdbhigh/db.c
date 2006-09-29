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


#include <stdlib.h>
#include <stdio.h>

/* for setproctitle() */
#include <sys/types.h>
#include <unistd.h>

#include <string.h>
#include <strings.h>

#include "db.h"
#include "log.h"
#include "zlog.h"
#include "common.h"

#ifdef	HAVE_MYSQL
	MYSQL	mysql;
#endif

#ifdef	HAVE_PGSQL
	PGconn	*conn;
#endif

#ifdef	HAVE_ORACLE
	sqlo_db_handle_t oracle;
#endif

extern void	apply_actions(DB_TRIGGER *trigger,int trigger_value);
extern void	update_services(int triggerid, int status);
extern int	CONFIG_NODEID;
extern int	CONFIG_MASTER_NODEID;

void	DBclose(void)
{
#ifdef	HAVE_MYSQL
	mysql_close(&mysql);
#endif
#ifdef	HAVE_PGSQL
	PQfinish(conn);
#endif
#ifdef	HAVE_ORACLE
	sqlo_finish(oracle);
#endif
}

/*
 * Connect to the database.
 * If fails, program terminates.
 */ 
void    DBconnect(void)
{
	/*	zabbix_log(LOG_LEVEL_ERR, "[%s] [%s] [%s]\n",dbname, dbuser, dbpassword ); */
#ifdef	HAVE_MYSQL
	/* For MySQL >3.22.00 */
	/*	if( ! mysql_connect( &mysql, NULL, dbuser, dbpassword ) )*/

	mysql_init(&mysql);

    if( ! mysql_real_connect( &mysql, CONFIG_DBHOST, CONFIG_DBUSER, CONFIG_DBPASSWORD, CONFIG_DBNAME, CONFIG_DBPORT, CONFIG_DBSOCKET,0 ) )
	{
		zabbix_log(LOG_LEVEL_ERR, "Failed to connect to database: Error: %s",mysql_error(&mysql) );
		exit(FAIL);
	}
	else
	{
		if( mysql_select_db( &mysql, CONFIG_DBNAME ) != 0 )
		{
			zabbix_log(LOG_LEVEL_ERR, "Failed to select database: Error: %s",mysql_error(&mysql) );
			exit(FAIL);
		}
	}
	if(mysql_autocommit(&mysql, 1) != 0)
	{
			zabbix_log(LOG_LEVEL_ERR, "Failed to set autocommit to 1: Error: %s",mysql_error(&mysql));
			exit(FAIL);
	}
#endif
#ifdef	HAVE_PGSQL
/*	conn = PQsetdb(pghost, pgport, pgoptions, pgtty, dbName); */
/*	conn = PQsetdb(NULL, NULL, NULL, NULL, dbname);*/
	conn = PQsetdbLogin(CONFIG_DBHOST, NULL, NULL, NULL, CONFIG_DBNAME, CONFIG_DBUSER, CONFIG_DBPASSWORD );

/* check to see that the backend connection was successfully made */
	if (PQstatus(conn) != CONNECTION_OK)
	{
		zabbix_log(LOG_LEVEL_ERR, "Connection to database '%s' failed.", CONFIG_DBNAME);
		exit(FAIL);
	}
#endif
#ifdef	HAVE_ORACLE
	char    connect[MAX_STRING_LEN];

	if (SQLO_SUCCESS != sqlo_init(SQLO_OFF, 1, 100))
	{
		zabbix_log(LOG_LEVEL_ERR, "Failed to init libsqlora8");
		exit(FAIL);
	}
			        /* login */
	zbx_snprintf(connect, sizeof(connect),"%s/%s@%s", CONFIG_DBUSER, CONFIG_DBPASSWORD, CONFIG_DBNAME);
	if (SQLO_SUCCESS != sqlo_connect(&oracle, connect))
	{
		printf("Cannot login with %s\n", connect);
		zabbix_log(LOG_LEVEL_ERR, "Cannot login with %s", connect);
		exit(FAIL);
	}
	sqlo_autocommit_on(oracle);
#endif
}

/*
 * Execute SQL statement. For non-select statements only.
 * If fails, program terminates.
 */ 
int DBexecute(const char *fmt, ...)
{
	char	sql[ZBX_MAX_SQL_LEN];

	va_list args;
#ifdef	HAVE_PGSQL
	PGresult	*result;
#endif
#ifdef	HAVE_ORACLE
	int ret;
#endif

	va_start(args, fmt);
	vsnprintf(sql, ZBX_MAX_SQL_LEN-1, fmt, args);
	va_end(args);

	sql[ZBX_MAX_SQL_LEN-1]='\0';

	zabbix_log( LOG_LEVEL_DEBUG, "Executing query:%s", sql);
#ifdef	HAVE_MYSQL
	if(mysql_query(&mysql,sql) != 0)
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",sql);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s [%d]", mysql_error(&mysql), mysql_errno(&mysql) );
		return FAIL;
	}
	return (long)mysql_affected_rows(&mysql);
#endif
#ifdef	HAVE_PGSQL
	result = PQexec(conn,sql);

	if( result==NULL)
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",sql);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s", "Result is NULL" );
		PQclear(result);
		return FAIL;
	}
	if( PQresultStatus(result) != PGRES_COMMAND_OK)
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",sql);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s", PQresStatus(PQresultStatus(result)) );
		PQclear(result);
		return FAIL;
	}
	PQclear(result);
	return SUCCEED;
#endif
#ifdef	HAVE_ORACLE
	if ( (ret = sqlo_exec(oracle, sql))<0 )
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",sql);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s", sqlo_geterror(oracle) );
		zbx_error("Query::%s.",sql);
		zbx_error("Query failed:%s.", sqlo_geterror(oracle) );
		ret = FAIL;
	}
	return ret;
#endif
}


/*
 * Execute SQL statement. For non-select statements only.
 * If fails, program terminates.
 */ 
int	DBexecute_old(char *query)
{
/* Do not include any code here. Will break HAVE_PGSQL section */
#ifdef	HAVE_MYSQL
/*	if(strstr(query, "17828") != NULL)*/
	zabbix_log( LOG_LEVEL_DEBUG, "Executing query:%s",query);

	if(mysql_query(&mysql,query) != 0)
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",query);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s [%d]", mysql_error(&mysql), mysql_errno(&mysql) );
		return FAIL;
	}
	return (long)mysql_affected_rows(&mysql);
#endif
#ifdef	HAVE_PGSQL
	int ret = SUCCEED;
	PGresult	*result;

	zabbix_log( LOG_LEVEL_DEBUG, "Executing query:%s",query);
	result = PQexec(conn,query);

	if( result==NULL)
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",query);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s", "Result is NULL" );
		ret = FAIL;
	} else if( PQresultStatus(result) != PGRES_COMMAND_OK)
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",query);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s", PQresStatus(PQresultStatus(result)) );
		ret = FAIL;
	}
	
	ret = (int)PQoidValue(result); /* return object id, for insert id processing */
	
	PQclear(result);
	return ret;
#endif
#ifdef	HAVE_ORACLE
	int ret;
	zabbix_log( LOG_LEVEL_DEBUG, "Executing query:%s",query);
	if ( (ret = sqlo_exec(oracle, query))<0 )
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",query);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s", sqlo_geterror(oracle) );
		zbx_error("Query::%s.",query);
		zbx_error("Query failed:%s.", sqlo_geterror(oracle) );
		ret = FAIL;
	}
	return ret;
#endif
}

int	DBis_null(char *field)
{
	int ret = FAIL;

	if(field == NULL)	ret = SUCCEED;
#ifdef	HAVE_ORACLE
	else if(field[0] == 0)	ret = SUCCEED;
#endif
	return ret;
}

#ifdef  HAVE_PGSQL
void	PG_DBfree_result(DB_RESULT result)
{
	int i = 0;

	/* free old data */
	if(result->values)
	{
		for(i = 0; i < result->fld_num; i++)
		{
			if(!result->values[i]) continue;
			
			free(result->values[i]);
			result->values[i] = NULL;
		}
		result->fld_num = 0;
		free(result->values);
		result->values = NULL;
	}

	PQclear(result->pg_result);
}
#endif

DB_ROW	DBfetch(DB_RESULT result)
{
#ifdef	HAVE_MYSQL
	return mysql_fetch_row(result);
#endif
#ifdef	HAVE_PGSQL

	int	i;

	/* EOF */
	if(!result)	return NULL;
		
	/* free old data */
	if(result->values)
	{
		for(i = 0; i < result->fld_num; i++)
		{
			if(!result->values[i]) continue;
			
			free(result->values[i]);
			result->values[i] = NULL;
		}
		result->fld_num = 0;
		free(result->values);
		result->values = NULL;
	}
	
	/* EOF */
	if(result->cursor == result->row_num) return NULL;

	/* init result */	
	result->fld_num = PQnfields(result->pg_result);

	if(result->fld_num > 0)
	{
		result->values = malloc(sizeof(char*) * result->fld_num);
		for(i = 0; i < result->fld_num; i++)
		{
			 result->values[i] = strdup(PQgetvalue(result->pg_result, result->cursor, i));
		}
	}

	result->cursor++;

	return result->values;	
#endif
#ifdef	HAVE_ORACLE
	int res;

	res = sqlo_fetch(result, 1);

	if(SQLO_SUCCESS == res)
	{
		return sqlo_values(result, NULL, 1);
	}
	else if(SQLO_NO_DATA == res)
	{
		return 0;
	}
	else
	{
		zabbix_log( LOG_LEVEL_ERR, "Fetch failed:%s\n", sqlo_geterror(oracle) );
		exit(FAIL);
	}
#endif
}

/*
 * Execute SQL statement. For select statements only.
 * If fails, program terminates.
 */ 
DB_RESULT DBselect(const char *fmt, ...)
{
	char	sql[ZBX_MAX_SQL_LEN];

	va_list args;
#ifdef	HAVE_PGSQL
	PGresult	*result;
#endif
#ifdef	HAVE_ORACLE
	sqlo_stmt_handle_t sth;
#endif

	va_start(args, fmt);
	vsnprintf(sql, ZBX_MAX_SQL_LEN-1, fmt, args);
	va_end(args);

	sql[ZBX_MAX_SQL_LEN-1]='\0';

	zabbix_log( LOG_LEVEL_DEBUG, "Executing query:%s", sql);

#ifdef	HAVE_MYSQL
	if(mysql_query(&mysql,sql) != 0)
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",sql);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s [%d]", mysql_error(&mysql), mysql_errno(&mysql) );

		exit(FAIL);
	}
	return	mysql_store_result(&mysql);
#endif
#ifdef	HAVE_PGSQL
	result = PQexec(conn,sql);

	if( result==NULL)
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",sql);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s", "Result is NULL" );
		exit( FAIL );
	}
	if( PQresultStatus(result) != PGRES_TUPLES_OK)
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",sql);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s", PQresStatus(PQresultStatus(result)) );
		exit( FAIL );
	}
	return result;
#endif
#ifdef	HAVE_ORACLE
	if(0 > (sth = (sqlo_open(oracle, sql,0,NULL))))
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",sql);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s", sqlo_geterror(oracle));
		exit(FAIL);
	}
	return sth;
#endif
}

/*
 * Execute SQL statement. For select statements only.
 * If fails, program terminates.
 */ 
DB_RESULT DBselect_old(char *query)
{
/* Do not include any code here. Will break HAVE_PGSQL section */
#ifdef	HAVE_MYSQL
	zabbix_log( LOG_LEVEL_DEBUG, "Executing query:%s",query);

	if(mysql_query(&mysql,query) != 0)
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",query);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s [%d]", mysql_error(&mysql), mysql_errno(&mysql) );

		exit(FAIL);
	}
	return	mysql_store_result(&mysql);
#endif
#ifdef	HAVE_PGSQL
	PGresult		*pg_result;
	ZBX_PG_DB_RESULT	*result = NULL;

	zabbix_log( LOG_LEVEL_DEBUG, "Executing query:%s",query);

	pg_result = PQexec(conn,query);

	if( pg_result==NULL)
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",query);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s", "Result is NULL" );
		PQclear(pg_result);
		exit( FAIL );
	}
	else if( PQresultStatus(pg_result) != PGRES_TUPLES_OK)
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",query);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s", PQresStatus(PQresultStatus(pg_result)) );
		PQclear(pg_result);
		exit( FAIL );
	}
	else
	{
		result = malloc(sizeof(ZBX_PG_DB_RESULT));
		result->pg_result	= pg_result;
		result->row_num		= PQntuples(pg_result);
		result->fld_num		= 0;
		result->cursor		= 0;
		result->values		= NULL;
	}
	
	return result;
#endif
#ifdef	HAVE_ORACLE
	sqlo_stmt_handle_t sth;

	zabbix_log( LOG_LEVEL_DEBUG, "Executing query:%s",query);
	if(0 > (sth = (sqlo_open(oracle, query,0,NULL))))
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",query);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s", sqlo_geterror(oracle));
		exit(FAIL);
	}
	return sth;
#endif
}

/*
 * Execute SQL statement. For select statements only.
 * If fails, program terminates.
 */ 
DB_RESULT DBselectN(char *query, int n)
{
#ifdef	HAVE_MYSQL
	return DBselect("%s limit %d", query, n);
#endif
#ifdef	HAVE_PGSQL
	return DBselect("%s limit %d", query, n);
#endif
#ifdef	HAVE_ORACLE
	return DBselect("select * from (%s) where rownum<=%d", query, n);
#endif
}

/*
 * Get value for given row and field. Must be called after DBselect.
 */ 
/*
char	*DBget_field(DB_RESULT result, int rownum, int fieldnum)
{
#ifdef	HAVE_MYSQL
	MYSQL_ROW	row;

	mysql_data_seek(result, rownum);
	row=mysql_fetch_row(result);
	if(row == NULL)
	{
		zabbix_log(LOG_LEVEL_ERR, "Error while mysql_fetch_row():Error [%s] Rownum [%d] Fieldnum [%d]", mysql_error(&mysql), rownum, fieldnum );
		zabbix_syslog("MYSQL: Error while mysql_fetch_row():Error [%s] Rownum [%d] Fieldnum [%d]", mysql_error(&mysql), rownum, fieldnum );
		exit(FAIL);
	}
	return row[fieldnum];
#endif
#ifdef	HAVE_PGSQL
	return PQgetvalue(result, rownum, fieldnum);
#endif
#ifdef	HAVE_ORACLE
	return FAIL;
#endif
}
*/

/*
 * Get value of autoincrement field for last insert or update statement
 */ 
int	DBinsert_id(int exec_result, const char *table, const char *field)
{
#ifdef	HAVE_MYSQL
	zabbix_log(LOG_LEVEL_DEBUG, "In DBinsert_id()" );
	
	if(exec_result == FAIL) return 0;
	
	return mysql_insert_id(&mysql);
#endif

#ifdef	HAVE_PGSQL
	DB_RESULT	tmp_res;
	int		id_res = FAIL;

	zabbix_log(LOG_LEVEL_DEBUG, "In DBinsert_id()" );
	
	if(exec_result < 0) return 0;
	if(exec_result == FAIL) return 0;
	if((Oid)exec_result == InvalidOid) return 0;
	
	tmp_res = DBselect("select %s from %s where oid=%i", field, table, exec_result);
	
	id_res = atoi(PQgetvalue(tmp_res->pg_result, 0, 0));
	
	DBfree_result(tmp_res);
	
	return id_res;
#endif

#ifdef	HAVE_ORACLE
	DB_ROW	row;
	
	zabbix_log(LOG_LEVEL_DEBUG, "In DBinsert_id()" );

	if(exec_result == FAIL) return 0;
	
	row = DBfetch(DBselect("select %s_%s.currval from dual", table, field));

	return atoi(row[0]);
	
#endif
}

/*
 * Returs number of affected rows of last select, update, delete or replace
 */
/*
long    DBaffected_rows()
{
#ifdef  HAVE_MYSQL
	return (long)mysql_affected_rows(&mysql);
#endif
#ifdef  HAVE_PGSQL
	NOT IMPLEMENTED YET
#endif
#ifdef	HAVE_ORACLE
	return FAIL;
#endif
}*/


/*
 * Return SUCCEED if result conains no records
 */ 
/*int	DBis_empty(DB_RESULT *result)
{
	zabbix_log(LOG_LEVEL_DEBUG, "In DBis_empty");
	if(result == NULL)
	{
		return	SUCCEED;
	}
	if(DBnum_rows(result) == 0)
	{
		return	SUCCEED;
	}
	if(DBget_field(result,0,0) == 0)
	{
		return	SUCCEED;
	}

	return FAIL;
}*/

/*
 * Get number of selected records.
 */ 
/*
int	DBnum_rows(DB_RESULT result)
{
#ifdef	HAVE_MYSQL
	int rows;

	zabbix_log(LOG_LEVEL_DEBUG, "In DBnum_rows");
	if(result == NULL)
	{
		return	0;
	}
// Order is important !
	rows = mysql_num_rows(result);
	if(rows == 0)
	{
		return	0;
	}
	
// This is necessary to exclude situations like
// atoi(DBget_field(result,0,0). This leads to coredump.
//
// This is required for empty results for count(*), etc 
	if(DBget_field(result,0,0) == 0)
	{
		return	0;
	}
	zabbix_log(LOG_LEVEL_DEBUG, "Result of DBnum_rows [%d]", rows);
	return rows;
#endif
#ifdef	HAVE_PGSQL
	zabbix_log(LOG_LEVEL_DEBUG, "In DBnum_rows");
	return PQntuples(result);
#endif
#ifdef	HAVE_ORACLE
	return sqlo_prows(result);
#endif
}
*/

/*
 * Get function value.
 */ 
int     DBget_function_result(double *result,char *functionid)
{
	DB_RESULT dbresult;
	DB_ROW	row;
	int		res = SUCCEED;

/* 0 is added to distinguish between lastvalue==NULL and empty result */
	dbresult = DBselect("select 0,lastvalue from functions where functionid=%s", functionid );

	row = DBfetch(dbresult);

	if(!row)
	{
		zabbix_log(LOG_LEVEL_WARNING, "No function for functionid:[%s]", functionid );
		zabbix_syslog("No function for functionid:[%s]", functionid );
		res = FAIL;
	}
	else if(DBis_null(row[1]) == SUCCEED)
	{
		zabbix_log(LOG_LEVEL_DEBUG, "function.lastvalue==NULL [%s]", functionid );
		res = FAIL;
	}
	else
	{
        	*result=atof(row[1]);
	}
        DBfree_result(dbresult);

        return res;
}

/******************************************************************************
 *                                                                            *
 * Function: get_latest_event_status                                          *
 *                                                                            *
 * Purpose: return status of latest event of the trigger                      *
 *                                                                            *
 * Parameters: triggerid - trigger ID, status - trigger status                *
 *                                                                            *
 * Return value: On SUCCESS, status - status of last event                    *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments: Rewrite required to simplify logic ?                             *
 *                                                                            *
 ******************************************************************************/
static void	get_latest_event_status(zbx_uint64_t triggerid, int *prev_status, int *latest_status)
{
	char		sql[MAX_STRING_LEN];
	DB_RESULT	result;
	DB_ROW		row;

	zabbix_log(LOG_LEVEL_DEBUG,"In latest_event()");

	zbx_snprintf(sql,sizeof(sql),"select value from events where triggerid=" ZBX_FS_UI64 " order by clock desc",triggerid);
	zabbix_log(LOG_LEVEL_DEBUG,"SQL [%s]", sql);
	result = DBselectN(sql,2);
	row = DBfetch(result);

	if(!row || DBis_null(row[0])==SUCCEED)
        {
		zabbix_log(LOG_LEVEL_DEBUG, "Result for last is empty" );
                *prev_status = TRIGGER_VALUE_UNKNOWN;
		*latest_status = TRIGGER_VALUE_UNKNOWN;
        }
	else
	{
                *prev_status = TRIGGER_VALUE_FALSE;
		*latest_status = atoi(row[0]);

		row = DBfetch(result);
		if(row && DBis_null(row[0]) != SUCCEED)
		{
			*prev_status = atoi(row[0]);
		}

	}
	DBfree_result(result);
}

/* Returns previous trigger value. If not value found, return TRIGGER_VALUE_FALSE */
/*int	DBget_prev_trigger_value(int triggerid)
{
	int	clock;
	int	value;

	DB_RESULT	result;
	DB_ROW		row;

	zabbix_log(LOG_LEVEL_DEBUG,"In DBget_prev_trigger_value[%d]", triggerid);

	result = DBselect("select max(clock) from events where triggerid=%d",triggerid);

	row=DBfetch(result);

	if(!row || DBis_null(row[0])==SUCCEED)
	{
		zabbix_log(LOG_LEVEL_DEBUG, "Result for MAX is empty" );
		DBfree_result(result);
		return TRIGGER_VALUE_UNKNOWN;
	}
	clock=atoi(row[0]);
	DBfree_result(result);

	result=DBselect("select max(clock) from events where triggerid=%d and clock<%d",triggerid,clock);
	row=DBfetch(result);

	if(!row || DBis_null(row[0])==SUCCEED)
	{
		zabbix_log(LOG_LEVEL_DEBUG, "Result for MAX is empty" );
		DBfree_result(result);
		return TRIGGER_VALUE_FALSE;
	}
	clock=atoi(row[0]);
	DBfree_result(result);

	result = DBselect("select value from events where triggerid=%d and clock=%d",triggerid,clock);
	row=DBfetch(result);

	if(!row || DBis_null(row[0])==SUCCEED)
	{
		zabbix_log(LOG_LEVEL_DEBUG, "Result of SQL is empty");
		DBfree_result(result);
		return TRIGGER_VALUE_UNKNOWN;
	}
	value=atoi(row[0]);
	DBfree_result(result);

	return value;
}*/

/* SUCCEED if latest event with triggerid has this status */
/* Rewrite required to simplify logic ?*/
/*
static int	latest_event(int triggerid, int status)
{
	char		sql[MAX_STRING_LEN];
	DB_RESULT	result;
	DB_ROW		row;
	int 		ret = FAIL;


	zabbix_log(LOG_LEVEL_DEBUG,"In latest_event()");

	zbx_snprintf(sql,sizeof(sql),"select value from events where triggerid=%d order by clock desc",triggerid);
	zabbix_log(LOG_LEVEL_DEBUG,"SQL [%s]",sql);
	result = DBselectN(sql,1);
	row = DBfetch(result);

	if(!row || DBis_null(row[0])==SUCCEED)
        {
                zabbix_log(LOG_LEVEL_DEBUG, "Result for last is empty" );
        }
	else
	{
		if(atoi(row[0]) == status)
		{
			ret = SUCCEED;
		}
	}

	DBfree_result(result);

	return ret;
}
*/

/* SUCCEED if latest service alarm has this status */
/* Rewrite required to simplify logic ?*/
int	latest_service_alarm(zbx_uint64_t serviceid, int status)
{
	int	clock;
	DB_RESULT	result;
	DB_ROW		row;
	int ret = FAIL;


	zabbix_log(LOG_LEVEL_DEBUG,"In latest_service_alarm()");

	result = DBselect("select max(clock) from service_alarms where serviceid=" ZBX_FS_UI64,serviceid);
	row = DBfetch(result);

	if(!row || DBis_null(row[0])==SUCCEED)
        {
                zabbix_log(LOG_LEVEL_DEBUG, "Result for MAX is empty" );
                ret = FAIL;
        }
	else
	{
		clock=atoi(row[0]);
		DBfree_result(result);

		result = DBselect("select value from service_alarms where serviceid=" ZBX_FS_UI64 " and clock=%d",serviceid,clock);
		row = DBfetch(result);
		if(row && DBis_null(row[0]) != SUCCEED)
		{
			if(atoi(row[0]) == status)
			{
				ret = SUCCEED;
			}
		}
	}

	DBfree_result(result);

	return ret;
}

/* Returns eventid or 0 */
/*
int	add_event(int triggerid,int status,int clock,int *eventid)
{
	*eventid=0;

	zabbix_log(LOG_LEVEL_DEBUG,"In add_event(%d,%d,%d)",triggerid, status, *eventid);

	if(latest_event(triggerid,status) == SUCCEED)
	{
		zabbix_log(LOG_LEVEL_DEBUG,"Alarm for triggerid [%d] status [%d] already exists",triggerid,status);
		return FAIL;
	}


	*eventid = DBinsert_id(
		DBexecute("insert into events(triggerid,clock,value) values(%d,%d,%d)", triggerid, clock, status),
		"events", "eventid");

	if(status == TRIGGER_VALUE_FALSE || status == TRIGGER_VALUE_TRUE)
	{
		DBexecute("update events set retries=3,error='Trigger changed its status. WIll not send repeats.' where triggerid=%d and repeats>0 and status=%d", triggerid, ALERT_STATUS_NOT_SENT);
	}

	zabbix_log(LOG_LEVEL_DEBUG,"End of add_event()");
	
	return SUCCEED;
}
*/

int	DBadd_service_alarm(zbx_uint64_t serviceid,int status,int clock)
{
	zabbix_log(LOG_LEVEL_DEBUG,"In add_service_alarm()");

	if(latest_service_alarm(serviceid,status) == SUCCEED)
	{
		return SUCCEED;
	}

	DBexecute("insert into service_alarms(serviceid,clock,value) values(" ZBX_FS_UI64 ",%d,%d)", serviceid, clock, status);

	zabbix_log(LOG_LEVEL_DEBUG,"End of add_service_alarm()");
	
	return SUCCEED;
}

int	DBupdate_trigger_value(DB_TRIGGER *trigger, int new_value, int now, char *reason)
{
	int	ret = SUCCEED;
	DB_EVENT	event;
	int		event_last_status;
	int		event_prev_status;

	if(reason==NULL)
	{
		zabbix_log(LOG_LEVEL_DEBUG,"In update_trigger_value[" ZBX_FS_UI64 ",%d,%d]", trigger->triggerid, new_value, now);
	}
	else
	{
		zabbix_log(LOG_LEVEL_DEBUG,"In update_trigger_value[" ZBX_FS_UI64 ",%d,%d,%s]", trigger->triggerid, new_value, now, reason);
	}

	/* New trigger value differs from current one */
	if(trigger->value != new_value)
	{
		get_latest_event_status(trigger->triggerid, &event_prev_status, &event_last_status);

		/* The lastest event has the same status, skip of so. */
		if(event_last_status != new_value)
		{
			if(reason==NULL)
			{
				DBexecute("update triggers set value=%d,lastchange=%d,error='' where triggerid=" ZBX_FS_UI64,new_value,now,trigger->triggerid);
			}
			else
			{
				DBexecute("update triggers set value=%d,lastchange=%d,error='%s' where triggerid=" ZBX_FS_UI64,new_value,now,reason, trigger->triggerid);
			}
			if(	((trigger->value == TRIGGER_VALUE_TRUE) && (new_value == TRIGGER_VALUE_FALSE)) ||
				((trigger->value == TRIGGER_VALUE_FALSE) && (new_value == TRIGGER_VALUE_TRUE)) ||
				((event_prev_status == TRIGGER_VALUE_FALSE) && (trigger->value == TRIGGER_VALUE_UNKNOWN) && (new_value == TRIGGER_VALUE_TRUE)) ||
				((event_prev_status == TRIGGER_VALUE_TRUE) && (trigger->value == TRIGGER_VALUE_UNKNOWN) && (new_value == TRIGGER_VALUE_FALSE)))
			{
				/* Preparing event for processing */
				memset(&event,0,sizeof(DB_EVENT));
				event.eventid = 0;
				event.triggerid = trigger->triggerid;
				event.clock = now;
				event.value = new_value;
				event.acknowledged = 0;

				/* Processing event */
				if(process_event(event) == SUCCEED)
				{
					zabbix_log(LOG_LEVEL_DEBUG,"Event processed OK");
				}
				else
				{
					zabbix_log(LOG_LEVEL_WARNING,"Event processed not OK");
				}
/*				zabbix_log(LOG_LEVEL_DEBUG,"In update_trigger_value. Before apply_actions. Triggerid [%d] prev [%d] curr [%d] new [%d]", trigger->triggerid, event_prev_status, trigger->value, new_value);
				apply_actions(trigger,new_value);
				if(new_value == TRIGGER_VALUE_TRUE)
				{
					update_services(trigger->triggerid, trigger->priority);
				}
				else
				{
					update_services(trigger->triggerid, 0);
				}*/
			}
		}
		else
		{
			zabbix_log(LOG_LEVEL_DEBUG,"Alarm not added for triggerid [" ZBX_FS_UI64 "]", trigger->triggerid);
			ret = FAIL;
		}
	}
	return ret;
}

void update_triggers_status_to_unknown(zbx_uint64_t hostid,int clock,char *reason)
{
	DB_RESULT	result;
	DB_ROW		row;
	DB_TRIGGER	trigger;

	zabbix_log(LOG_LEVEL_DEBUG,"In update_triggers_status_to_unknown()");

/*	zbx_snprintf(sql,sizeof(sql),"select distinct t.triggerid from hosts h,items i,triggers t,functions f where f.triggerid=t.triggerid and f.itemid=i.itemid and h.hostid=i.hostid and h.hostid=%d and i.key_<>'%s'",hostid,SERVER_STATUS_KEY);*/
	result = DBselect("select distinct t.triggerid,t.value,t.comments from hosts h,items i,triggers t,functions f where f.triggerid=t.triggerid and f.itemid=i.itemid and h.hostid=i.hostid and h.hostid=" ZBX_FS_UI64 " and i.key_ not in ('%s','%s','%s')",hostid,SERVER_STATUS_KEY, SERVER_ICMPPING_KEY, SERVER_ICMPPINGSEC_KEY);

	while((row=DBfetch(result)))
	{
		ZBX_STR2UINT64(trigger.triggerid,row[0]);
/*		trigger.triggerid=atoi(row[0]);*/
		trigger.value=atoi(row[1]);
		strscpy(trigger.comments, row[2]);
		DBupdate_trigger_value(&trigger,TRIGGER_VALUE_UNKNOWN,clock,reason);
	}

	DBfree_result(result);
	zabbix_log(LOG_LEVEL_DEBUG,"End of update_triggers_status_to_unknown()");

	return; 
}

void  DBdelete_service(zbx_uint64_t serviceid)
{
	DBexecute("delete from services_links where servicedownid=" ZBX_FS_UI64 " or serviceupid=" ZBX_FS_UI64,
		serviceid, serviceid);
	DBexecute("delete from services where serviceid=" ZBX_FS_UI64,
		serviceid);
}

void  DBdelete_services_by_triggerid(zbx_uint64_t triggerid)
{
	zbx_uint64_t	serviceid;
	DB_RESULT	result;
	DB_ROW		row;

	zabbix_log(LOG_LEVEL_DEBUG,"In DBdelete_services_by_triggerid(" ZBX_FS_UI64 ")", triggerid);
	result = DBselect("select serviceid from services where triggerid=" ZBX_FS_UI64, triggerid);

	while((row=DBfetch(result)))
	{
/*		serviceid=atoi(row[0]);*/
		ZBX_STR2UINT64(serviceid, row[0]);
		DBdelete_service(serviceid);
	}
	DBfree_result(result);

	zabbix_log(LOG_LEVEL_DEBUG,"End of DBdelete_services_by_triggerid(" ZBX_FS_UI64 ")", triggerid);
}

void  DBdelete_trigger(zbx_uint64_t triggerid)
{
	DBexecute("delete from trigger_depends where triggerid_down=" ZBX_FS_UI64 " or triggerid_up=" ZBX_FS_UI64,
		triggerid, triggerid);
	DBexecute("delete from functions where triggerid=" ZBX_FS_UI64,
		triggerid);
	DBexecute("delete from events where triggerid=" ZBX_FS_UI64,
		triggerid);
/*	zbx_snprintf(sql,sizeof(sql),"delete from actions where triggerid=%d and scope=%d", triggerid, ACTION_SCOPE_TRIGGER);
	DBexecute(sql);*/

	DBdelete_services_by_triggerid(triggerid);

	DBexecute("update sysmaps_links set triggerid=NULL where triggerid=" ZBX_FS_UI64,
		triggerid);
	DBexecute("delete from triggers where triggerid=" ZBX_FS_UI64,
		triggerid);
}

void  DBdelete_triggers_by_itemid(zbx_uint64_t itemid)
{
	zbx_uint64_t	triggerid;
	DB_RESULT	result;
	DB_ROW		row;

	zabbix_log(LOG_LEVEL_DEBUG,"In DBdelete_triggers_by_itemid(" ZBX_FS_UI64 ")",
		itemid);
	result = DBselect("select triggerid from functions where itemid=" ZBX_FS_UI64,
		itemid);

	while((row=DBfetch(result)))
	{
		ZBX_STR2UINT64(triggerid, row[0]);
/*		triggerid=atoi(row[0]);*/
		DBdelete_trigger(triggerid);
	}
	DBfree_result(result);

	DBexecute("delete from functions where itemid=" ZBX_FS_UI64,
		itemid);

	zabbix_log(LOG_LEVEL_DEBUG,"End of DBdelete_triggers_by_itemid(" ZBX_FS_UI64 ")",
		itemid);
}

void DBdelete_trends_by_itemid(zbx_uint64_t itemid)
{
	DBexecute("delete from trends where itemid=" ZBX_FS_UI64,
		itemid);
}

void DBdelete_history_by_itemid(zbx_uint64_t itemid)
{
	DBexecute("delete from history where itemid=" ZBX_FS_UI64,
		itemid);
	DBexecute("delete from history_str where itemid=" ZBX_FS_UI64,
		itemid);
}

void DBdelete_sysmaps_links_by_shostid(zbx_uint64_t shostid)
{
	DBexecute("delete from sysmaps_links where shostid1=" ZBX_FS_UI64 " or shostid2=" ZBX_FS_UI64,
		shostid, shostid);
}

void DBdelete_sysmaps_hosts_by_hostid(zbx_uint64_t hostid)
{
	zbx_uint64_t	shostid;
	DB_RESULT	result;
	DB_ROW		row;

	zabbix_log(LOG_LEVEL_DEBUG,"In DBdelete_sysmaps_hosts(" ZBX_FS_UI64 ")",
		hostid);
	result = DBselect("select shostid from sysmaps_elements where elementid=" ZBX_FS_UI64,
		hostid);

	while((row=DBfetch(result)))
	{
		ZBX_STR2UINT64(shostid, row[0]);
/*		shostid=atoi(row[0]);*/
		DBdelete_sysmaps_links_by_shostid(shostid);
	}
	DBfree_result(result);

	DBexecute("delete from sysmaps_elements where elementid=" ZBX_FS_UI64,
		hostid);
}

/*
int DBdelete_history_pertial(int itemid)
{
	char	sql[MAX_STRING_LEN];

#ifdef	HAVE_ORACLE
	zbx_snprintf(sql,sizeof(sql),"delete from history where itemid=%d and rownum<500", itemid);
#else
	zbx_snprintf(sql,sizeof(sql),"delete from history where itemid=%d limit 500", itemid);
#endif
	DBexecute(sql);

	return DBaffected_rows();
}
*/

void DBupdate_triggers_status_after_restart(void)
{
	int	lastchange;
	int	now;

	DB_RESULT	result;
	DB_RESULT	result2;
	DB_ROW	row;
	DB_ROW	row2;
	DB_TRIGGER	trigger;

	zabbix_log(LOG_LEVEL_DEBUG,"In DBupdate_triggers_after_restart()");

	now=time(NULL);

	result = DBselect("select distinct t.triggerid,t.value from hosts h,items i,triggers t,functions f where f.triggerid=t.triggerid and f.itemid=i.itemid and h.hostid=i.hostid and i.nextcheck+i.delay<%d and i.key_<>'%s' and h.status not in (%d,%d)",now,SERVER_STATUS_KEY, HOST_STATUS_DELETED, HOST_STATUS_TEMPLATE);

	while((row=DBfetch(result)))
	{
		ZBX_STR2UINT64(trigger.triggerid,row[0]);
/*		trigger.triggerid=atoi(row[0]);*/
		trigger.value=atoi(row[1]);

		result2 = DBselect("select min(i.nextcheck+i.delay) from hosts h,items i,triggers t,functions f where f.triggerid=t.triggerid and f.itemid=i.itemid and h.hostid=i.hostid and i.nextcheck<>0 and t.triggerid=%d and i.type<>%d",trigger.triggerid,ITEM_TYPE_TRAPPER);
		row2=DBfetch(result2);
		if(!row2 || DBis_null(row2[0])==SUCCEED)
		{
			zabbix_log(LOG_LEVEL_DEBUG, "No triggers to update (2)");
			DBfree_result(result2);
			continue;
		}

		lastchange=atoi(row2[0]);
		DBfree_result(result2);

		DBupdate_trigger_value(&trigger,TRIGGER_VALUE_UNKNOWN,lastchange,"ZABBIX was down.");
	}

	DBfree_result(result);
	zabbix_log(LOG_LEVEL_DEBUG,"End of DBupdate_triggers_after_restart()");

	return; 
}

void DBupdate_host_availability(zbx_uint64_t hostid,int available,int clock, char *error)
{
	DB_RESULT	result;
	DB_ROW		row;
	char	error_esc[MAX_STRING_LEN];
	int	disable_until;

	zabbix_log(LOG_LEVEL_DEBUG,"In update_host_availability()");

	if(error!=NULL)
	{
		DBescape_string(error,error_esc,MAX_STRING_LEN);
	}
	else
	{
		strscpy(error_esc,"");
	}

	result = DBselect("select available,disable_until from hosts where hostid=" ZBX_FS_UI64, hostid);
	row=DBfetch(result);

	if(!row)
	{
		zabbix_log(LOG_LEVEL_ERR, "Cannot select host with hostid [" ZBX_FS_UI64 "]",hostid);
		zabbix_syslog("Cannot select host with hostid [" ZBX_FS_UI64 "]",hostid);
		DBfree_result(result);
		return;
	}

	disable_until = atoi(row[1]);

	if(available == atoi(row[0]))
	{
/*		if((available==HOST_AVAILABLE_FALSE) 
		&&(clock+CONFIG_UNREACHABLE_PERIOD>disable_until) )
		{
		}
		else
		{*/
			zabbix_log(LOG_LEVEL_DEBUG, "Host already has availability [%d]",available);
			DBfree_result(result);
			return;
/*		}*/
	}

	DBfree_result(result);

	if(available==HOST_AVAILABLE_TRUE)
	{
		DBexecute("update hosts set available=%d,error=' ',errors_from=0 where hostid=" ZBX_FS_UI64,HOST_AVAILABLE_TRUE,hostid);
	}
	else if(available==HOST_AVAILABLE_FALSE)
	{
/*		if(disable_until+CONFIG_UNREACHABLE_PERIOD>clock)
		{
			zbx_snprintf(sql,sizeof(sql),"update hosts set available=%d,disable_until=disable_until+%d,error='%s' where hostid=%d",HOST_AVAILABLE_FALSE,CONFIG_UNREACHABLE_DELAY,error_esc,hostid);
		}
		else
		{
			zbx_snprintf(sql,sizeof(sql),"update hosts set available=%d,disable_until=%d,error='%s' where hostid=%d",HOST_AVAILABLE_FALSE,clock+CONFIG_UNREACHABLE_DELAY,error_esc,hostid);
		}*/
		/* '%s ' - space to make Oracle happy */
		DBexecute("update hosts set available=%d,error='%s ' where hostid=" ZBX_FS_UI64,
			HOST_AVAILABLE_FALSE,error_esc,hostid);
	}
	else
	{
		zabbix_log( LOG_LEVEL_ERR, "Unknown host availability [%d] for hostid [" ZBX_FS_UI64 "]",
			available, hostid);
		zabbix_syslog("Unknown host availability [%d] for hostid [" ZBX_FS_UI64 "]",
			available, hostid);
		return;
	}

	update_triggers_status_to_unknown(hostid,clock,"Host is unavailable.");
	zabbix_log(LOG_LEVEL_DEBUG,"End of update_host_availability()");

	return;
}

int	DBupdate_item_status_to_notsupported(zbx_uint64_t itemid, char *error)
{
	char	error_esc[MAX_STRING_LEN];

	zabbix_log(LOG_LEVEL_DEBUG,"In DBupdate_item_status_to_notsupported()");

	if(error!=NULL)
	{
		DBescape_string(error,error_esc,MAX_STRING_LEN);
	}
	else
	{
		strscpy(error_esc,"");
	}

	/* '&s ' to make Oracle happy */
	DBexecute("update items set status=%d,error='%s ' where itemid=" ZBX_FS_UI64,
		ITEM_STATUS_NOTSUPPORTED,error_esc,itemid);

	return SUCCEED;
}

int	DBadd_trend(int itemid, double value, int clock)
{
	DB_RESULT	result;
	DB_ROW		row;
	int	hour;
	int	num;
	double	value_min, value_avg, value_max;	

	zabbix_log(LOG_LEVEL_DEBUG,"In add_trend()");

	hour=clock-clock%3600;

	result = DBselect("select num,value_min,value_avg,value_max from trends where itemid=" ZBX_FS_UI64 " and clock=%d",
		itemid, hour);

	row=DBfetch(result);

	if(row)
	{
		num=atoi(row[0]);
		value_min=atof(row[1]);
		value_avg=atof(row[2]);
		value_max=atof(row[3]);
		if(value<value_min)	value_min=value;
/* Unfortunate mistake... */
/*		if(value>value_avg)	value_max=value;*/
		if(value>value_max)	value_max=value;
		value_avg=(num*value_avg+value)/(num+1);
		num++;
		DBexecute("update trends set num=%d, value_min=%f, value_avg=%f, value_max=%f where itemid=" ZBX_FS_UI64 " and clock=%d",
			num, value_min, value_avg, value_max, itemid, hour);
	}
	else
	{
		DBexecute("insert into trends (clock,itemid,num,value_min,value_avg,value_max) values (%d," ZBX_FS_UI64 ",%d,%f,%f,%f)",
			hour, itemid, 1, value, value, value);
	}

	DBfree_result(result);

	return SUCCEED;
}

int	DBadd_history(zbx_uint64_t itemid, double value, int clock)
{
	zabbix_log(LOG_LEVEL_DEBUG,"In add_history()");

	DBexecute("insert into history (clock,itemid,value) values (%d," ZBX_FS_UI64 ",%f)",
		clock,itemid,value);

	if(CONFIG_MASTER_NODEID>=0)
	{
		DBexecute("insert into history_sync (nodeid,clock,itemid,value) values (%d,%d," ZBX_FS_UI64 ",%f)",
			get_nodeid_by_id(itemid),clock,itemid,value);
	}

	DBadd_trend(itemid, value, clock);

	return SUCCEED;
}

int	DBadd_history_uint(zbx_uint64_t itemid, zbx_uint64_t value, int clock)
{
	zabbix_log(LOG_LEVEL_DEBUG,"In add_history_uint()");

	DBexecute("insert into history_uint (clock,itemid,value) values (%d," ZBX_FS_UI64 "," ZBX_FS_UI64 ")",
		clock,itemid,value);

	DBadd_trend(itemid, (double)value, clock);

	return SUCCEED;
}

int	DBadd_history_str(zbx_uint64_t itemid, char *value, int clock)
{
	char	value_esc[MAX_STRING_LEN];

	zabbix_log(LOG_LEVEL_DEBUG,"In add_history_str()");

	DBescape_string(value,value_esc,MAX_STRING_LEN);
	DBexecute("insert into history_str (clock,itemid,value) values (%d," ZBX_FS_UI64 ",'%s')",
		clock,itemid,value_esc);

	return SUCCEED;
}

int	DBadd_history_text(zbx_uint64_t itemid, char *value, int clock)
{
#ifdef HAVE_ORACLE
	char	sql[MAX_STRING_LEN];
	char	*value_esc;
	int	value_esc_max_len = 0;
	int	ret = FAIL;

	sqlo_lob_desc_t		loblp;		/* the lob locator */
	sqlo_stmt_handle_t	sth;

	sqlo_autocommit_off(oracle);

	zabbix_log(LOG_LEVEL_DEBUG,"In add_history_text()");

	value_esc_max_len = strlen(value)+1024;
	value_esc = malloc(value_esc_max_len);
	if(value_esc == NULL)
	{
		zabbix_log(LOG_LEVEL_DEBUG,"Can't allocate required memory");
		goto lbl_exit;
	}

	DBescape_string(value, value_esc, value_esc_max_len-1);
	value_esc_max_len = strlen(value_esc);

	/* alloate the lob descriptor */
	if(sqlo_alloc_lob_desc(oracle, &loblp) < 0)
	{
		zabbix_log(LOG_LEVEL_DEBUG,"CLOB allocating failed:%s", sqlo_geterror(oracle));
		goto lbl_exit;
	}

	zbx_snprintf(sql, sizeof(sql), "insert into history_text (clock,itemid,value)"
		" values (%d," ZBX_FS_UI64 ", EMPTY_CLOB()) returning value into :1",
		clock, itemid);

	zabbix_log(LOG_LEVEL_DEBUG,"Query:%s", sql);

	/* parse the statement */
	sth = sqlo_prepare(oracle, sql);
	if(sth < 0)
	{
		zabbix_log(LOG_LEVEL_DEBUG,"Query prepearing failed:%s", sqlo_geterror(oracle));
		goto lbl_exit;
	}

	/* bind input variables. Note: we bind the lob descriptor here */
	if(SQLO_SUCCESS != sqlo_bind_by_pos(sth, 1, SQLOT_CLOB, &loblp, 0, NULL, 0))
	{
		zabbix_log(LOG_LEVEL_DEBUG,"CLOB binding failed:%s", sqlo_geterror(oracle));
		goto lbl_exit_loblp;
	}

	/* execute the statement */
	if(sqlo_execute(sth, 1) != SQLO_SUCCESS)
	{
		zabbix_log(LOG_LEVEL_DEBUG,"Query failed:%s", sqlo_geterror(oracle));
		goto lbl_exit_loblp;
	}

	/* write the lob */
	ret = sqlo_lob_write_buffer(oracle, loblp, value_esc_max_len, value_esc, value_esc_max_len, SQLO_ONE_PIECE);
	if(ret < 0)
	{
		zabbix_log(LOG_LEVEL_DEBUG,"CLOB writing failed:%s", sqlo_geterror(oracle) );
		goto lbl_exit_loblp;
	}

	/* commiting */
	if(sqlo_commit(oracle) < 0)
	{
		zabbix_log(LOG_LEVEL_DEBUG,"Commiting failed:%s", sqlo_geterror(oracle) );
	}

	ret = SUCCEED;

lbl_exit_loblp:
	sqlo_free_lob_desc(oracle, &loblp);

lbl_exit:
	if(sth >= 0)	sqlo_close(sth);
	if(value_esc)	free(value_esc);

	sqlo_autocommit_on(oracle);

	return ret;

#else /* HAVE_ORACLE */

	char    *value_esc;
	int	value_esc_max_len = 0;
	int	sql_max_len = 0;

	zabbix_log(LOG_LEVEL_DEBUG,"In add_history_str()");

	value_esc_max_len = strlen(value)+1024;
	value_esc = malloc(value_esc_max_len);
	if(value_esc == NULL)
	{
		return FAIL;
	}

	sql_max_len = value_esc_max_len+100;

	DBescape_string(value,value_esc,value_esc_max_len);
	DBexecute("insert into history_text (clock,itemid,value) values (%d," ZBX_FS_UI64 ",'%s')",
		clock,itemid,value_esc);

	free(value_esc);

	return SUCCEED;

#endif
}


int	DBadd_history_log(zbx_uint64_t itemid, char *value, int clock, int timestamp,char *source, int severity)
{
	char	value_esc[MAX_STRING_LEN];
	char	source_esc[MAX_STRING_LEN];

	zabbix_log(LOG_LEVEL_DEBUG,"In add_history_log()");

	DBescape_string(value,value_esc,MAX_STRING_LEN);
	DBescape_string(source,source_esc,MAX_STRING_LEN);
	DBexecute("insert into history_log (clock,itemid,timestamp,value,source,severity) values (%d," ZBX_FS_UI64 ",%d,'%s','%s',%d)",
		clock,itemid,timestamp,value_esc,source_esc,severity);

	return SUCCEED;
}


int	DBget_items_count(void)
{
	int	res;
	char	sql[MAX_STRING_LEN];
	DB_RESULT	result;
	DB_ROW		row;

	zabbix_log(LOG_LEVEL_DEBUG,"In DBget_items_count()");

	result = DBselect("select count(*) from items");

	row=DBfetch(result);

	if(!row || DBis_null(row[0])==SUCCEED)
	{
		zabbix_log(LOG_LEVEL_ERR, "Cannot execute query [%s]", sql);
		zabbix_syslog("Cannot execute query [%s]", sql);
		DBfree_result(result);
		return 0;
	}

	res  = atoi(row[0]);

	DBfree_result(result);

	return res;
}

int	DBget_triggers_count(void)
{
	int	res;
	DB_RESULT	result;
	DB_ROW		row;

	zabbix_log(LOG_LEVEL_DEBUG,"In DBget_triggers_count()");

	result = DBselect("select count(*) from triggers");

	row=DBfetch(result);

	if(!row || DBis_null(row[0])==SUCCEED)
	{
		zabbix_log(LOG_LEVEL_ERR, "Cannot execute query");
		zabbix_syslog("Cannot execute query");
		DBfree_result(result);
		return 0;
	}

	res  = atoi(row[0]);

	DBfree_result(result);

	return res;
}

int	DBget_items_unsupported_count(void)
{
	int	res;
	DB_RESULT	result;
	DB_ROW		row;

	zabbix_log(LOG_LEVEL_DEBUG,"In DBget_items_unsupported_count()");

	result = DBselect("select count(*) from items where status=%d", ITEM_STATUS_NOTSUPPORTED);

	row=DBfetch(result);

	if(!row || DBis_null(row[0])==SUCCEED)
	{
		zabbix_log(LOG_LEVEL_ERR, "Cannot execute query");
		zabbix_syslog("Cannot execute query");
		DBfree_result(result);
		return 0;
	}

	res  = atoi(row[0]);

	DBfree_result(result);

	return res;
}

int	DBget_history_str_count(void)
{
	int	res;
	DB_RESULT	result;
	DB_ROW		row;

	zabbix_log(LOG_LEVEL_DEBUG,"In DBget_history_str_count()");

	result = DBselect("select count(*) from history_str");

	row=DBfetch(result);

	if(!row || DBis_null(row[0])==SUCCEED)
	{
		zabbix_log(LOG_LEVEL_ERR, "Cannot execute query");
		zabbix_syslog("Cannot execute query");
		DBfree_result(result);
		return 0;
	}

	res  = atoi(row[0]);

	DBfree_result(result);

	return res;
}

int	DBget_history_count(void)
{
	int	res;
	DB_RESULT	result;
	DB_ROW		row;

	zabbix_log(LOG_LEVEL_DEBUG,"In DBget_history_count()");

	result = DBselect("select count(*) from history");

	row=DBfetch(result);

	if(!row || DBis_null(row[0])==SUCCEED)
	{
		zabbix_log(LOG_LEVEL_ERR, "Cannot execute query");
		zabbix_syslog("Cannot execute query");
		DBfree_result(result);
		return 0;
	}

	res  = atoi(row[0]);

	DBfree_result(result);

	return res;
}

int	DBget_trends_count(void)
{
	int	res;
	DB_RESULT	result;
	DB_ROW		row;

	zabbix_log(LOG_LEVEL_DEBUG,"In DBget_trends_count()");

	result = DBselect("select count(*) from trends");

	row=DBfetch(result);

	if(!row || DBis_null(row[0])==SUCCEED)
	{
		zabbix_log(LOG_LEVEL_ERR, "Cannot execute query");
		zabbix_syslog("Cannot execute query");
		DBfree_result(result);
		return 0;
	}

	res  = atoi(row[0]);

	DBfree_result(result);

	return res;
}

int	DBget_queue_count(void)
{
	int	res;
	DB_RESULT	result;
	DB_ROW		row;
	int	now;

	zabbix_log(LOG_LEVEL_DEBUG,"In DBget_queue_count()");

	now=time(NULL);
/*	zbx_snprintf(sql,sizeof(sql),"select count(*) from items i,hosts h where i.status=%d and i.type not in (%d) and h.status=%d and i.hostid=h.hostid and i.nextcheck<%d and i.key_<>'status'", ITEM_STATUS_ACTIVE, ITEM_TYPE_TRAPPER, HOST_STATUS_MONITORED, now);*/
	result = DBselect("select count(*) from items i,hosts h where i.status=%d and i.type not in (%d) and ((h.status=%d and h.available!=%d) or (h.status=%d and h.available=%d and h.disable_until<=%d)) and i.hostid=h.hostid and i.nextcheck<%d and i.key_ not in ('%s','%s','%s','%s')", ITEM_STATUS_ACTIVE, ITEM_TYPE_TRAPPER, HOST_STATUS_MONITORED, HOST_AVAILABLE_FALSE, HOST_STATUS_MONITORED, HOST_AVAILABLE_FALSE, now, now, SERVER_STATUS_KEY, SERVER_ICMPPING_KEY, SERVER_ICMPPINGSEC_KEY, SERVER_ZABBIXLOG_KEY);

	row=DBfetch(result);

	if(!row || DBis_null(row[0])==SUCCEED)
	{
		zabbix_log(LOG_LEVEL_ERR, "Cannot execute query");
		zabbix_syslog("Cannot execute query");
		DBfree_result(result);
		return 0;
	}

	res  = atoi(row[0]);

	DBfree_result(result);

	return res;
}

int	DBadd_alert(zbx_uint64_t actionid, zbx_uint64_t userid, zbx_uint64_t triggerid,  zbx_uint64_t mediatypeid, char *sendto, char *subject, char *message, int maxrepeats, int repeatdelay)
{
	int	now;
	char	sendto_esc[MAX_STRING_LEN];
	char	subject_esc[MAX_STRING_LEN];
	char	message_esc[MAX_STRING_LEN];

	zabbix_log(LOG_LEVEL_DEBUG,"In add_alert(triggerid[%d])",triggerid);

	now = time(NULL);
/* Does not work on PostgreSQL */
/*	zbx_snprintf(sql,sizeof(sql),"insert into alerts (alertid,actionid,clock,mediatypeid,sendto,subject,message,status,retries) values (NULL,%d,%d,%d,'%s','%s','%s',0,0)",actionid,now,mediatypeid,sendto,subject,message);*/
	DBescape_string(sendto,sendto_esc,MAX_STRING_LEN);
	DBescape_string(subject,subject_esc,MAX_STRING_LEN);
	DBescape_string(message,message_esc,MAX_STRING_LEN);
	DBexecute("insert into alerts (actionid,triggerid,userid,clock,mediatypeid,sendto,subject,message,status,retries,maxrepeats,delay)"
		" values (" ZBX_FS_UI64 "," ZBX_FS_UI64 "," ZBX_FS_UI64 ",%d," ZBX_FS_UI64 ",'%s','%s','%s',0,0,%d,%d)",
		actionid,triggerid,userid,now,mediatypeid,sendto_esc,subject_esc,message_esc, maxrepeats, repeatdelay);

	return SUCCEED;
}

void	DBvacuum(void)
{
#ifdef	HAVE_PGSQL
	char *table_for_housekeeping[]={"services", "services_links", "graphs_items", "graphs", "sysmaps_links",
			"sysmaps_elements", "sysmaps", "config", "groups", "hosts_groups", "alerts",
			"actions", "events", "functions", "history", "history_str", "hosts", "trends",
			"items", "media", "media_type", "triggers", "trigger_depends", "users",
			"sessions", "rights", "service_alarms", "profiles", "screens", "screens_items",
			NULL};

	char	sql[MAX_STRING_LEN];
	char	*table;
	int	i;

	zbx_setproctitle("housekeeper [vacuum DB]");

	i=0;
	while (NULL != (table = table_for_housekeeping[i++]))
	{
		DBexecute("vacuum analyze %s", table);
	}
#endif

#ifdef	HAVE_MYSQL
	/* Nothing to do */
#endif
}

/* Broken:
void    DBescape_string(char *from, char *to, int maxlen)
{
	int	i,ptr;
	char	*f;

	ptr=0;
	f=(char *)strdup(from);
	for(i=0;f[i]!=0;i++)
	{
		if( (f[i]=='\'') || (f[i]=='\\'))
		{
			if(ptr>maxlen-1)	break;
			to[ptr]='\\';
			if(ptr+1>maxlen-1)	break;
			to[ptr+1]=f[i];
			ptr+=2;
		}
		else
		{
			if(ptr>maxlen-1)	break;
			to[ptr]=f[i];
			ptr++;
		}
	}
	free(f);

	to[ptr]=0;
	to[maxlen-1]=0;
}
*/

void    DBescape_string(char *from, char *to, int maxlen)
{
	int     i,ptr;

	assert(from);
	assert(to);

	maxlen--;
	for(i=0, ptr=0; from && from[i] && ptr < maxlen; i++)
	{
#ifdef	HAVE_ORACLE
		if( (from[i] == '\''))
		{
			to[ptr++] = '\'';
#else /* HAVE_ORACLE */
		if( (from[i] == '\'') || (from[i] == '\\'))
		{
			to[ptr++] = '\\';
#endif
			if(ptr >= maxlen)       break;
		}
		to[ptr++] = from[i];
	}
	to[ptr]=0;
}

void	DBget_item_from_db(DB_ITEM *item,DB_ROW row)
{
	char	*s;

	ZBX_STR2UINT64(item->itemid, row[0]);
//	item->itemid=atoi(row[0]);
	strscpy(item->key,row[1]);
	item->host=row[2];
	item->port=atoi(row[3]);
	item->delay=atoi(row[4]);
	item->description=row[5];
	item->nextcheck=atoi(row[6]);
	item->type=atoi(row[7]);
	item->snmp_community=row[8];
	item->snmp_oid=row[9];
	item->useip=atoi(row[10]);
	item->ip=row[11];
	item->history=atoi(row[12]);
	s=row[13];
	if(DBis_null(s)==SUCCEED)
	{
		item->lastvalue_null=1;
	}
	else
	{
		item->lastvalue_null=0;
		item->lastvalue_str=s;
		item->lastvalue=atof(s);
	}
	s=row[14];
	if(DBis_null(s)==SUCCEED)
	{
		item->prevvalue_null=1;
	}
	else
	{
		item->prevvalue_null=0;
		item->prevvalue_str=s;
		item->prevvalue=atof(s);
	}
//	item->hostid=atoi(row[15]);
	ZBX_STR2UINT64(item->hostid, row[15]);
	item->host_status=atoi(row[16]);
	item->value_type=atoi(row[17]);

	item->host_errors_from=atoi(row[18]);
	item->snmp_port=atoi(row[19]);
	item->delta=atoi(row[20]);

	s=row[21];
	if(DBis_null(s)==SUCCEED)
	{
		item->prevorgvalue_null=1;
	}
	else
	{
		item->prevorgvalue_null=0;
		item->prevorgvalue=atof(s);
	}
	s=row[22];
	if(DBis_null(s)==SUCCEED)
	{
		item->lastclock=0;
	}
	else
	{
		item->lastclock=atoi(s);
	}

	item->units=row[23];
	item->multiplier=atoi(row[24]);

	item->snmpv3_securityname = row[25];
	item->snmpv3_securitylevel = atoi(row[26]);
	item->snmpv3_authpassphrase = row[27];
	item->snmpv3_privpassphrase = row[28];
	item->formula = row[29];
	item->host_available=atoi(row[30]);
	item->status=atoi(row[31]);
	item->trapper_hosts=row[32];
	item->logtimefmt=row[33];
	ZBX_STR2UINT64(item->valuemapid, row[34]);
//	item->valuemapid=atoi(row[34]);
	item->delay_flex=row[35];
}

/*
 * Execute SQL statement. For select statements only.
 * If fails, program terminates.
 */ 
zbx_uint64_t DBget_nextid(char *table, char *field)
{
	DB_RESULT	result;
	DB_ROW		row;
	zbx_uint64_t	res;
	zbx_uint64_t	min;
	zbx_uint64_t	max;

	zabbix_log(LOG_LEVEL_DEBUG,"In DBget_nextid(%s,%s)", table, field);

	min = (zbx_uint64_t)100000000000000*(zbx_uint64_t)CONFIG_NODEID;
	max = (zbx_uint64_t)100000000000000*(zbx_uint64_t)(CONFIG_NODEID+1)-1;

	result = DBselect("select max(%s) from %s where %s>=" ZBX_FS_UI64 " and %s<=" ZBX_FS_UI64, field, table, field, min, field, max);
//	zabbix_log(LOG_LEVEL_WARNING, "select max(%s) from %s where %s>=" ZBX_FS_UI64 " and %s<=" ZBX_FS_UI64, field, table, field, min, field, max);

	row=DBfetch(result);

	if(row && (DBis_null(row[0])!=SUCCEED))
	{
		sscanf(row[0],ZBX_FS_UI64,&res);

		res++;
	}
	else
	{
//	zabbix_log(LOG_LEVEL_WARNING,"4");
		res=(zbx_uint64_t)100000000000000*(zbx_uint64_t)CONFIG_NODEID+1;
	}
	DBfree_result(result);
//	zabbix_log(LOG_LEVEL_WARNING, ZBX_FS_UI64, res);

	return res;
}
