#include <stdlib.h>
#include <stdio.h>

#include "db.h"
#include "log.h"
#include "common.h"

#ifdef	HAVE_MYSQL
	MYSQL	mysql;
#endif

#ifdef	HAVE_PGSQL
	PGconn	*conn;
#endif

void	DBclose(void)
{
#ifdef	HAVE_MYSQL
	mysql_close(&mysql);
#endif
#ifdef	HAVE_PGSQL
	PQfinish(conn);
#endif
}

/*
 * Connect to database.
 * If fails, program terminates.
 */ 
void    DBconnect( char *dbname, char *dbuser, char *dbpassword, char *dbsocket)
{
/*	zabbix_log(LOG_LEVEL_ERR, "[%s] [%s] [%s]\n",dbname, dbuser, dbpassword ); */
#ifdef	HAVE_MYSQL
/* For MySQL >3.22.00 */
/*	if( ! mysql_connect( &mysql, NULL, dbuser, dbpassword ) )*/
	if( ! mysql_real_connect( &mysql, NULL, dbuser, dbpassword, dbname, 3306, dbsocket,0 ) )
	{
		zabbix_log(LOG_LEVEL_ERR, "Failed to connect to database: Error: %s\n",mysql_error(&mysql) );
		exit( FAIL );
	}
	if( mysql_select_db( &mysql, dbname ) != 0 )
	{
		zabbix_log(LOG_LEVEL_ERR, "Failed to select database: Error: %s\n",mysql_error(&mysql) );
		exit( FAIL );
	}
#endif
#ifdef	HAVE_PGSQL
/*	conn = PQsetdb(pghost, pgport, pgoptions, pgtty, dbName); */
/*	conn = PQsetdb(NULL, NULL, NULL, NULL, dbname);*/
	conn = PQsetdbLogin(NULL, NULL, NULL, NULL, dbname, dbuser, dbpassword );

/* check to see that the backend connection was successfully made */
	if (PQstatus(conn) == CONNECTION_BAD)
	{
		zabbix_log(LOG_LEVEL_ERR, "Connection to database '%s' failed.\n", dbname);
		zabbix_log(LOG_LEVEL_ERR, "%s", PQerrorMessage(conn));
		exit(FAIL);
	}
#endif
}

/*
 * Execute SQL statement. For non-select statements only.
 * If fails, program terminates.
 */ 
int	DBexecute(char *query)
{

#ifdef	HAVE_MYSQL
	zabbix_log( LOG_LEVEL_DEBUG, "Executing query:%s\n",query); 
/*	zabbix_log( LOG_LEVEL_WARNING, "Executing query:%s\n",query)*/;

	if( mysql_query(&mysql,query) != 0 )
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",query);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s", mysql_error(&mysql) );
		return FAIL;
	}
#endif
#ifdef	HAVE_PGSQL
	PGresult	*result;

	zabbix_log( LOG_LEVEL_DEBUG, "Executing query:%s\n",query);

	result = PQexec(conn,query);

	if( result==NULL)
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",query);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s", "Result is NULL" );
		PQclear(result);
		return FAIL;
	}
	if( PQresultStatus(result) != PGRES_COMMAND_OK)
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",query);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s", PQresStatus(PQresultStatus(result)) );
		PQclear(result);
		return FAIL;
	}
	PQclear(result);
#endif
	return	SUCCEED;
}

/*
 * Execute SQL statement. For select statements only.
 * If fails, program terminates.
 */ 
DB_RESULT *DBselect(char *query)
{
#ifdef	HAVE_MYSQL
	zabbix_log( LOG_LEVEL_DEBUG, "Executing query:%s\n",query);
/*	zabbix_log( LOG_LEVEL_WARNING, "Executing query:%s\n",query);*/

	if( mysql_query(&mysql,query) != 0 )
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",query);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s", mysql_error(&mysql) );
		exit( FAIL );
	}
	return	mysql_store_result(&mysql);
#endif
#ifdef	HAVE_PGSQL
	PGresult	*result;

	zabbix_log( LOG_LEVEL_DEBUG, "Executing query:%s\n",query);
	result = PQexec(conn,query);

	if( result==NULL)
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",query);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s", "Result is NULL" );
		exit( FAIL );
	}
	if( PQresultStatus(result) != PGRES_TUPLES_OK)
	{
		zabbix_log( LOG_LEVEL_ERR, "Query::%s",query);
		zabbix_log(LOG_LEVEL_ERR, "Query failed:%s", PQresStatus(PQresultStatus(result)) );
		exit( FAIL );
	}
	return result;
#endif
}

/*
 * Get value for given row and field. Must be called after DBselect.
 */ 
char	*DBget_field(DB_RESULT *result, int rownum, int fieldnum)
{
#ifdef	HAVE_MYSQL
	MYSQL_ROW	row;

	mysql_data_seek(result, rownum);
	row=mysql_fetch_row(result);
	zabbix_log(LOG_LEVEL_DEBUG, "Got field:%s", row[fieldnum] );
	return row[fieldnum];
#endif
#ifdef	HAVE_PGSQL
	return PQgetvalue(result, rownum, fieldnum);
#endif
}

/*
 * Return SUCCEED if result conains no records
 */ 
int	DBis_empty(DB_RESULT *result)
{
	if(result == NULL)
	{
		return	SUCCEED;
	}
	if(DBnum_rows(result) == 0)
	{
		return	SUCCEED;
	}
/* This is necessary to exclude situations like
 * atoi(DBget_field(result,0,0). This lead to coredump.
 */
	if(DBget_field(result,0,0) == 0)
	{
		return	SUCCEED;
	}

	return FAIL;
}

/*
 * Get number of selected records.
 */ 
int	DBnum_rows(DB_RESULT *result)
{
#ifdef	HAVE_MYSQL
	return mysql_num_rows(result);
#endif
#ifdef	HAVE_PGSQL
	return PQntuples(result);
#endif
}

/*
 * Get function value.
 */ 
int     DBget_function_result(float *Result,char *FunctionID)
{
	DB_RESULT *result;

        char	c[MAX_STRING_LEN+1];
        int	rows;

	sprintf( c, "select lastvalue from functions where functionid=%s", FunctionID );
	result = DBselect(c);

	if(result == NULL)
	{
        	DBfree_result(result);
		zabbix_log(LOG_LEVEL_WARNING, "Query failed for functionid:[%s]", FunctionID );
		return FAIL;	
	}
        rows = DBnum_rows(result);
	if(rows == 0)
	{
        	DBfree_result(result);
		zabbix_log(LOG_LEVEL_WARNING, "Query failed for functionid:[%s]", FunctionID );
		return FAIL;	
	}
        *Result=atof(DBget_field(result,0,0));
        DBfree_result(result);

        return SUCCEED;
}
