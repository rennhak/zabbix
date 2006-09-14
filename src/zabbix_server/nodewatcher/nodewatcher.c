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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/wait.h>

#include <string.h>

#ifdef HAVE_NETDB_H
	#include <netdb.h>
#endif

/* Required for getpwuid */
#include <pwd.h>

#include <signal.h>
#include <errno.h>

#include <time.h>

#include "common.h"
#include "cfg.h"
#include "db.h"
#include "log.h"
#include "zlog.h"

#include "dbsync.h"
#include "nodewatcher.h"

/******************************************************************************
 *                                                                            *
 * Function: calculate_checksums                                              *
 *                                                                            *
 * Purpose: calculate check sums of configuration data                        *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value: SUCCESS - calculated succesfully                             * 
 *               FAIL - an error occured                                      *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int calculate_checksums()
{

	char	sql[64000];
	char	tmp[MAX_STRING_LEN];
	char	fields[MAX_STRING_LEN];

	int	i = 0;
	int	j;
	DB_RESULT	result;
	DB_RESULT	result2;
	DB_ROW		row,row2;

//	zabbix_log( LOG_LEVEL_WARNING, "In calculate_checksums");
	DBexecute("delete from node_cksum where cksumtype=%d", NODE_CKSUM_TYPE_NEW);
	// insert into node_cksum (select NULL,0,'items','itemid',itemid,0,md5(concat(key_)) as md5 from items);

	/* Select all nodes */
	zbx_snprintf(sql,sizeof(sql),"select nodeid from nodes");
	result =DBselect(sql);
	while((row=DBfetch(result)))
	{
		zbx_snprintf(sql,sizeof(sql),"select 'table                  ','field                ',itemid, '012345678901234' from items where 1=0\n");

		for(i=0;tables[i].table!=0;i++)
		{
//			zabbix_log( LOG_LEVEL_WARNING, "In calculate_checksums2 [%s]", tables[i].table );
			/* Do not sync some of tables */
			if( (tables[i].flags & ZBX_SYNC) ==0)	continue;

			j=0;
			fields[0]=0;
			while(tables[i].fields[j].name != 0)
			{
//				zabbix_log( LOG_LEVEL_WARNING, "In calculate_checksums2 [%s,%s]", tables[i].table,tables[i].fields[j].name );
				strncat(fields,tables[i].fields[j].name,sizeof(tmp));
				strncat(fields,",",sizeof(tmp));
				j++;
			}
			if(fields[0]!=0)	fields[strlen(fields)-1] = 0;

			// select table,recid,md5(fields) from table union all ...
			zbx_snprintf(tmp,sizeof(tmp),"union all select '%s','%s',%s,md5(concat(%s)) from %s where mod(%s,100)=%s\n",
					tables[i].table, tables[i].recid, tables[i].recid, fields, tables[i].table, tables[i].recid, row[0]);
			strncat(sql,tmp,sizeof(sql));
		}
//		zabbix_log( LOG_LEVEL_WARNING, "SQL [%s]", sql);

		result2 =DBselect(sql);
		while((row2=DBfetch(result2)))
		{
			DBexecute("insert into node_cksum (nodeid,tablename,fieldname,recordid,cksumtype,cksum)"\
				"values (%s,'%s','%s',%s,%d,'%s')",row[0],row2[0],row2[1],row2[2],NODE_CKSUM_TYPE_NEW,row2[3]);
		}
		DBfree_result(result2);
	}
	DBfree_result(result);


	return SUCCEED;
}

/******************************************************************************
 *                                                                            *
 * Function: update_checksums                                                 *
 *                                                                            *
 * Purpose: overwrite old checksums with new ones                             *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value: SUCCESS - calculated succesfully                             * 
 *               FAIL - an error occured                                      *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int update_checksums()
{
	/* Begin work */
	DBexecute("delete from node_cksum where cksumtype=%d", NODE_CKSUM_TYPE_OLD);
	DBexecute("update node_cksum set cksumtype=%d where cksumtype=%d", NODE_CKSUM_TYPE_OLD, NODE_CKSUM_TYPE_NEW);
	/* Commit */

	return SUCCEED;
}

/******************************************************************************
 *                                                                            *
 * Function: compare_checksums                                                *
 *                                                                            *
 * Purpose: compare new checksums with old ones. Write difference to          *
 *          table 'node_config'                                               *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value: SUCCESS - calculated succesfully                             * 
 *               FAIL - an error occured                                      *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int compare_checksums()
{
	DB_RESULT	result;
	DB_ROW		row;

	/* Begin work */

	/* Find updated records */
	result = DBselect("select new.nodeid,new.tablename,new.recordid from node_cksum old, node_cksum new where new.tablename=old.tablename and new.recordid=old.recordid and new.fieldname=old.fieldname and new.nodeid=old.nodeid and new.cksum<>old.cksum and new.cksumtype=%d and old.cksumtype=%d", NODE_CKSUM_TYPE_NEW, NODE_CKSUM_TYPE_OLD);
	while((row=DBfetch(result)))
	{
//		zabbix_log( LOG_LEVEL_WARNING, "Adding record to node_configlog");
		DBexecute("insert into node_configlog (nodeid,tablename,recordid,operation)" \
			  "values (%s,'%s',%s,%d)", row[0],row[1],row[2],NODE_CONFIGLOG_OP_UPDATE);
	}
	DBfree_result(result);

	/* Find new records */
	result = DBselect("select new.nodeid,new.tablename,new.recordid from node_cksum new" \
			  " left join node_cksum old" \
			  " on new.tablename=old.tablename and new.recordid=old.recordid and new.fieldname=old.fieldname and new.nodeid=old.nodeid and new.cksumtype<>old.cksumtype" \
			  " where old.cksumid is null and new.cksumtype=%d", NODE_CKSUM_TYPE_NEW);

	while((row=DBfetch(result)))
	{
//		zabbix_log( LOG_LEVEL_WARNING, "Adding record to node_configlog");
		DBexecute("insert into node_configlog (nodeid,tablename,recordid,operation)" \
			  "values (%s,'%s',%s,%d)", row[0],row[1],row[2],NODE_CONFIGLOG_OP_ADD);
	}
	DBfree_result(result);

	/* Find deleted records */
	result = DBselect("select new.nodeid,new.tablename,new.recordid from node_cksum new" \
			  " left join node_cksum old" \
			  " on new.tablename=old.tablename and new.recordid=old.recordid and new.fieldname=old.fieldname and new.nodeid=old.nodeid and new.cksumtype<>old.cksumtype" \
			  " where old.cksumid is null and new.cksumtype=%d", NODE_CKSUM_TYPE_OLD);

	while((row=DBfetch(result)))
	{
//		zabbix_log( LOG_LEVEL_WARNING, "Adding record to node_configlog");
		DBexecute("insert into node_configlog (nodeid,tablename,recordid,operation)" \
			  "values (%s,'%s',%s,%d)", row[0],row[1],row[2],NODE_CONFIGLOG_OP_DELETE);
	}
	DBfree_result(result);

	/* Commit */

	return SUCCEED;
}


/******************************************************************************
 *                                                                            *
 * Function: main_nodewatcher_loop                                            *
 *                                                                            *
 * Purpose: periodically calculates checks sum of config data                 *
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
int main_nodewatcher_loop()
{
//	zabbix_log( LOG_LEVEL_WARNING, "In main_nodeupdater_loop()");
	for(;;)
	{

		zbx_setproctitle("connecting to the database");

		DBconnect();
		calculate_checksums();
		compare_checksums();
		update_checksums();
		DBclose();

		zbx_setproctitle("sender [sleeping for %d seconds]", 30);

		sleep(30);
	}
}
