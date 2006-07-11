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

#include "config.h"

#include "common.h"
#include "sysinfo.h"

struct net_stat_s {
	unsigned long ibytes;
	unsigned long ipackets;
	unsigned long ierr;
	unsigned long idrop;
	unsigned long obytes;
	unsigned long opackets;
	unsigned long oerr;
	unsigned long odrop;
	unsigned long colls;
};

static int get_net_stat(const char *if_name, struct net_stat_s *result)
{
	int ret = SYSINFO_RET_FAIL;
	char line[MAX_STRING_LEN];

	char name[MAX_STRING_LEN];
	unsigned long tmp = 0;
	
	FILE *f;
	char	*p;

	assert(result);

	if(NULL != (f = fopen("/proc/net/dev","r") ))
	{
		
		while(fgets(line,MAX_STRING_LEN,f) != NULL)
		{

			p = strstr(line,":");
			if(p) p[0]='\t';
			
			if(sscanf(line,"%s\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t \
					    %lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\n",
				name, 
				&(result->ibytes), 	/* bytes */
				&(result->ipackets),	/* packets */
				&(result->ierr), 	/* errs */
				&(result->idrop),	/* drop */
			        &(tmp), 		/* fifo */
				&(tmp),			/* frame */
				&(tmp), 		/* compressed */
				&(tmp),			/* multicast */
				&(result->obytes), 	/* bytes */
				&(result->opackets),	/* packets*/
				&(result->oerr),	/* errs */
				&(result->odrop),	/* drop */
			        &(tmp), 		/* fifo */
				&(result->colls),	/* icolls */
			        &(tmp), 		/* carrier */
			        &(tmp)	 		/* compressed */
				) == 17)
			{
				if(strncmp(name, if_name, MAX_STRING_LEN) == 0)
				{
					ret = SYSINFO_RET_OK;
					break;
				}
			}
		}
		zbx_fclose(f);
	}

	if(ret != SYSINFO_RET_OK)
	{
		memset(result, 0, sizeof(struct net_stat_s));
	}
	
	return ret;
}

int	NET_IF_IN(const char *cmd, const char *param, unsigned flags, AGENT_RESULT *result)
{
	struct net_stat_s	ns;
	
	char	if_name[MAX_STRING_LEN];
	char	mode[MAX_STRING_LEN];
	
	int ret = SYSINFO_RET_FAIL;
        
	assert(result);

        init_result(result);

        if(num_param(param) > 2)
        {
                return SYSINFO_RET_FAIL;
        }

        if(get_param(param, 1, if_name, sizeof(if_name)) != 0)
        {
                return SYSINFO_RET_FAIL;
        }
	
	if(get_param(param, 2, mode, sizeof(mode)) != 0)
        {
                mode[0] = '\0';
        }
        if(mode[0] == '\0')
	{
		/* default parameter */
		zbx_snprintf(mode, sizeof(mode), "bytes");
	}

	ret = get_net_stat(if_name, &ns);
	

	if(ret == SYSINFO_RET_OK)
	{
		if(strncmp(mode, "bytes", MAX_STRING_LEN) == 0)
		{
			SET_UI64_RESULT(result, ns.ibytes);
		} 
		else if(strncmp(mode, "packets", MAX_STRING_LEN) == 0)
		{
			SET_UI64_RESULT(result, ns.ipackets);
		}
		else if(strncmp(mode, "errors", MAX_STRING_LEN) == 0)
		{
			SET_UI64_RESULT(result, ns.ierr);
		}
		else if(strncmp(mode, "dropped", MAX_STRING_LEN) == 0)
		{
			SET_UI64_RESULT(result, ns.idrop);
		}
		else
		{
			ret = SYSINFO_RET_FAIL;
		}
	}
	
	return ret;
}

int	NET_IF_OUT(const char *cmd, const char *param, unsigned flags, AGENT_RESULT *result)
{
	struct net_stat_s	ns;
	
	char	if_name[MAX_STRING_LEN];
	char	mode[MAX_STRING_LEN];
	
	int ret = SYSINFO_RET_FAIL;
        
	assert(result);

        init_result(result);

        if(num_param(param) > 2)
        {
                return SYSINFO_RET_FAIL;
        }

        if(get_param(param, 1, if_name, sizeof(if_name)) != 0)
        {
                return SYSINFO_RET_FAIL;
        }
	
	if(get_param(param, 2, mode, sizeof(mode)) != 0)
        {
                mode[0] = '\0';
        }
        if(mode[0] == '\0')
	{
		/* default parameter */
		zbx_snprintf(mode, sizeof(mode), "bytes");
	}

	ret = get_net_stat(if_name, &ns);
	

	if(ret == SYSINFO_RET_OK)
	{
		if(strncmp(mode, "bytes", MAX_STRING_LEN) == 0)
		{
			SET_UI64_RESULT(result, ns.obytes);
		} 
		else if(strncmp(mode, "packets", MAX_STRING_LEN) == 0)
		{
			SET_UI64_RESULT(result, ns.opackets);
		}
		else if(strncmp(mode, "errors", MAX_STRING_LEN) == 0)
		{
			SET_UI64_RESULT(result, ns.oerr);
		}
		else if(strncmp(mode, "dropped", MAX_STRING_LEN) == 0)
		{
			SET_UI64_RESULT(result, ns.odrop);
		}
		else
		{
			ret = SYSINFO_RET_FAIL;
		}
	}
	
	return ret;
}

int	NET_IF_TOTAL(const char *cmd, const char *param, unsigned flags, AGENT_RESULT *result)
{
	struct net_stat_s	ns;
	
	char	if_name[MAX_STRING_LEN];
	char	mode[MAX_STRING_LEN];
	
	int ret = SYSINFO_RET_FAIL;
        
	assert(result);

        init_result(result);

        if(num_param(param) > 2)
        {
                return SYSINFO_RET_FAIL;
        }

        if(get_param(param, 1, if_name, sizeof(if_name)) != 0)
        {
                return SYSINFO_RET_FAIL;
        }
	
	if(get_param(param, 2, mode, sizeof(mode)) != 0)
        {
                mode[0] = '\0';
        }
        if(mode[0] == '\0')
	{
		/* default parameter */
		zbx_snprintf(mode, sizeof(mode), "bytes");
	}

	ret = get_net_stat(if_name, &ns);
	

	if(ret == SYSINFO_RET_OK)
	{
		if(strncmp(mode, "bytes", MAX_STRING_LEN) == 0)
		{
			SET_UI64_RESULT(result, ns.ibytes + ns.obytes);
		} 
		else if(strncmp(mode, "packets", MAX_STRING_LEN) == 0)
		{
			SET_UI64_RESULT(result, ns.ipackets + ns.opackets);
		}
		else if(strncmp(mode, "errors", MAX_STRING_LEN) == 0)
		{
			SET_UI64_RESULT(result, ns.ierr + ns.oerr);
		}
		else if(strncmp(mode, "dropped", MAX_STRING_LEN) == 0)
		{
			SET_UI64_RESULT(result, ns.idrop + ns.odrop);
		}
		else
		{
			ret = SYSINFO_RET_FAIL;
		}
	}
	
	return ret;
}

int     NET_TCP_LISTEN(const char *cmd, const char *param, unsigned flags, AGENT_RESULT *result)
{
        assert(result);

        init_result(result);
	
	return SYSINFO_RET_FAIL;
}

int     NET_IF_COLLISIONS(const char *cmd, const char *param, unsigned flags, AGENT_RESULT *result)
{
	struct net_stat_s	ns;
	
	char	if_name[MAX_STRING_LEN];
	
	int ret = SYSINFO_RET_FAIL;
        
	assert(result);

        init_result(result);

        if(num_param(param) > 1)
        {
                return SYSINFO_RET_FAIL;
        }

        if(get_param(param, 1, if_name, MAX_STRING_LEN) != 0)
        {
                return SYSINFO_RET_FAIL;
        }
	

	ret = get_net_stat(if_name, &ns);
	

	if(ret == SYSINFO_RET_OK)
	{
		SET_UI64_RESULT(result, ns.colls);
	}
	
	return ret;
}

