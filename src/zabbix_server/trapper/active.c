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


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>

#include <string.h>

#include <time.h>

#include <sys/socket.h>
#include <errno.h>

/* Functions: pow(), round() */
#include <math.h>

#include "common.h"
#include "db.h"
#include "log.h"
#include "zlog.h"

#include "active.h"

/******************************************************************************
 *                                                                            *
 * Function: send_list_of_active_checks                                       *
 *                                                                            *
 * Purpose: send list of active checks to the host                            *
 *                                                                            *
 * Parameters: sockfd - open socket of server-agent connection                *
 *             host - hostname                                                *
 *                                                                            *
 * Return value:  SUCCEED - list of active checks sent succesfully            *
 *                FAIL - an error occured                                     *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments: format of the list: key:delay:last_log_size                      *
 *                                                                            *
 ******************************************************************************/
int	send_list_of_active_checks(zbx_sock_t *sock, const char *host)
{
	char	s[MAX_STRING_LEN];
	char	*host_esc;
	DB_RESULT result;
	DB_ROW	row;

	zabbix_log( LOG_LEVEL_DEBUG, "In send_list_of_active_checks()");

	host_esc = DBdyn_escape_string(host);

	if (0 != CONFIG_REFRESH_UNSUPPORTED) {
		result = DBselect("select i.key_,i.delay,i.lastlogsize from items i,hosts h"
			" where i.hostid=h.hostid and h.status=%d and i.type=%d and h.host='%s'"
			" and h.proxy_hostid=0 and (i.status=%d or (i.status=%d and i.nextcheck<=%d))" DB_NODE,
			HOST_STATUS_MONITORED,
			ITEM_TYPE_ZABBIX_ACTIVE,
			host_esc,
			ITEM_STATUS_ACTIVE, ITEM_STATUS_NOTSUPPORTED, time(NULL),
			DBnode_local("h.hostid"));
	} else {
		result = DBselect("select i.key_,i.delay,i.lastlogsize from items i,hosts h"
			" where i.hostid=h.hostid and h.status=%d and i.type=%d and h.host='%s'"
			" and h.proxy_hostid=0 and i.status=%d" DB_NODE,
			HOST_STATUS_MONITORED,
			ITEM_TYPE_ZABBIX_ACTIVE,
			host_esc,
			ITEM_STATUS_ACTIVE,
			DBnode_local("h.hostid"));
	}

	zbx_free(host_esc);

	while((row=DBfetch(result)))
	{

		zbx_snprintf(s,sizeof(s),"%s:%s:%s\n",
			row[0],
			row[1],
			row[2]);
		zabbix_log( LOG_LEVEL_DEBUG, "Sending [%s]",
			s);

		if( zbx_tcp_send_raw(sock,s) != SUCCEED )
		{
			zabbix_log( LOG_LEVEL_WARNING, "Error while sending list of active checks");
			return  FAIL;
		}
	}
	DBfree_result(result);

	zbx_snprintf(s,sizeof(s),"%s\n",
		"ZBX_EOF");
	zabbix_log( LOG_LEVEL_DEBUG, "Sending [%s]",
		s);

	if( zbx_tcp_send_raw(sock,s) != SUCCEED )
	{
		zabbix_log( LOG_LEVEL_WARNING, "Error while sending list of active checks");
		return  FAIL;
	}

	return  SUCCEED;
}

/******************************************************************************
 *                                                                            *
 * Function: send_list_of_active_checks_json                                  *
 *                                                                            *
 * Purpose: send list of active checks to the host                            *
 *                                                                            *
 * Parameters: sockfd - open socket of server-agent connection                *
 *             json - request buffer                                          *
 *                                                                            *
 * Return value:  SUCCEED - list of active checks sent succesfully            *
 *                FAIL - an error occured                                     *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
int	send_list_of_active_checks_json(zbx_sock_t *sock, struct zbx_json_parse *jp)
{
	char		host[MAX_STRING_LEN], tmp[32];
	const char	*pf;
	DB_RESULT	result;
	DB_ROW		row;
	DB_ITEM		item;
	struct zbx_json	json;

	zabbix_log( LOG_LEVEL_DEBUG, "In send_list_of_active_checks_json()");

	if (NULL == (pf = zbx_json_pair_by_name(jp, ZBX_PROTO_TAG_HOST)) ||
			NULL == (pf = zbx_json_decodevalue(pf, host, sizeof(host))))
	{
		zabbix_log( LOG_LEVEL_WARNING, "No tag \"%s\" in JSON request",
			ZBX_PROTO_TAG_HOST);
		return FAIL;
	}

	zabbix_log( LOG_LEVEL_DEBUG, "Host:%s", host);

	if (0 != CONFIG_REFRESH_UNSUPPORTED) {
		result = DBselect("select %s where i.hostid=h.hostid and h.status=%d and i.type=%d and h.host='%s'"
			" and h.proxy_hostid=0 and (i.status=%d or (i.status=%d and i.nextcheck<=%d))" DB_NODE,
			ZBX_SQL_ITEM_SELECT,
			HOST_STATUS_MONITORED,
			ITEM_TYPE_ZABBIX_ACTIVE,
			host,
			ITEM_STATUS_ACTIVE, ITEM_STATUS_NOTSUPPORTED, time(NULL),
			DBnode_local("h.hostid"));
	} else {
		result = DBselect("select %s where i.hostid=h.hostid and h.status=%d and i.type=%d and h.host='%s'"
			" and h.proxy_hostid=0 and i.status=%d" DB_NODE,
			ZBX_SQL_ITEM_SELECT,
			HOST_STATUS_MONITORED,
			ITEM_TYPE_ZABBIX_ACTIVE,
			host,
			ITEM_STATUS_ACTIVE,
			DBnode_local("h.hostid"));
	}

	zbx_json_init(&json, 8*1024);
	zbx_json_addstring(&json, ZBX_PROTO_TAG_RESPONSE, ZBX_PROTO_VALUE_SUCCESS, ZBX_JSON_TYPE_STRING);
	zbx_json_addarray(&json, ZBX_PROTO_TAG_DATA);

	while((row=DBfetch(result)))
	{
		DBget_item_from_db(&item, row);

		zbx_json_addobject(&json, NULL);
		zbx_json_addstring(&json, ZBX_PROTO_TAG_KEY, item.key, ZBX_JSON_TYPE_STRING);
		if (0 != strcmp(item.key, item.key_orig))
			zbx_json_addstring(&json, ZBX_PROTO_TAG_KEY_ORIG, item.key_orig, ZBX_JSON_TYPE_STRING);
		zbx_snprintf(tmp, sizeof(tmp), "%d", item.delay);
		zbx_json_addstring(&json, ZBX_PROTO_TAG_DELAY, tmp, ZBX_JSON_TYPE_STRING);
		zbx_snprintf(tmp, sizeof(tmp), "%d", item.lastlogsize);
		zbx_json_addstring(&json, ZBX_PROTO_TAG_LOGLASTSIZE, tmp, ZBX_JSON_TYPE_STRING);
		zbx_json_close(&json);
	}
	DBfree_result(result);

	zabbix_log( LOG_LEVEL_DEBUG, "Sending [%s]",
		json.buffer);

	if( zbx_tcp_send_raw(sock, json.buffer) != SUCCEED )
	{
		zabbix_log( LOG_LEVEL_WARNING, "Error while sending list of active checks");
		return  FAIL;
	}

	zbx_json_free(&json);

	return  SUCCEED;
}
