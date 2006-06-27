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

#include "checks_snmp.h"

#ifdef HAVE_SNMP
/*int	get_value_snmp(double *result,char *result_str,DB_ITEM *item,char *error, int max_error_len)*/
int	get_value_snmp(DB_ITEM *item, AGENT_RESULT *value)
{

	#define NEW_APPROACH

	struct snmp_session session, *ss;
	struct snmp_pdu *pdu;
	struct snmp_pdu *response;

#ifdef NEW_APPROACH
	char temp[MAX_STRING_LEN];
#endif

	oid anOID[MAX_OID_LEN];
	size_t anOID_len = MAX_OID_LEN;

	struct variable_list *vars;
	int status;

	unsigned char *ip;
	char 	*p;
	double dbl;

	char error[MAX_STRING_LEN];

	int ret=SUCCEED;

	zabbix_log( LOG_LEVEL_DEBUG, "In get_value_SNMP()");

	init_result(value);

/*	assert((item->type == ITEM_TYPE_SNMPv1)||(item->type == ITEM_TYPE_SNMPv2c)); */
	assert((item->type == ITEM_TYPE_SNMPv1)||(item->type == ITEM_TYPE_SNMPv2c)||(item->type == ITEM_TYPE_SNMPv3));

	snmp_sess_init( &session );
/*	session.version = version;*/
	if(item->type == ITEM_TYPE_SNMPv1)
	{
		session.version = SNMP_VERSION_1;
	}
	else if(item->type == ITEM_TYPE_SNMPv2c)
	{
		session.version = SNMP_VERSION_2c;
	}
	else if(item->type == ITEM_TYPE_SNMPv3)
	{
		session.version = SNMP_VERSION_3;
	}
	else
	{
		zbx_snprintf(error,MAX_STRING_LEN,"Error in get_value_SNMP. Wrong item type [%d]. Must be SNMP.", item->type);

		zabbix_log( LOG_LEVEL_ERR, error);
		SET_MSG_RESULT(value, strdup(error));

		return FAIL;
	}


	if(item->useip == 1)
	{
	#ifdef NEW_APPROACH
		zbx_snprintf(temp,sizeof(temp),"%s:%d", item->ip, item->snmp_port);
		session.peername = temp;
	#else
		session.peername = item->ip;
	#endif
	}
	else
	{
	#ifdef NEW_APPROACH
		zbx_snprintf(temp, sizeof(temp), "%s:%d", item->host, item->snmp_port);
		session.peername = temp;
	#else
		session.peername = item->host;
	#endif
	}

	if( (session.version == SNMP_VERSION_1) || (item->type == ITEM_TYPE_SNMPv2c))
	{
		session.community = item->snmp_community;
		session.community_len = strlen(session.community);
		zabbix_log( LOG_LEVEL_DEBUG, "SNMP [%s@%s:%d]",session.community, session.peername, session.remote_port);
	}
	else if(session.version == SNMP_VERSION_3)
	{
		/* set the SNMPv3 user name */
		session.securityName = item->snmpv3_securityname;
		session.securityNameLen = strlen(session.securityName);

		/* set the security level to authenticated, but not encrypted */

		if(item->snmpv3_securitylevel == ITEM_SNMPV3_SECURITYLEVEL_NOAUTHNOPRIV)
		{
			session.securityLevel = SNMP_SEC_LEVEL_NOAUTH;
		}
		else if(item->snmpv3_securitylevel == ITEM_SNMPV3_SECURITYLEVEL_AUTHNOPRIV)
		{
			session.securityLevel = SNMP_SEC_LEVEL_AUTHNOPRIV;
			
			/* set the authentication method to MD5 */
			session.securityAuthProto = usmHMACMD5AuthProtocol;
			session.securityAuthProtoLen = USM_AUTH_PROTO_MD5_LEN;
			session.securityAuthKeyLen = USM_AUTH_KU_LEN;

			if (generate_Ku(session.securityAuthProto,
				session.securityAuthProtoLen,
				(u_char *) item->snmpv3_authpassphrase, strlen(item->snmpv3_authpassphrase),
				session.securityAuthKey,
				&session.securityAuthKeyLen) != SNMPERR_SUCCESS)
			{
				zbx_snprintf(error,MAX_STRING_LEN,"Error generating Ku from authentication pass phrase.");

				zabbix_log( LOG_LEVEL_ERR, error);
				SET_MSG_RESULT(value, strdup(error));

				return FAIL;
			}
		}
		else if(item->snmpv3_securitylevel == ITEM_SNMPV3_SECURITYLEVEL_AUTHPRIV)
		{
			session.securityLevel = SNMP_SEC_LEVEL_AUTHPRIV;

			/* set the authentication method to MD5 */
			session.securityAuthProto = usmHMACMD5AuthProtocol;
			session.securityAuthProtoLen = USM_AUTH_PROTO_MD5_LEN;
			session.securityAuthKeyLen = USM_AUTH_KU_LEN;

			if (generate_Ku(session.securityAuthProto,
				session.securityAuthProtoLen,
				(u_char *) item->snmpv3_authpassphrase, strlen(item->snmpv3_authpassphrase),
				session.securityAuthKey,
				&session.securityAuthKeyLen) != SNMPERR_SUCCESS)
			{
				zbx_snprintf(error,MAX_STRING_LEN,"Error generating Ku from authentication pass phrase.");

				zabbix_log( LOG_LEVEL_ERR, error);
				SET_MSG_RESULT(value, strdup(error));

				return FAIL;
			}
			
			/* set the private method to DES */
			session.securityPrivProto = usmDESPrivProtocol;
    			session.securityPrivProtoLen = USM_PRIV_PROTO_DES_LEN;
			session.securityPrivKeyLen = USM_PRIV_KU_LEN;
			
			if (generate_Ku(session.securityAuthProto,
				session.securityAuthProtoLen,
		                (u_char *) item->snmpv3_privpassphrase, strlen(item->snmpv3_privpassphrase),
				session.securityPrivKey,
				&session.securityPrivKeyLen) != SNMPERR_SUCCESS) 
			{
				zbx_snprintf(error,MAX_STRING_LEN,"Error generating Ku from priv pass phrase.");

				zabbix_log( LOG_LEVEL_ERR, error);
				SET_MSG_RESULT(value, strdup(error));

				return FAIL;
			}
		}
		zabbix_log( LOG_LEVEL_DEBUG, "SNMPv3 [%s@%s:%d]",session.securityName, session.peername, session.remote_port);
	}
	else
	{
		zbx_snprintf(error,MAX_STRING_LEN,"Error in get_value_SNMP. Unsupported session.version [%d]",(int)session.version);
		zabbix_log( LOG_LEVEL_ERR, error);
		SET_MSG_RESULT(value, strdup(error));
		
		return FAIL;
	}

	zabbix_log( LOG_LEVEL_DEBUG, "OID [%s]", item->snmp_oid);

	SOCK_STARTUP;
	ss = snmp_open(&session);

	if(ss == NULL)
	{
		SOCK_CLEANUP;

		zbx_snprintf(error,MAX_STRING_LEN,"Error doing snmp_open()");
		zabbix_log( LOG_LEVEL_ERR, error);
		SET_MSG_RESULT(value, strdup(error));

		return FAIL;
	}
	zabbix_log( LOG_LEVEL_DEBUG, "In get_value_SNMP() 0.2");

	pdu = snmp_pdu_create(SNMP_MSG_GET);
	read_objid(item->snmp_oid, anOID, &anOID_len);

#if OTHER_METHODS
	get_node("sysDescr.0", anOID, &anOID_len);
	read_objid(".1.3.6.1.2.1.1.1.0", anOID, &anOID_len);
	read_objid("system.sysDescr.0", anOID, &anOID_len);
#endif

	snmp_add_null_var(pdu, anOID, anOID_len);
	zabbix_log( LOG_LEVEL_DEBUG, "In get_value_SNMP() 0.3");
  
	status = snmp_synch_response(ss, pdu, &response);
	zabbix_log( LOG_LEVEL_DEBUG, "Status send [%d]", status);
	zabbix_log( LOG_LEVEL_DEBUG, "In get_value_SNMP() 0.4");

	zabbix_log( LOG_LEVEL_DEBUG, "In get_value_SNMP() 1");

	if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR)
	{

	zabbix_log( LOG_LEVEL_DEBUG, "In get_value_SNMP() 2");
/*		for(vars = response->variables; vars; vars = vars->next_variable)
		{
			print_variable(vars->name, vars->name_length, vars);
		}*/

		for(vars = response->variables; vars; vars = vars->next_variable)
		{
			int count=1;
			zabbix_log( LOG_LEVEL_DEBUG, "AV loop()");

/*			if(	(vars->type == ASN_INTEGER) ||*/
			if(	(vars->type == ASN_UINTEGER)||
				(vars->type == ASN_COUNTER) ||
#ifdef OPAQUE_SPECIAL_TYPES
				(vars->type == ASN_UNSIGNED64) ||
#endif
				(vars->type == ASN_TIMETICKS) ||
				(vars->type == ASN_GAUGE)
			)
			{
/*				*result=(long)*vars->val.integer;*/
				/*
				 * This solves situation when large numbers are stored as negative values
				 * http://sourceforge.net/tracker/index.php?func=detail&aid=700145&group_id=23494&atid=378683
				 */ 
				/*sprintf(result_str,"%ld",(long)*vars->val.integer);*/
/*				zbx_snprintf(result_str,MAX_STRING_LEN,"%lu",(long)*vars->val.integer);*/

				/* Not correct. Returns huge values. */
/*				SET_UI64_RESULT(value, (zbx_uint64_t)*vars->val.integer);*/
				SET_UI64_RESULT(value, (unsigned long)*vars->val.integer);
				zabbix_log( LOG_LEVEL_DEBUG, "OID [%s] Type [%d] UI64[" ZBX_FS_UI64 "]", item->snmp_oid, vars->type, (zbx_uint64_t)*vars->val.integer);
				zabbix_log( LOG_LEVEL_DEBUG, "OID [%s] Type [%d] ULONG[%lu]", item->snmp_oid, vars->type, (uint64_t)(unsigned long)*vars->val.integer);
			}
			else if(vars->type == ASN_COUNTER64)
			{
/*				*result=((long)(vars->val.counter64->high)<<32)+(long)(vars->val.counter64->low);*/
				SET_UI64_RESULT(value, ((vars->val.counter64->high)<<32)+(vars->val.counter64->low));
			}
			else if(vars->type == ASN_INTEGER
#define ASN_FLOAT           (ASN_APPLICATION | 8)
#define ASN_DOUBLE          (ASN_APPLICATION | 9)

#ifdef OPAQUE_SPECIAL_TYPES
				|| (vars->type == ASN_INTEGER64)
#endif
			)
			{
				SET_UI64_RESULT(value, (zbx_uint64_t)*vars->val.integer);
/*				*result=(long)*vars->val.integer;
				zbx_snprintf(result_str,MAX_STRING_LEN,"%ld",(long)*vars->val.integer);*/
			}
#ifdef OPAQUE_SPECIAL_TYPES
			else if(vars->type == ASN_FLOAT)
			{
/*				*result=(double)*vars->val.floatVal;
				zbx_snprintf(result_str,MAX_STRING_LEN,"%f",(double)*vars->val.floatVal);*/
				
				SET_DBL_RESULT(value, *vars->val.floatVal);
			}
			else if(vars->type == ASN_DOUBLE)
			{
/*				*result=(double)*vars->val.doubleVal;
				zbx_snprintf(result_str,MAX_STRING_LEN,"%lf",(double)*vars->val.doubleVal);*/
				SET_DBL_RESULT(value, *vars->val.doubleVal);
			}
#endif
			else if(vars->type == ASN_OCTET_STR)
			{
/*				memcpy(result_str,vars->val.string,vars->val_len);
				result_str[vars->val_len] = '\0';*/
				if(item->value_type == ITEM_VALUE_TYPE_FLOAT)
				{
					p = malloc(vars->val_len+1);
					if(p)
					{
						memcpy(p, vars->val.string, vars->val_len);
						p[vars->val_len] = '\0';
						dbl = strtod(p, NULL);

						SET_DBL_RESULT(value, dbl);
					}
					else
					{
						zbx_snprintf(error,MAX_STRING_LEN,"Cannot allocate required memory");
						zabbix_log( LOG_LEVEL_ERR, error);
						SET_MSG_RESULT(value, strdup(error));
					}
				}
				else if(item->value_type != ITEM_VALUE_TYPE_STR)
				{
					zbx_snprintf(error,MAX_STRING_LEN,"Cannot store SNMP string value (ASN_OCTET_STR) in item having numeric type");
					zabbix_log( LOG_LEVEL_ERR, error);
					SET_MSG_RESULT(value, strdup(error));

					ret = NOTSUPPORTED;
				}
				else
				{
					p = malloc(vars->val_len+1);
					if(p)
					{
						memcpy(p, vars->val.string, vars->val_len);
						p[vars->val_len] = '\0';

						SET_STR_RESULT(value, p);
					}
					else
					{
						zbx_snprintf(error,MAX_STRING_LEN,"Cannot allocate required memory");
						zabbix_log( LOG_LEVEL_ERR, error);
						SET_MSG_RESULT(value, strdup(error));
					}
				}
			}
			else if(vars->type == ASN_IPADDRESS)
			{
/*				ip = vars->val.string;
				zbx_snprintf(result_str,MAX_STRING_LEN,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);*/
/*				if(item->type == 0)
				{
					ret = NOTSUPPORTED;
				}*/
				if(item->value_type != ITEM_VALUE_TYPE_STR)
				{
					zbx_snprintf(error,MAX_STRING_LEN,"Cannot store SNMP string value (ASN_IPADDRESS) in item having numeric type");
					zabbix_log( LOG_LEVEL_ERR, error);
					SET_MSG_RESULT(value, strdup(error));
					ret = NOTSUPPORTED;
				}
				else
				{
					p = malloc(16);
					if(p)
					{
						zbx_snprintf(p,MAX_STRING_LEN,"%d.%d.%d.%d",ip[0],ip[1],ip[2],ip[3]);

						SET_STR_RESULT(value, p);
					}
					else
					{
						zbx_snprintf(error,MAX_STRING_LEN,"Cannot allocate required memory");
						zabbix_log( LOG_LEVEL_ERR, error);
						SET_MSG_RESULT(value, strdup(error));
					}
				}
			}
			else
			{
/* count is not really used. Has to be removed */ 
				count++;

				zbx_snprintf(error,MAX_STRING_LEN,"OID [%s] value #%d has unknow type [%X]",item->snmp_oid, count,vars->type);

				zabbix_log( LOG_LEVEL_ERR, error);
				SET_MSG_RESULT(value, strdup(error));

				ret  = NOTSUPPORTED;
			}
		}
	}
	else
	{
		if (status == STAT_SUCCESS)
		{
			zabbix_log( LOG_LEVEL_WARNING, "SNMP error in packet. Reason: %s\n",
				snmp_errstring(response->errstat));
			if(response->errstat == SNMP_ERR_NOSUCHNAME)
			{
				zbx_snprintf(error,MAX_STRING_LEN,"SNMP error [%s]", snmp_errstring(response->errstat));

				zabbix_log( LOG_LEVEL_ERR, error);
				SET_MSG_RESULT(value, strdup(error));

				ret=NOTSUPPORTED;
			}
			else
			{
				zbx_snprintf(error,MAX_STRING_LEN,"SNMP error [%s]", snmp_errstring(response->errstat));

				zabbix_log( LOG_LEVEL_ERR, error);
				SET_MSG_RESULT(value, strdup(error));

				ret=FAIL;
			}
		}
		else if(status == STAT_TIMEOUT)
		{
			zbx_snprintf(error,MAX_STRING_LEN,"Timeout while connecting to [%s]",session.peername);

/*			snmp_sess_perror("snmpget", ss);*/
			zabbix_log( LOG_LEVEL_ERR, error);
			SET_MSG_RESULT(value, strdup(error));

			ret = NETWORK_ERROR;
		}
		else
		{
			zbx_snprintf(error,MAX_STRING_LEN,"SNMP error [%d]",status);

			zabbix_log( LOG_LEVEL_ERR, error);
			SET_MSG_RESULT(value, strdup(error));

			ret=FAIL;
		}
	}

	if (response)
	{
		snmp_free_pdu(response);
	}
	snmp_close(ss);

	SOCK_CLEANUP;
	return ret;
}
#endif
