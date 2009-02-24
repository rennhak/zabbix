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

/******************************************************************************
 *                                                                            *
 * Function: convert_expression                                               *
 *                                                                            *
 * Purpose: convert trigger expression to new node ID                         *
 *                                                                            *
 * Parameters: old_id - old id, new_id - new node id                          *
 *             old_exp - old expression, new_exp - new expression             *
 *                                                                            *
 * Return value: SUCCESS - converted succesfully                              *
 *               FAIL - an error occured                                      *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int convert_expression(int old_id, int new_id, zbx_uint64_t prefix, const char *old_exp, char *new_exp)
{
	int		i;
	char		id[MAX_STRING_LEN];
	enum		state_t {NORMAL, ID} state = NORMAL;
	char		*p, *p_id = NULL;
	zbx_uint64_t	tmp;

	p = new_exp;

	for (i = 0; old_exp[i] != 0; i++)
	{
		if (state == ID)
		{
			if (old_exp[i] == '}')
			{
				state = NORMAL;
				ZBX_STR2UINT64(tmp, id);
				tmp += prefix;
				p += zbx_snprintf(p, MAX_STRING_LEN, ZBX_FS_UI64, tmp);
				*p++ = old_exp[i];
			}
			else
			{
				p_id[0] = old_exp[i];
				p_id++;
			}
		}
		else if (old_exp[i] == '{')
		{
			state = ID;
			memset(id,0,MAX_STRING_LEN);
			p_id = id;
			*p++ = old_exp[i];
		}
		else
			*p++ = old_exp[i];
	}

	return SUCCEED;
}

/******************************************************************************
 *                                                                            *
 * Function: convert_triggers_expression                                      *
 *                                                                            *
 * Purpose: convert trigger expressions to new node ID                        *
 *                                                                            *
 * Parameters: old_id - old id, new_id - new node id                          *
 *                                                                            *
 * Return value: SUCCESS - converted succesfully                              *
 *               FAIL - an error occured                                      *
 *                                                                            *
 * Author: Alexander Vladishev                                                *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int convert_triggers_expression(int old_id, int new_id)
{
	zbx_uint64_t	prefix;
	const ZBX_TABLE	*r_table;
	DB_RESULT	result;
	DB_ROW		row;
	char		new_expression[MAX_STRING_LEN];
	char		*new_expression_esc;

	if (NULL == (r_table = DBget_table("functions")))
	{
		printf("triggers.expression FAILED\n");
		return FAIL;
	}

	prefix = (zbx_uint64_t)__UINT64_C(100000000000000)*(zbx_uint64_t)new_id;
	if (r_table->flags & ZBX_SYNC)
		prefix += (zbx_uint64_t)__UINT64_C(100000000000)*(zbx_uint64_t)new_id;

	result = DBselect("select expression,triggerid from triggers");

	while (NULL != (row = DBfetch(result)))
	{
		memset(new_expression, 0, sizeof(new_expression));
		convert_expression(old_id, new_id, prefix, row[0], new_expression);

		new_expression_esc = DBdyn_escape_string_len(new_expression, TRIGGER_EXPRESSION_LEN);
		DBexecute("update triggers set expression='%s' where triggerid=%s",
				new_expression_esc,
				row[1]);
		zbx_free(new_expression_esc);
	}
	DBfree_result(result);

	return SUCCEED;
}

/******************************************************************************
 *                                                                            *
 * Function: convert_profiles                                                 *
 *                                                                            *
 * Purpose: convert profiles idx2 and value_id fields to new node ID          *
 *                                                                            *
 * Parameters: old_id - old id, new_id - new node id                          *
 *                                                                            *
 * Return value: SUCCESS - converted succesfully                              *
 *               FAIL - an error occured                                      *
 *                                                                            *
 * Author: Alexander Vladishev                                                *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int convert_profiles(int old_id, int new_id, const char *field_name)
{
	zbx_uint64_t	prefix;

	prefix = (zbx_uint64_t)__UINT64_C(100000000000000)*(zbx_uint64_t)new_id + (zbx_uint64_t)__UINT64_C(100000000000)*(zbx_uint64_t)new_id;

	DBexecute("update profiles set %s=%s+" ZBX_FS_UI64 " where %s>0",
			field_name,
			field_name,
			prefix,
			field_name);

	return SUCCEED;
}

/******************************************************************************
 *                                                                            *
 * Function: convert_special_field                                            *
 *                                                                            *
 * Purpose: special processing for multipurposes fields                       *
 *                                                                            *
 * Parameters: old_id - old id, new_id - new node id                          *
 *                                                                            *
 * Return value: SUCCESS - converted succesfully                              *
 *               FAIL - an error occured                                      *
 *                                                                            *
 * Author: Aleksander Vladishev                                               *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
static int convert_special_field(int old_id, int new_id, const char *table_name, const char *field_name, const char *type_field_name,
		const char *rel_table_name, int type)
{
	zbx_uint64_t	prefix;
	const ZBX_TABLE	*r_table;

	if (NULL == (r_table = DBget_table(rel_table_name)))
	{
		printf("%s.%s FAILED\n", table_name, field_name);
		return FAIL;
	}

	prefix = (zbx_uint64_t)__UINT64_C(100000000000000)*(zbx_uint64_t)new_id;
	if (r_table->flags & ZBX_SYNC)
		prefix += (zbx_uint64_t)__UINT64_C(100000000000)*(zbx_uint64_t)new_id;

	DBexecute("update %s set %s=%s+" ZBX_FS_UI64 " where %s=%d and %s>0",
			table_name,
			field_name,
			field_name,
			prefix,
			type_field_name,
			type,
			field_name);

	return SUCCEED;
}

/******************************************************************************
 *                                                                            *
 * Function: change_nodeid                                                    *
 *                                                                            *
 * Purpose: convert database data to new node ID                              *
 *                                                                            *
 * Parameters: old_id - old id, new_id - new node id                          *
 *                                                                            *
 * Return value: SUCCESS - converted succesfully                              * 
 *               FAIL - an error occured                                      *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
int change_nodeid(int old_id, int new_id)
{
	struct conv_t
	{
	        char	*rel;
        	int	type;
	};

	struct special_conv_t
	{
	        char		*table_name, *field_name, *type_field_name;
		struct conv_t	convs[10];
	};

	struct special_conv_t special_convs[]=
	{
		{"sysmaps_elements",	"elementid",	"elementtype",
			{
			{"hosts",	SYSMAP_ELEMENT_TYPE_HOST},
			{"sysmaps",	SYSMAP_ELEMENT_TYPE_MAP},
			{"triggers",	SYSMAP_ELEMENT_TYPE_TRIGGER},
			{"groups",	SYSMAP_ELEMENT_TYPE_HOST_GROUP},
			{"images",	SYSMAP_ELEMENT_TYPE_IMAGE},
			{NULL}
			}
		},
		{"events",		"objectid",	"object",
			{
			{"triggers",	EVENT_OBJECT_TRIGGER},
			{"dhosts",	EVENT_OBJECT_DHOST},
			{"dservices",	EVENT_OBJECT_DSERVICE},
			{NULL}
			}
		},
		{"operations",		"objectid",	"object",
			{
			{"users",	OPERATION_OBJECT_USER},
			{"usrgrp",	OPERATION_OBJECT_GROUP},
			{NULL}
			}
		},
		{"ids",			"nextid",	NULL,
			{
			{NULL}
			}
		},
		{"node_cksum",		"recordid",	NULL,
			{
			{NULL}
			}
		},
		{"screens_items",	"resourceid",	"resourcetype",
			{
			{"graphs",	SCREEN_RESOURCE_GRAPH},
			{"items",	SCREEN_RESOURCE_SIMPLE_GRAPH},
			{"sysmaps",	SCREEN_RESOURCE_MAP},
			{"items",	SCREEN_RESOURCE_PLAIN_TEXT},
			{"groups",	SCREEN_RESOURCE_HOSTS_INFO},
/*			{"",	SCREEN_RESOURCE_TRIGGERS_INFO},
			{"",	SCREEN_RESOURCE_SERVER_INFO},
			{"",	SCREEN_RESOURCE_CLOCK},*/
			{"screens",	SCREEN_RESOURCE_SCREEN},
			{"groups",	SCREEN_RESOURCE_TRIGGERS_OVERVIEW},
			{"groups",	SCREEN_RESOURCE_DATA_OVERVIEW},
/*			{"",	SCREEN_RESOURCE_URL},
			{"",	SCREEN_RESOURCE_ACTIONS},
			{"",	SCREEN_RESOURCE_EVENTS},*/
			{NULL}
			}
		},
		{NULL}
	};

	int		i, j, s, t;
	zbx_uint64_t	prefix;
	const ZBX_TABLE	*r_table;

	if(old_id!=0)
	{
		printf("Conversion from non-zero node id is not supported.\n");
		return FAIL;
	}

	if(new_id>999 || new_id<0)
	{
		printf("Node ID must be in range of 0-999.\n");
		return FAIL;
	}

	zabbix_set_log_level(LOG_LEVEL_WARNING);

	DBconnect(ZBX_DB_CONNECT_EXIT);

	DBbegin();

	printf("Converting tables ");
	fflush(stdout);

	for (i = 0; NULL != tables[i].table; i++)
	{
		printf(".");
		fflush(stdout);

		for (j = 0; NULL != tables[i].fields[j].name; j++)
		{
			for (s = 0; NULL != special_convs[s].table_name; s++)
				if (0 == strcmp(special_convs[s].table_name, tables[i].table) &&
						0 == strcmp(special_convs[s].field_name, tables[i].fields[j].name))
					break;

			if (NULL != special_convs[s].table_name)
			{
				for (t = 0; NULL != special_convs[s].convs[t].rel; t++)
				{
					convert_special_field(old_id, new_id, special_convs[s].table_name, special_convs[s].field_name,
							special_convs[s].type_field_name, special_convs[s].convs[t].rel,
							special_convs[s].convs[t].type);
				}
				continue;
			}

			if (tables[i].fields[j].type == ZBX_TYPE_ID)
			{
				if (0 == strcmp(tables[i].fields[j].name, tables[i].recid))	/* primaky key */
				{
					prefix = (zbx_uint64_t)__UINT64_C(100000000000000)*(zbx_uint64_t)new_id;

					if (tables[i].flags & ZBX_SYNC)
						prefix += (zbx_uint64_t)__UINT64_C(100000000000)*(zbx_uint64_t)new_id;
				}
				else if (NULL != tables[i].fields[j].rel)	/* relations */
				{
					if (NULL == (r_table = DBget_table(tables[i].fields[j].rel)))
					{
						printf("%s.%s FAILED\n", tables[i].table, tables[i].fields[j].name);
						fflush(stdout);
						continue;
					}

					prefix = (zbx_uint64_t)__UINT64_C(100000000000000)*(zbx_uint64_t)new_id;

					if (r_table->flags & ZBX_SYNC)
						prefix += (zbx_uint64_t)__UINT64_C(100000000000)*(zbx_uint64_t)new_id;
				}
				else if (0 == strcmp("profiles", tables[i].table))	/* special processing for table 'profiles' */
				{
					convert_profiles(old_id, new_id, tables[i].fields[j].name);
					continue;
				}
				else
				{
					printf("%s.%s FAILED\n", tables[i].table, tables[i].fields[j].name);
					fflush(stdout);
					continue;
				}

				DBexecute("update %s set %s=%s+" ZBX_FS_UI64 " where %s>0",
						tables[i].table,
						tables[i].fields[j].name,
						tables[i].fields[j].name,
						prefix,
						tables[i].fields[j].name);
			}
		}
	}

	/* Special processing for trigger expressions */
	convert_triggers_expression(old_id, new_id);

	DBexecute("insert into nodes (nodeid,name,ip,nodetype) values (%d,'Local node','127.0.0.1',1)",
			new_id);

	DBcommit();
	
	DBclose();
	printf(" done.\nConversion completed.\n");

	return SUCCEED;
}
