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
#include "zbxdb.h"
#include "log.h"
#include "zlog.h"

#ifdef	HAVE_SQLITE3
	int		sqlite_transaction_started = 0;
	sqlite3		*conn = NULL;
	PHP_MUTEX	sqlite_access;
#endif

#ifdef	HAVE_MYSQL
	MYSQL	*conn = NULL;
#endif

#ifdef	HAVE_POSTGRESQL
	PGconn	*conn = NULL;
	int     ZBX_PG_BYTEAOID = 0;
#endif

#ifdef	HAVE_ORACLE
	sqlo_db_handle_t oracle;
#endif

void	zbx_db_close(void)
{
#ifdef	HAVE_MYSQL
	mysql_close(conn);
	conn = NULL;
#endif
#ifdef	HAVE_POSTGRESQL
	PQfinish(conn);
	conn = NULL;
#endif
#ifdef	HAVE_ORACLE
	if (SQLO_SUCCESS != sqlo_finish(oracle))
		zabbix_errlog(ERR_Z3004, 0, "sqlo_finish");
#endif
#ifdef	HAVE_SQLITE3
	sqlite_transaction_started = 0;
	sqlite3_close(conn);
	conn = NULL;
/*	php_sem_remove(&sqlite_access);*/
#endif
}


/*
 * Connect to the database.
 * If fails, program terminates.
 */ 
int	zbx_db_connect(char *host, char *user, char *password, char *dbname, char *dbsocket, int port)
{
	int	ret = ZBX_DB_OK;
#ifdef	HAVE_SQLITE3
	char	*p, *path;
#endif /* HAVE_SQLITE3 */

#ifdef	HAVE_MYSQL
	/* For MySQL >3.22.00 */
	/*	if( ! mysql_connect( conn, NULL, dbuser, dbpassword ) )*/

	conn = mysql_init(NULL);

	if (!mysql_real_connect(conn, host, user, password, dbname, port, dbsocket, CLIENT_MULTI_STATEMENTS))
	{
		zabbix_errlog(ERR_Z3001, dbname, mysql_errno(conn), mysql_error(conn));
		ret = ZBX_DB_FAIL;
	}

	if (ZBX_DB_OK == ret)
	{
		if (0 != mysql_select_db(conn, dbname))
		{
			zabbix_errlog(ERR_Z3001, dbname, mysql_errno(conn), mysql_error(conn));
			ret = ZBX_DB_FAIL;
		}
	}

	if (ZBX_DB_OK == ret)
		DBexecute("SET CHARACTER SET utf8");

	if (ZBX_DB_FAIL == ret)
	{
		switch (mysql_errno(conn)) {
		case CR_SERVER_GONE_ERROR:
		case CR_CONNECTION_ERROR:
		case CR_SERVER_LOST:
		case ER_SERVER_SHUTDOWN:
		case ER_UNKNOWN_ERROR:
			ret = ZBX_DB_DOWN;
			break;
		default:
			break;
		}
	}

	return ret;
#endif
#ifdef	HAVE_POSTGRESQL
	char		*cport = NULL;
	DB_RESULT	result;
	DB_ROW		row;

	if( port )	cport = zbx_dsprintf(cport, "%i", port);

	conn = PQsetdbLogin(host, cport, NULL, NULL, dbname, user, password );

	zbx_free(cport);

	/* check to see that the backend connection was successfully made */
	if (PQstatus(conn) != CONNECTION_OK)
	{
		zabbix_errlog(ERR_Z3001, dbname, 0, PQerrorMessage(conn));
		ret = ZBX_DB_FAIL;
	}

	result = DBselect("select oid from pg_type where typname = 'bytea'");
	row = DBfetch(result);
	if(row)
	{
		ZBX_PG_BYTEAOID = atoi(row[0]);
	}
	DBfree_result(result);

	return ret;
#endif
#ifdef	HAVE_ORACLE
	char	connect[MAX_STRING_LEN];

	zbx_strlcpy(connect, user, sizeof(connect));

	if (password && *password) {
		zbx_strlcat(connect, "/", sizeof(connect));
		zbx_strlcat(connect, password, sizeof(connect));

		if (dbname && *dbname) {
			zbx_strlcat(connect, "@", sizeof(connect));
			zbx_strlcat(connect, dbname, sizeof(connect));
		}
	}

	if (SQLO_SUCCESS != sqlo_init(SQLO_OFF, 1, 100)) {
		zabbix_errlog(ERR_Z3001, connect, 0, "Failed to init libsqlora8");
		ret = ZBX_DB_FAIL;
	}

	if (ZBX_DB_OK == ret) {
		/* login */ /* TODO: how to use port??? */
		if (SQLO_SUCCESS != sqlo_connect(&oracle, connect)) {
			zabbix_errlog(ERR_Z3001, connect, 0, "sqlo_connect");
			ret = ZBX_DB_FAIL;
		}
	}

	if (ZBX_DB_OK == ret)
		sqlo_autocommit_off(oracle);

	return ret;
#endif
#ifdef	HAVE_SQLITE3
	/* check to see that the backend connection was successfully made */
	if (SQLITE_OK != (ret = sqlite3_open(dbname, &conn))) {
		zabbix_errlog(ERR_Z3001, dbname, 0, sqlite3_errmsg(conn));
		exit(FAIL);
	}

	/* Do not return SQLITE_BUSY immediately, wait for N ms */
	sqlite3_busy_timeout(conn, 60*1000);

	sqlite_transaction_started = 0;

	path = strdup(dbname);
	if (NULL != (p = strrchr(path, '/')))
		*++p = '\0';
	else
		*path = '\0';

	DBexecute("PRAGMA synchronous = 0"); /* OFF */
	DBexecute("PRAGMA temp_store = 2"); /* MEMORY */
	DBexecute("PRAGMA temp_store_directory = '%s'", path);

	zbx_free(path);

	return ret;
#endif
}

void	zbx_db_init(char *host, char *user, char *password, char *dbname, char *dbsocket, int port)
{
#ifdef	HAVE_SQLITE3
	int		ret;
	struct stat	buf;
#endif

#ifdef	HAVE_SQLITE3
	if (0 != stat(dbname, &buf)) {
		zabbix_log(LOG_LEVEL_WARNING, "Cannot open database file \"%s\": %s", dbname, strerror(errno));
		zabbix_log(LOG_LEVEL_WARNING, "Creating database ...");

		ret = sqlite3_open(dbname, &conn);
		if (SQLITE_OK != ret) {
			zabbix_errlog(ERR_Z3002, dbname, 0, sqlite3_errmsg(conn));
			exit(FAIL);
		}

		DBexecute("%s", db_schema);
		DBclose();
	}
#endif
}

int __zbx_zbx_db_execute(const char *fmt, ...)
{
	va_list args;
	int ret;

	va_start(args, fmt);
	ret = zbx_db_vexecute(fmt, args);
	va_end(args);

	return ret;
}

#ifdef HAVE___VA_ARGS__
#	define zbx_db_select(fmt, ...)	__zbx_zbx_db_select(ZBX_CONST_STRING(fmt), ##__VA_ARGS__)
#else
#	define zbx_db_select __zbx_zbx_db_select
#endif /* HAVE___VA_ARGS__ */
static DB_RESULT __zbx_zbx_db_select(const char *fmt, ...)
{
	va_list args;
	DB_RESULT	result;

	va_start(args, fmt);
	result = zbx_db_vselect(fmt, args);
	va_end(args);

	return result;
}


/******************************************************************************
 *                                                                            *
 * Function: DBbegin                                                          *
 *                                                                            *
 * Purpose: Start transaction                                                 *
 *                                                                            *
 * Parameters: -                                                              *
 *                                                                            *
 * Return value: -                                                            *
 *                                                                            *
 * Author: Eugene Grigorjev                                                   *
 *                                                                            *
 * Comments: Do nothing of DB does not support transactions                   *
 *                                                                            *
 ******************************************************************************/
void	zbx_db_begin(void)
{
#ifdef	HAVE_MYSQL
	zbx_db_execute("%s","begin;");
#endif
#ifdef	HAVE_POSTGRESQL
	zbx_db_execute("%s","begin;");
#endif
#ifdef	HAVE_SQLITE3
	sqlite_transaction_started++;
	
	if(sqlite_transaction_started == 1)
	{
		php_sem_acquire(&sqlite_access);

		zbx_db_execute("%s","begin;");
	}
	else
	{
		zabbix_log( LOG_LEVEL_DEBUG, "POSSIBLE ERROR: Used incorect logic in database processing started subtransaction!");
	}
#endif
}

/******************************************************************************
 *                                                                            *
 * Function: DBcommit                                                         *
 *                                                                            *
 * Purpose: Commit transaction                                                *
 *                                                                            *
 * Parameters: -                                                              *
 *                                                                            *
 * Return value: -                                                            *
 *                                                                            *
 * Author: Eugene Grigorjev                                                   *
 *                                                                            *
 * Comments: Do nothing of DB does not support transactions                   *
 *                                                                            *
 ******************************************************************************/
void zbx_db_commit(void)
{
#ifdef	HAVE_MYSQL
	zbx_db_execute("%s","commit;");
#endif
#ifdef	HAVE_POSTGRESQL
	zbx_db_execute("%s","commit;");
#endif
#ifdef	HAVE_ORACLE
	zbx_db_execute("%s","commit");
#endif
#ifdef	HAVE_SQLITE3

	if(sqlite_transaction_started > 1)
	{
		sqlite_transaction_started--;
	}
	
	if(sqlite_transaction_started == 1)
	{
		zbx_db_execute("%s","commit;");

		sqlite_transaction_started = 0;

		php_sem_release(&sqlite_access);
	}

#endif
}

/******************************************************************************
 *                                                                            *
 * Function: DBrollback                                                       *
 *                                                                            *
 * Purpose: Rollback transaction                                              *
 *                                                                            *
 * Parameters: -                                                              *
 *                                                                            *
 * Return value: -                                                            *
 *                                                                            *
 * Author: Eugene Grigorjev                                                   *
 *                                                                            *
 * Comments: Do nothing of DB does not support transactions                   *
 *                                                                            *
 ******************************************************************************/
void zbx_db_rollback(void)
{
#ifdef	HAVE_MYSQL
	zbx_db_execute("rollback;");
#endif
#ifdef	HAVE_POSTGRESQL
	zbx_db_execute("rollback;");
#endif
#ifdef	HAVE_ORACLE
	zbx_db_execute("rollback");
#endif
#ifdef	HAVE_SQLITE3

	if(sqlite_transaction_started > 1)
	{
		sqlite_transaction_started--;
	}

	if(sqlite_transaction_started == 1)
	{
		zbx_db_execute("rollback;");

		sqlite_transaction_started = 0;

		php_sem_release(&sqlite_access);
	}

#endif
}

/*
 * Execute SQL statement. For non-select statements only.
 * If fails, program terminates.
 */ 
int zbx_db_vexecute(const char *fmt, va_list args)
{
	char	*sql = NULL;
	int	ret = ZBX_DB_OK;

/*	double	sec;*/

#ifdef	HAVE_POSTGRESQL
	PGresult	*result;
	char		*error = NULL;
#endif
#ifdef	HAVE_SQLITE3
	char *error=0;
#endif
#ifdef HAVE_MYSQL
	int		status;
#endif

/*	sec = zbx_time();*/

	sql = zbx_dvsprintf(sql, fmt, args);

	zabbix_log( LOG_LEVEL_DEBUG, "Query [%s]", sql);
#ifdef	HAVE_MYSQL
	if(!conn)
	{
		zabbix_errlog(ERR_Z3003);
		ret = ZBX_DB_FAIL;
	}
	else
	{
		if (0 != (status = mysql_query(conn,sql)))
		{
			zabbix_errlog(ERR_Z3005, mysql_errno(conn), mysql_error(conn), sql);
			switch(mysql_errno(conn)) {
				case	CR_SERVER_GONE_ERROR:
				case	CR_CONNECTION_ERROR:
				case	CR_SERVER_LOST:
				case	ER_SERVER_SHUTDOWN:
				case	ER_UNKNOWN_ERROR:
					ret = ZBX_DB_DOWN;
					break;
				default:
					ret = ZBX_DB_FAIL;
					break;
			}
		}
		else
		{
			do {
				if (mysql_field_count(conn) == 0)
				{
/*					zabbix_log(LOG_LEVEL_DEBUG, ZBX_FS_UI64 " rows affected",
							(zbx_uint64_t)mysql_affected_rows(conn));*/
					ret += (int)mysql_affected_rows(conn);
				}
				else  /* some error occurred */
				{
					zabbix_log(LOG_LEVEL_DEBUG, "Could not retrieve result set");
					break;
				}

				/* more results? -1 = no, >0 = error, 0 = yes (keep looping) */
			        if ((status = mysql_next_result(conn)) > 0)
					zabbix_errlog(ERR_Z3005, mysql_errno(conn), mysql_error(conn), sql);
			} while (status == 0);
		}
	}
#endif
#ifdef	HAVE_POSTGRESQL
	result = PQexec(conn,sql);

	if( result==NULL)
	{
		zabbix_errlog(ERR_Z3005, 0, "Result is NULL", sql);
		ret = ZBX_DB_FAIL;
	}
	else if( PQresultStatus(result) != PGRES_COMMAND_OK)
	{
		error = zbx_dsprintf(error, "%s:%s",
				PQresStatus(PQresultStatus(result)),
				PQresultErrorMessage(result));
		zabbix_errlog(ERR_Z3005, 0, error, sql);
		zbx_free(error);
		ret = ZBX_DB_FAIL;
	}

	if(ret == ZBX_DB_OK)
	{
		ret = PQntuples(result);
	}
	PQclear(result);
#endif
#ifdef	HAVE_ORACLE
	if ((ret = sqlo_exec(oracle, sql))<0)
	{
		zabbix_errlog(ERR_Z3005, 0, sqlo_geterror(oracle), sql);
		ret = ZBX_DB_FAIL;
	}
#endif
#ifdef	HAVE_SQLITE3
	if (!sqlite_transaction_started)
	{
		php_sem_acquire(&sqlite_access);
	}
	
lbl_exec:
	if (SQLITE_OK != (ret = sqlite3_exec(conn, sql, NULL, 0, &error)))
	{
		if (ret == SQLITE_BUSY)
			goto lbl_exec; /* attention deadlock!!! */

		zabbix_errlog(ERR_Z3005, 0, error, sql);
		sqlite3_free(error);
		ret = ZBX_DB_FAIL;
	}

	if (ret == ZBX_DB_OK)
	{
		ret = sqlite3_changes(conn);
	}

	if (!sqlite_transaction_started)
	{
		php_sem_release(&sqlite_access);
	}
#endif

/*	sec = zbx_time() - sec;	
	if(sec > 0.1)
		zabbix_log( LOG_LEVEL_WARNING, "Long query: " ZBX_FS_DBL " sec, \"%s\"", sec, sql);*/

	zbx_free(sql);

	return ret;
}


int	zbx_db_is_null(char *field)
{
	int ret = FAIL;

	if(field == NULL)	ret = SUCCEED;
#ifdef HAVE_ORACLE
	else if(field[0] == 0)	ret = SUCCEED;
#endif
	return ret;
}

#ifdef  HAVE_POSTGRESQL
/* in db.h - #define DBfree_result   PG_DBfree_result */
void	PG_DBfree_result(DB_RESULT result)
{
	if(!result) return;

	/* free old data */
	if(result->values)
	{
		result->fld_num = 0;
		zbx_free(result->values);
		result->values = NULL;
	}

	PQclear(result->pg_result);
	zbx_free(result);
}
#endif
#ifdef  HAVE_SQLITE3
/* in db.h - #define DBfree_result   SQ_DBfree_result */
void	SQ_DBfree_result(DB_RESULT result)
{
	if(!result) return;
	
	if(result->data)
	{
		sqlite3_free_table(result->data);
	}

	zbx_free(result);
}
#endif

DB_ROW	zbx_db_fetch(DB_RESULT result)
{
#ifdef	HAVE_MYSQL
	if(!result)	return NULL;

	return mysql_fetch_row(result);
#endif
#ifdef	HAVE_POSTGRESQL

	int	i;

	/* EOF */
	if(!result)	return NULL;
		
	/* free old data */
	if(result->values)
	{
		zbx_free(result->values);
		result->values = NULL;
	}
	
	/* EOF */
	if(result->cursor == result->row_num) return NULL;

	/* init result */	
	result->fld_num = PQnfields(result->pg_result);

	if(result->fld_num > 0)
	{
		result->values = zbx_malloc(result->values, sizeof(char*) * result->fld_num);
		for(i = 0; i < result->fld_num; i++)
		{
			if(PQgetisnull(result->pg_result, result->cursor, i))
			{
				result->values[i] = NULL;
			}
			else
			{
				result->values[i] = PQgetvalue(result->pg_result, result->cursor, i);
				if(PQftype(result->pg_result,i) == ZBX_PG_BYTEAOID) /* binary data type BYTEAOID */
					zbx_pg_unescape_bytea((u_char *)result->values[i]);
			}
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
		return (DB_ROW)sqlo_values(result, NULL, 1);
	}
	else if(SQLO_NO_DATA == res)
	{
		return NULL;
	}

	zabbix_errlog(ERR_Z3006, 0, sqlo_geterror(oracle));
	exit(FAIL);
	return NULL;
#endif
#ifdef HAVE_SQLITE3
	
	/* EOF */
	if(!result)	return NULL;
		
	/* EOF */
	if(result->curow >= result->nrow) return NULL;

	if(!result->data) return NULL;

	result->curow++; /* NOTE: First row == header row */

	return &(result->data[result->curow * result->ncolumn]);	
#endif

	return NULL;
}

/*
 * Execute SQL statement. For select statements only.
 * If fails, program terminates.
 */ 
DB_RESULT zbx_db_vselect(const char *fmt, va_list args)
{
	char	*sql = NULL;
	DB_RESULT result;

/*	double	sec;*/

#ifdef	HAVE_ORACLE
	sqlo_stmt_handle_t sth;
#endif
#ifdef	HAVE_SQLITE3
	int ret = FAIL;
	char *error=NULL;
#endif
#ifdef	HAVE_POSTGRESQL
	char	*error = NULL;
#endif

/*	sec = zbx_time();*/

	sql = zbx_dvsprintf(sql, fmt, args);

	zabbix_log( LOG_LEVEL_DEBUG, "Query [%s]", sql);

#ifdef	HAVE_MYSQL
	if(!conn)
	{
		zabbix_errlog(ERR_Z3003);
		result = NULL;
	}
	else
	{
		if(mysql_query(conn,sql) != 0)
		{
			zabbix_errlog(ERR_Z3005, mysql_errno(conn), mysql_error(conn), sql);
			switch(mysql_errno(conn)) {
				case	CR_SERVER_GONE_ERROR:
				case	CR_CONNECTION_ERROR:
				case	CR_SERVER_LOST:
				case	ER_SERVER_SHUTDOWN:
				case	ER_UNKNOWN_ERROR:
					result = (DB_RESULT)ZBX_DB_DOWN;
					break;
				default:
					result = NULL;
					break;
			}
		}
		else
		{
			result = mysql_store_result(conn);
		}
	}
#endif
#ifdef	HAVE_POSTGRESQL
	result = zbx_malloc(NULL, sizeof(ZBX_PG_DB_RESULT));
	result->pg_result = PQexec(conn,sql);
	result->values = NULL;
	result->cursor = 0;

	if(result->pg_result==NULL)
	{
		zabbix_errlog(ERR_Z3005, 0, "Result is NULL", sql);
		exit(FAIL);
	}
	if( PQresultStatus(result->pg_result) != PGRES_TUPLES_OK)
	{
		error = zbx_dsprintf(error, "%s:%s",
				PQresStatus(PQresultStatus(result->pg_result)),
			 	PQresultErrorMessage(result->pg_result));
		zabbix_errlog(ERR_Z3005, 0, error, sql);
		zbx_free(error);
		exit(FAIL);
	}
	
	/* init rownum */	
	result->row_num = PQntuples(result->pg_result);

#endif
#ifdef	HAVE_ORACLE
	if(0 > (sth = (sqlo_open(oracle, sql,0,NULL))))
	{
		zabbix_errlog(ERR_Z3005, 0, sqlo_geterror(oracle), sql);
		exit(FAIL);
	}
	result = sth;
#endif
#ifdef HAVE_SQLITE3
	if(!sqlite_transaction_started)
	{
		php_sem_acquire(&sqlite_access);
	}

	result = zbx_malloc(NULL, sizeof(ZBX_SQ_DB_RESULT));
	result->curow = 0;

lbl_get_table:
	if(SQLITE_OK != (ret = sqlite3_get_table(conn,sql,&result->data,&result->nrow, &result->ncolumn, &error)))
	{
		if(ret == SQLITE_BUSY) goto lbl_get_table; /* attention deadlock!!! */

		zabbix_errlog(ERR_Z3005, 0, error, sql);
		sqlite3_free(error);
		if(!sqlite_transaction_started)
		{
			php_sem_release(&sqlite_access);
		}
		exit(FAIL);
	}

	if(!sqlite_transaction_started)
	{
		php_sem_release(&sqlite_access);
	}
#endif

/*	sec = zbx_time() - sec;	
	if(sec > 0.1)
		zabbix_log( LOG_LEVEL_WARNING, "Long query: " ZBX_FS_DBL " sec, \"%s\"", sec, sql);*/

	zbx_free(sql);
	return result;
}

/*
 * Get value of autoincrement field for last insert or update statement
 */ 
zbx_uint64_t	zbx_db_insert_id(int exec_result, const char *table, const char *field)
{
#ifdef	HAVE_MYSQL
	zabbix_log(LOG_LEVEL_DEBUG, "In DBinsert_id()" );
	
	if(exec_result == FAIL) return 0;
	
	return mysql_insert_id(conn);
#endif

#ifdef	HAVE_POSTGRESQL
	DB_RESULT	tmp_res;
	zbx_uint64_t	id_res = FAIL;

	zabbix_log(LOG_LEVEL_DEBUG, "In DBinsert_id()" );
	
	if(exec_result < 0) return 0;
	if(exec_result == FAIL) return 0;
	if((Oid)exec_result == InvalidOid) return 0;
	
	tmp_res = zbx_db_select("select %s from %s where oid=%i", field, table, exec_result);

	ZBX_STR2UINT64(id_res, PQgetvalue(tmp_res->pg_result, 0, 0));
/*	id_res = atoi(PQgetvalue(tmp_res->pg_result, 0, 0));*/
	
	DBfree_result(tmp_res);
	
	return id_res;
#endif

#ifdef	HAVE_ORACLE
	DB_ROW	row;
	char    sql[MAX_STRING_LEN];
	DB_RESULT       result;
	zbx_uint64_t	id;
	
	zabbix_log(LOG_LEVEL_DEBUG, "In DBinsert_id()" );

	if(exec_result == FAIL) return 0;

	zbx_snprintf(sql, sizeof(sql), "select %s_%s.currval from dual", table, field);

	result=DBselect("%s", sql);
	
	row = DBfetch(result);

	ZBX_STR2UINT64(id, row[0]);
/*	id = atoi(row[0]);*/
	DBfree_result(result);

	return id;
#endif
#ifdef	HAVE_SQLITE3
	return (zbx_uint64_t)sqlite3_last_insert_rowid(conn);
#endif
}

/*
 * Execute SQL statement. For select statements only.
 * If fails, program terminates.
 */ 
DB_RESULT zbx_db_select_n(char *query, int n)
{
#ifdef	HAVE_MYSQL
	return zbx_db_select("%s limit %d", query, n);
#endif
#ifdef	HAVE_POSTGRESQL
	return zbx_db_select("%s limit %d", query, n);
#endif
#ifdef	HAVE_ORACLE
	return zbx_db_select("select * from (%s) where rownum<=%d", query, n);
#endif
#ifdef	HAVE_SQLITE3
	return zbx_db_select("%s limit %d", query, n);
#endif
}

