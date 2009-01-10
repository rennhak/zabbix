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
#include "dbsync.h"

#include "nodesender.h"
#include "nodewatcher.h"
#include "nodecomms.h"
#include "../trapper/nodesync.h"


static const ZBX_TABLE *DBget_table(const char *tablename)
{
	int	t;

	for (t = 0; tables[t].table != 0; t++ )
		if (0 == strcmp(tables[t].table, tablename))
			return &tables[t];
	return NULL;
}

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
int calculate_checksums(int nodeid, const char *tablename, const zbx_uint64_t id)
{
	char		*sql = NULL;
	int		sql_allocated = 16*1024, sql_offset = 0;
	int		t, f, res = SUCCEED;

	zabbix_log(LOG_LEVEL_DEBUG, "In calculate_checksums");

	sql = zbx_malloc(sql, sql_allocated);

	for (t = 0; tables[t].table != 0; t++) {
		/* Do not sync some of tables */
		if ((tables[t].flags & ZBX_SYNC) == 0)
			continue;

		if (NULL != tablename && 0 != strcmp(tablename, tables[t].table))
			continue;

#ifdef	HAVE_MYSQL
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
			"%s select %d,'%s',%s,%d,concat_ws(',',",
			sql_offset > 0 ? "union all" : "insert into node_cksum (nodeid,tablename,recordid,cksumtype,cksum)",
			nodeid,
			tables[t].table,
			tables[t].recid,
			NODE_CKSUM_TYPE_NEW);
#else
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
			"%s select %d,'%s',%s,%d,",
			sql_offset > 0 ? "union all" : "insert into node_cksum (nodeid,tablename,recordid,cksumtype,cksum)",
			nodeid,
			tables[t].table,
			tables[t].recid,
			NODE_CKSUM_TYPE_NEW);
#endif

		for (f = 0; tables[t].fields[f].name != 0; f ++) {
			if ((tables[t].fields[f].flags & ZBX_SYNC) == 0)
				continue;

			if (tables[t].fields[f].flags & ZBX_NOTNULL) {
				switch ( tables[t].fields[f].type ) {
				case ZBX_TYPE_ID	:
				case ZBX_TYPE_INT	:
				case ZBX_TYPE_UINT	:
					zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 128,
						"%s",
						tables[t].fields[f].name);
					break;
				default	:
					zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 128,
						"md5(%s)",
						tables[t].fields[f].name);
					break;
				}
			} else {
				switch ( tables[t].fields[f].type ) {
				case ZBX_TYPE_ID	:
				case ZBX_TYPE_INT	:
				case ZBX_TYPE_UINT	:
					zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 128,
						"case when %s is null then 'NULL' else %s end",
						tables[t].fields[f].name,
						tables[t].fields[f].name);
					break;
				default	:
					zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 128,
						"case when %s is null then 'NULL' else md5(%s) end",
						tables[t].fields[f].name,
						tables[t].fields[f].name);
					break;
				}
			}
#ifdef	HAVE_MYSQL
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 16,
					",");
#else
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 16,
					"||','||");
#endif
		}

		/* remove last delimiter */
		if (f > 0) {
#ifdef	HAVE_MYSQL
			sql_offset --;
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 16, ")");
#else
			sql_offset -= 7;
#endif
		}

		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 512,
			" from %s where"ZBX_COND_NODEID,
			tables[t].table,
			ZBX_NODE(tables[t].recid,nodeid));

		if (0 != id) {
			zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 128,
				" and %s="ZBX_FS_UI64,
				tables[t].recid,
				id);
		}
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 128, "\n");
	}
	if (SUCCEED == res && DBexecute("delete from node_cksum where nodeid=%d and cksumtype=%d",
		nodeid,
		NODE_CKSUM_TYPE_NEW) < ZBX_DB_OK)
		res = FAIL;
	if (SUCCEED == res && DBexecute("%s", sql) < ZBX_DB_OK)
		res = FAIL;

	zbx_free(sql);

	return res;
}

/******************************************************************************
 *                                                                            *
 * Function: send_config_data                                                 *
 *                                                                            *
 * Purpose: send configuration changes to required node                       *
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
char *get_config_data(int nodeid, int dest_nodetype)
{
	DB_RESULT	result;
	DB_RESULT	result2;
	DB_ROW		row;
	DB_ROW		row2;
	const ZBX_TABLE	*table;

	char	*data = NULL, *hex = NULL, *sql = NULL, c[2], sync[129], *s, *r[2], *d[2];
	int	data_offset=0, sql_offset = 0;
	int	data_allocated=1024, hex_allocated=1024, sql_allocated=8*1024;
	int	f, j, rowlen;
	
	zabbix_log( LOG_LEVEL_DEBUG, "In get_config_data(node:%d,dest_nodetype:%s)",
		nodeid,
		dest_nodetype == ZBX_NODE_MASTER ? "MASTER" : "SLAVE");

	data = zbx_malloc(data, data_allocated);
	hex = zbx_malloc(hex, hex_allocated);
	sql = zbx_malloc(sql, sql_allocated);
	c[0] = '1';	/* for new and updated records */
	c[1] = '2';	/* for deleted records */

	zbx_snprintf_alloc(&data, &data_allocated, &data_offset, 128, "Data%c%d%c%d\n",
		ZBX_DM_DELIMITER,
		CONFIG_NODEID,
		ZBX_DM_DELIMITER,
		nodeid);

	/* Find updated records */
	result = DBselect("select curr.tablename,curr.recordid,prev.cksum,curr.cksum,prev.sync "
		"from node_cksum curr, node_cksum prev "
		"where curr.nodeid=%d and prev.nodeid=curr.nodeid and "
		"curr.tablename=prev.tablename and curr.recordid=prev.recordid and "
		"curr.cksumtype=%d and prev.cksumtype=%d "
		"union all "
	/* Find new records */
		"select curr.tablename,curr.recordid,prev.cksum,curr.cksum,curr.sync "
		"from node_cksum curr left join node_cksum prev "
		"on prev.nodeid=curr.nodeid and prev.tablename=curr.tablename and "
		"prev.recordid=curr.recordid and prev.cksumtype=%d "
		"where curr.nodeid=%d and curr.cksumtype=%d and prev.tablename is null "
		"union all "
	/* Find deleted records */
		"select prev.tablename,prev.recordid,prev.cksum,curr.cksum,prev.sync "
		"from node_cksum prev left join node_cksum curr "
		"on curr.nodeid=prev.nodeid and curr.tablename=prev.tablename and "
		"curr.recordid=prev.recordid and curr.cksumtype=%d "
		"where prev.nodeid=%d and prev.cksumtype=%d and curr.tablename is null",
		nodeid, NODE_CKSUM_TYPE_NEW, NODE_CKSUM_TYPE_OLD,
		NODE_CKSUM_TYPE_OLD, nodeid, NODE_CKSUM_TYPE_NEW,
		NODE_CKSUM_TYPE_NEW, nodeid, NODE_CKSUM_TYPE_OLD);

	while (NULL != (row = DBfetch(result)))
	{
		/* Found table */
		if (NULL == (table = DBget_table(row[0])))
		{
			zabbix_log(LOG_LEVEL_WARNING, "Cannot find table [%s]",
					row[0]);
			continue;
		}

		if (DBis_null(row[4]) == FAIL)
			zbx_strlcpy(sync, row[4], sizeof(sync));
		else
			memset(sync, ' ', sizeof(sync));

		s = sync;

		/* Special (simpler) processing for operation DELETE */
		if (DBis_null(row[3]) == SUCCEED)
		{
			if((dest_nodetype == ZBX_NODE_SLAVE && *s != c[1]) || (dest_nodetype == ZBX_NODE_MASTER && *(s + 1) != c[1]))
			{
				zbx_snprintf_alloc(&data, &data_allocated, &data_offset, 128, "%s%c%s%c%d\n",
					row[0],
					ZBX_DM_DELIMITER,
					row[1],
					ZBX_DM_DELIMITER,
					NODE_CONFIGLOG_OP_DELETE);
			}
			continue;
		}

		r[0] = DBis_null(row[2]) == SUCCEED ? NULL : row[2];
		r[1] = DBis_null(row[3]) == SUCCEED ? NULL : row[3];
		f = 0;
		sql_offset = 0;

		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 128, "select ");
		do {
			while ((table->fields[f].flags & ZBX_SYNC) == 0)
				f++;

			d[0] = NULL;
			d[1] = NULL;
			if (NULL != r[0] && NULL != (d[0] = strchr(r[0], ',')))
				*d[0] = '\0';
			if (NULL != r[1] && NULL != (d[1] = strchr(r[1], ',')))
				*d[1] = '\0';

			if (r[0] == NULL || r[1] == NULL || (dest_nodetype == ZBX_NODE_SLAVE && *s != c[0]) ||
				(dest_nodetype == ZBX_NODE_MASTER && *(s+1) != c[0]) || strcmp(r[0], r[1]) != 0) {
				zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 128, "%s,length(%s),", 
					table->fields[f].name,
					table->fields[f].name);
			}
			s += 2;
			f++;

			if (d[0] != NULL) {
				*d[0] = ',';
				r[0] = d[0] + 1;
			} else
				r[0] = NULL; 
			if (d[1] != NULL) {
				*d[1] = ',';
				r[1] = d[1] + 1;
			} else
				r[1] = NULL; 
		} while (d[0] != NULL || d[1] != NULL);

		if (sql[sql_offset-1] != ',')
			continue;

		sql_offset--;
		zbx_snprintf_alloc(&sql, &sql_allocated, &sql_offset, 128, " from %s where %s=%s",
			row[0],
			table->recid,
			row[1]);

		result2 = DBselect("%s", sql);
		if (NULL == (row2=DBfetch(result2)))
			goto out;

		zbx_snprintf_alloc(&data, &data_allocated, &data_offset, 128, "%s%c%s%c%d",
			row[0],
			ZBX_DM_DELIMITER,
			row[1],
			ZBX_DM_DELIMITER,
			NODE_CONFIGLOG_OP_UPDATE);

		r[0] = DBis_null(row[2]) == SUCCEED ? NULL : row[2];
		r[1] = DBis_null(row[3]) == SUCCEED ? NULL : row[3];
		s = sync;
		f = 0;
		j = 0;

		do {
			while ((table->fields[f].flags & ZBX_SYNC) == 0)
				f++;

			d[0] = NULL;
			d[1] = NULL;
			if (NULL != r[0] && NULL != (d[0] = strchr(r[0], ',')))
				*d[0] = '\0';
			if (NULL != r[1] && NULL != (d[1] = strchr(r[1], ',')))
				*d[1] = '\0';

			if (r[0] == NULL || r[1] == NULL || (dest_nodetype == ZBX_NODE_SLAVE && *s != c[0]) ||
				(dest_nodetype == ZBX_NODE_MASTER && *(s+1) != c[0]) || strcmp(r[0], r[1]) != 0) {

				zbx_snprintf_alloc(&data, &data_allocated, &data_offset, 128, "%c%s%c%d%c",
					ZBX_DM_DELIMITER,
					table->fields[f].name,
					ZBX_DM_DELIMITER,
					table->fields[f].type,
					ZBX_DM_DELIMITER);

				/* Fieldname, type, value */
				if (DBis_null(row2[j*2]) == SUCCEED) {
					zbx_snprintf_alloc(&data, &data_allocated, &data_offset, 128, "NULL");
				} else if(table->fields[f].type == ZBX_TYPE_INT ||
					table->fields[f].type == ZBX_TYPE_UINT ||
					table->fields[f].type == ZBX_TYPE_ID ||
					table->fields[f].type == ZBX_TYPE_FLOAT) {

					zbx_snprintf_alloc(&data, &data_allocated, &data_offset, 128, "%s", row2[j*2]);
				} else {
					rowlen = atoi(row2[j*2+1]);
					zbx_binary2hex((u_char *)row2[j*2], rowlen, &hex, &hex_allocated);
					zbx_snprintf_alloc(&data, &data_allocated, &data_offset, strlen(hex)+128, "%s", hex);
/*zabbix_log(LOG_LEVEL_CRIT, "----- [field:%s][type:%d][row:%s][hex:%s]",table->fields[f].name,table->fields[f].type,row2[j*2],hex);*/
				}
				j++;
			}
			s += 2;
			f++;

			if (d[0] != NULL) {
				*d[0] = ',';
				r[0] = d[0] + 1;
			} else
				r[0] = NULL; 
			if (d[1] != NULL) {
				*d[1] = ',';
				r[1] = d[1] + 1;
			} else
				r[1] = NULL; 
		} while (d[0] != NULL || d[1] != NULL);
		zbx_snprintf_alloc(&data, &data_allocated, &data_offset, 128, "\n");
out:
		DBfree_result(result2);
	}
	DBfree_result(result);

	zbx_free(hex);
	zbx_free(sql);

	return data;
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
int update_checksums(int nodeid, int synked_nodetype, int synked, const char *tablename, const zbx_uint64_t id, char *fields)
{
	char		*r[2], *d[2], sync[129], *s;
	char		c[2], sql[2][256];
	char		cksum[32*64+32], *ck;
	DB_RESULT	result;
	DB_ROW		row;
	int		f;
	const ZBX_TABLE	*table;

	zabbix_log(LOG_LEVEL_DEBUG, "In update_checksums");

	c[0] = synked == SUCCEED ? '1' : ' ';	/* for new and updated records */
	c[1] = synked == SUCCEED ? '2' : ' ';	/* for deleted records */

	if (NULL != tablename) {
		zbx_snprintf(sql[0], sizeof(sql[0]), " and curr.tablename='%s' and curr.recordid=" ZBX_FS_UI64,
			tablename, id);
		zbx_snprintf(sql[1], sizeof(sql[1]), " and prev.tablename='%s' and prev.recordid=" ZBX_FS_UI64,
			tablename, id);
	} else {
		*sql[0] = '\0';
		*sql[1] = '\0';
	}

	/* Find updated records */
	result = DBselect("select curr.tablename,curr.recordid,prev.cksum,curr.cksum,prev.sync "
		"from node_cksum curr, node_cksum prev "
		"where curr.nodeid=%d and prev.nodeid=curr.nodeid and "
		"curr.tablename=prev.tablename and curr.recordid=prev.recordid and "
		"curr.cksumtype=%d and prev.cksumtype=%d%s "
		"union all "
	/* Find new records */
		"select curr.tablename,curr.recordid,prev.cksum,curr.cksum,NULL "
		"from node_cksum curr left join node_cksum prev "
		"on prev.nodeid=curr.nodeid and prev.tablename=curr.tablename and "
		"prev.recordid=curr.recordid and prev.cksumtype=%d "
		"where curr.nodeid=%d and curr.cksumtype=%d and prev.tablename is null%s "
		"union all "
	/* Find deleted records */
		"select prev.tablename,prev.recordid,prev.cksum,curr.cksum,prev.sync "
		"from node_cksum prev left join node_cksum curr "
		"on curr.nodeid=prev.nodeid and curr.tablename=prev.tablename and "
		"curr.recordid=prev.recordid and curr.cksumtype=%d "
		"where prev.nodeid=%d and prev.cksumtype=%d and curr.tablename is null%s",
		nodeid, NODE_CKSUM_TYPE_NEW, NODE_CKSUM_TYPE_OLD, sql[0],
		NODE_CKSUM_TYPE_OLD, nodeid, NODE_CKSUM_TYPE_NEW, sql[0],
		NODE_CKSUM_TYPE_NEW, nodeid, NODE_CKSUM_TYPE_OLD, sql[1]);

	while (NULL != (row = DBfetch(result)))
	{
		/* Found table */
		if (NULL == (table = DBget_table(row[0])))
		{
			zabbix_log(LOG_LEVEL_WARNING, "Cannot find table [%s]",
					row[0]);
			continue;
		}

		if (DBis_null(row[4]) == FAIL)
			zbx_strlcpy(sync, row[4], sizeof(sync));
		else
			memset(sync, ' ', sizeof(sync));

		s = sync;
		ck = cksum;
		*ck = '\0';

		/* Special (simpler) processing for operation DELETE */
		if (DBis_null(row[3]) == SUCCEED) {
/*			if (*(s+2) != '\0') {
				*s = ' ';
				*(s + 1) = ' ';
			}*/
			if (synked == SUCCEED) {
				if (synked_nodetype == ZBX_NODE_SLAVE)
					*s = c[1];
				else if (synked_nodetype == ZBX_NODE_MASTER) 
					*(s + 1) = c[1];
			}
			s += 2;
		} else {
			r[0] = DBis_null(row[2]) == SUCCEED ? NULL : row[2];
			r[1] = DBis_null(row[3]) == SUCCEED ? NULL : row[3];
			f = 0;

			do {
				while ((table->fields[f].flags & ZBX_SYNC) == 0)
					f++;

				d[0] = NULL;
				d[1] = NULL;
				if (NULL != r[0] && NULL != (d[0] = strchr(r[0], ',')))
					*d[0] = '\0';
				if (NULL != r[1] && NULL != (d[1] = strchr(r[1], ',')))
					*d[1] = '\0';

				if (NULL == tablename || SUCCEED == str_in_list(fields, table->fields[f].name, ',')) {
					ck += zbx_snprintf(ck, 64, "%s,", NULL != r[1] ? r[1] : r[0]);

					if (r[0] == NULL || r[1] == NULL || strcmp(r[0], r[1]) != 0) {
						if (synked_nodetype == ZBX_NODE_SLAVE) {
							*s = c[0];
							*(s + 1) = ' ';
						} else if (synked_nodetype == ZBX_NODE_MASTER) {
							*s = ' ';
							*(s + 1) = c[0];
						}
					} else {
						if (synked == SUCCEED) {
							if (synked_nodetype == ZBX_NODE_SLAVE)
								*s = c[0];
							else if (synked_nodetype == ZBX_NODE_MASTER) 
								*(s + 1) = c[0];
						}
					}
				} else
					ck += zbx_snprintf(ck, 64, "%s,", NULL != r[0] ? r[0] : "");
				s += 2;
				f++;

				if (d[0] != NULL) {
					*d[0] = ',';
					r[0] = d[0] + 1;
				} else
					r[0] = NULL; 
				if (d[1] != NULL) {
					*d[1] = ',';
					r[1] = d[1] + 1;
				} else
					r[1] = NULL; 
			} while (d[0] != NULL || d[1] != NULL);
		}
		*s = '\0';
		*--ck = '\0';

		if (DBis_null(row[2]) == SUCCEED || DBis_null(row[3]) == SUCCEED ||
			strcmp(row[4], sync) != 0 || strcmp(row[2], row[3]) != 0)
		{
			DBexecute("update node_cksum set cksumtype=%d,cksum='%s',sync='%s' "
				"where nodeid=%d and tablename='%s' and recordid=%s and cksumtype=%d",
				NODE_CKSUM_TYPE_OLD,
				cksum,
				sync,
				nodeid,
				row[0],
				row[1],
				DBis_null(row[2]) == SUCCEED ? NODE_CKSUM_TYPE_NEW : NODE_CKSUM_TYPE_OLD);
		}
	}
	DBfree_result(result);

	return SUCCEED;
}

/******************************************************************************
 *                                                                            *
 * Function: node_sync_lock                                                   *
 *                                                                            *
 * Purpose:                                                                   *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              * 
 *                                                                            *
 * Author: Aleksander Vladishev                                               *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
void node_sync_lock(int nodeid)
{
	zbx_mutex_lock(&node_sync_access);
}

/******************************************************************************
 *                                                                            *
 * Function: node_sync_unlock                                                 *
 *                                                                            *
 * Purpose:                                                                   *
 *                                                                            *
 * Parameters:                                                                *
 *                                                                            *
 * Return value:                                                              * 
 *                                                                            *
 * Author: Aleksander Vladishev                                               *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
void node_sync_unlock(int nodeid)
{
	zbx_mutex_unlock(&node_sync_access);
}

/******************************************************************************
 *                                                                            *
 * Function: process_nodes                                                    *
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
void process_nodes()
{
	DB_RESULT	result;
	DB_ROW		row;
	int		nodeid;
	int		master_nodeid;
	char		*data, *answer;
	zbx_sock_t	sock;
	int		res;
/*	int		now = time(NULL);*/
	int		sender_nodeid;

	master_nodeid = CONFIG_MASTER_NODEID;
	if (0 == master_nodeid)
		return;

	result = DBselect("select nodeid from nodes");
	while (NULL != (row=DBfetch(result))) {
		nodeid = atoi(row[0]);
		if (SUCCEED == is_master_node(CONFIG_NODEID, nodeid))
			continue;

		node_sync_lock(nodeid);

/*		DBbegin();*/

		res = calculate_checksums(nodeid, NULL, 0);
		if (SUCCEED == res && NULL != (data = get_config_data(nodeid, ZBX_NODE_MASTER))) {
			zabbix_log( LOG_LEVEL_WARNING, "NODE %d: Sending configuration changes to master node %d for node %d datalen %d",
				CONFIG_NODEID,
				master_nodeid,
				nodeid,
				strlen(data));
			if (SUCCEED == (res = connect_to_node(master_nodeid, &sock))) {
				if (SUCCEED == res)
					res = send_data_to_node(master_nodeid, &sock, data);
				if (SUCCEED == res)
					res = recv_data_from_node(master_nodeid, &sock, &answer);
				if (SUCCEED == res && 0 == strncmp(answer, "Data", 4)) {
					res = update_checksums(nodeid, ZBX_NODE_MASTER, SUCCEED, NULL, 0, NULL);
					if (SUCCEED == res)
						res = node_sync(answer, &sender_nodeid, &nodeid);
					send_data_to_node(master_nodeid, &sock, SUCCEED == res ? "OK" : "FAIL");
				}
				disconnect_node(&sock);
			}
			zbx_free(data);
		}

/*		DBcommit();*/

		node_sync_unlock(nodeid);
	}
	DBfree_result(result);

/*	zabbix_log(LOG_LEVEL_CRIT, "<-----> process_nodes [Selected records in %d seconds]", time(NULL)-now);*/
}
