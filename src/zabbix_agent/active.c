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
#include "active.h"

#include "log.h"
#include "zbxsock.h"
#include "threads.h"

METRIC	*metrics = NULL;

void	init_list()
{
	zabbix_log( LOG_LEVEL_DEBUG, "In init_list()");

	if(metrics==NULL)
	{
		metrics=malloc(sizeof(METRIC));
		metrics[0].key=NULL;
	}
	else
	{
		zabbix_log( LOG_LEVEL_WARNING, "Metrics are already initialised");
	}
}

void	disable_all_metrics()
{
	int i;

	zabbix_log( LOG_LEVEL_DEBUG, "In delete_all_metrics()");
	for(i=0;;i++)
	{
		if(metrics[i].key == NULL)	break;

		metrics[i].status = ITEM_STATUS_NOTSUPPORTED;
	}
}

int	get_min_nextcheck()
{
	int i;
	int min=-1;
	int nodata=0;

	for(i=0;;i++)
	{
		if(metrics[i].key == NULL)	break;

		nodata=1;
		if( (metrics[i].status == ITEM_STATUS_ACTIVE) &&
		    ((metrics[i].nextcheck < min) || (min == -1)))
		{
			min=metrics[i].nextcheck;
		}
	}

	if(nodata==0)
	{
		return	FAIL;
	}
	return min;
}

void	add_check(char *key, int refresh, int lastlogsize)
{
	int i;

	zabbix_log( LOG_LEVEL_DEBUG, "In add check [%s]", key);

	for(i=0;;i++)
	{
		if(metrics[i].key == NULL)
		{
			metrics[i].key=strdup(key);
			metrics[i].refresh=refresh;
			metrics[i].nextcheck=0;
			metrics[i].status=ITEM_STATUS_ACTIVE;
			metrics[i].lastlogsize=lastlogsize;

			metrics=realloc(metrics,(i+2)*sizeof(METRIC));
			metrics[i+1].key=NULL;
			break;
		}
		else if(strcmp(metrics[i].key,key)==0)
		{
			if(metrics[i].refresh!=refresh)
			{
				metrics[i].nextcheck=0;
			}
			metrics[i].refresh=refresh;
			metrics[i].lastlogsize=lastlogsize;
			metrics[i].status=ITEM_STATUS_ACTIVE;
			break;
		}
	}
}

/******************************************************************************
 *                                                                            *
 * Function: parse_list_of_checks                                             *
 *                                                                            *
 * Purpose: Parse list of active checks received from server                  *
 *                                                                            *
 * Parameters: str - NULL terminated string received from server              *
 *                                                                            *
 * Return value: returns SUCCEED on succesfull parsing,                       *
 *               FAIL on an incoorrect format of string                       *
 *                                                                            *
 * Author: Eugene Grigorjev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *    String reprents as "ZBX_EOF" termination list                           *
 *    With '\n' delimeter between elements.                                   *
 *    Each element represents as:                                             *
 *           <key>:<refresh time>:<last log size>                             *
 *                                                                            *
 ******************************************************************************/

int	parse_list_of_checks(char *str)
{
	char 
		*p, 
		*key, 
		*refresh, 
		*lastlogsize;

	disable_all_metrics();

	while(str)
	{
		p = strchr((str = p),'\n');
		if(p) p[0] = '\0';

		zabbix_log(LOG_LEVEL_DEBUG, "Parsed [%s]", p);
		if(strcmp(str, "ZBX_EOF") == 0)	break;

		if(p) p[0] = '\n';

		p = strchr(str,':');
		if(p) { p[0] = '\0'; p++; } else return FAIL;
		key = str;
		zabbix_log( LOG_LEVEL_DEBUG, "Key [%s]", key);

		p = strchr((str = p),':');
		if(p) { p[0] = '\0'; p++; } else return FAIL;
		refresh = str;
		zabbix_log( LOG_LEVEL_DEBUG, "Refresh [%s]", refresh);

		p = strchr((str = p),'\n');
		if(p) { p[0] = '\0'; p++; }
		lastlogsize = str;
		zabbix_log( LOG_LEVEL_DEBUG, "Lastlogsize [%s]", lastlogsize);
		str = p;		
		
		add_check(key, atoi(refresh), atoi(lastlogsize));
	}

	return SUCCEED;
}

int	get_active_checks(char *server, unsigned short port, char *error, int max_error_len)
{

	ZBX_SOCKET	s;

#ifdef TODO

	int		len,amount_read;

#endif /* TODO - partial (1 of 2) */

	char		c[MAX_BUF_LEN];

	struct hostent *hp;

	ZBX_SOCKADDR servaddr_in;

	zabbix_log( LOG_LEVEL_DEBUG, "get_active_checks: host[%s] port[%u]", server, port);

	servaddr_in.sin_family = AF_INET;
	hp = gethostbyname(server);

	if(hp==NULL)
	{
#ifdef	HAVE_HSTRERROR		
		zabbix_log( LOG_LEVEL_WARNING, "gethostbyname() failed [%s]", hstrerror(h_errno));
		snprintf(error,max_error_len-1,"gethostbyname() failed [%s]", (char*)hstrerror((int)h_errno));
#else
		zabbix_log( LOG_LEVEL_WARNING, "gethostbyname() failed [%d]", h_errno);
		snprintf(error,max_error_len-1,"gethostbyname() failed [%d]", h_errno);
#endif
		return	NETWORK_ERROR;
	}

	servaddr_in.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;

	servaddr_in.sin_port = htons(port);

	s = socket(AF_INET,SOCK_STREAM,0);

	if(s == -1)
	{
		zabbix_log(LOG_LEVEL_WARNING, "Cannot create socket [%s]", strerror(errno));
		snprintf(error,max_error_len-1,"Cannot create socket [%s]", strerror(errno));
		return	FAIL;
	}
 
	if(connect(s,(struct sockaddr *)&servaddr_in,sizeof(struct sockaddr_in)) == SOCKET_ERROR )
	{
		switch (errno)
		{
			case EINTR:
				zabbix_log( LOG_LEVEL_WARNING, "Timeout while connecting to [%s:%u]",server,port);
				snprintf(error,max_error_len-1,"Timeout while connecting to [%s:%u]",server,port);
				break;
			case EHOSTUNREACH:
				zabbix_log( LOG_LEVEL_WARNING, "No route to host [%s:%u]",server,port);
				snprintf(error,max_error_len-1,"No route to host [%s:%u]",server,port);
				break;
			default:
				zabbix_log( LOG_LEVEL_WARNING, "Cannot connect to [%s:%u] [%s]",server,port,strerror(errno));
				snprintf(error,max_error_len-1,"Cannot connect to [%s:%u] [%s]",server,port,strerror(errno));
		} 
		zbx_sock_close(s);
		return	NETWORK_ERROR;
	}

#ifdef TODO
	snprintf(c,sizeof(c)-1,"%s\n%s\n","ZBX_GET_ACTIVE_CHECKS",CONFIG_HOSTNAME);
	zabbix_log(LOG_LEVEL_DEBUG, "Sending [%s]", c);
	if( write(s,c,strlen(c)) == -1 )
	{
		switch (errno)
		{
			case EINTR:
				zabbix_log( LOG_LEVEL_WARNING, "Timeout while sending data to [%s:%u]",server,port);
				snprintf(error,max_error_len-1,"Timeout while sending data to [%s:%u]",server,port);
				break;
			default:
				zabbix_log( LOG_LEVEL_WARNING, "Error while sending data to [%s:%u] [%s]",server,port,strerror(errno));
				snprintf(error,max_error_len-1,"Error while sending data to [%s:%u] [%s]",server,port,strerror(errno));
		} 
		close(s);
		return	FAIL;
	} 

	memset(c,0,MAX_BUF_LEN);

	zabbix_log(LOG_LEVEL_DEBUG, "Before read");

	amount_read = 0;

	do
	{
		len=read(s,c+amount_read,(MAX_BUF_LEN-1)-amount_read);
		if (len > 0)
			amount_read += len;
		if(len == -1)
		{
			switch (errno)
			{
				case 	EINTR:
						zabbix_log( LOG_LEVEL_WARNING, "Timeout while receiving data from [%s:%u]",server,port);
						snprintf(error,max_error_len-1,"Timeout while receiving data from [%s:%u]",server,port);
						break;
				case	ECONNRESET:
						zabbix_log( LOG_LEVEL_WARNING, "Connection reset by peer.");
						snprintf(error,max_error_len-1,"Connection reset by peer.");
						close(s);
						return	NETWORK_ERROR;
				default:
						zabbix_log( LOG_LEVEL_WARNING, "Error while receiving data from [%s:%u] [%s]",server,port,strerror(errno));
						snprintf(error,max_error_len-1,"Error while receiving data from [%s:%u] [%s]",server,port,strerror(errno));
			} 
			close(s);
			return	FAIL;
		}
	}
	while (len > 0);

#endif /* TODO - partial (2 of 2) //don't forgot variables */

	parse_list_of_checks(c);

	if( zbx_sock_close(s)!=0 )
	{
		zabbix_log(LOG_LEVEL_WARNING, "Problem with close [%s]", strerror(errno));
	}

	return SUCCEED;
}

int	send_value(char *server,unsigned short port,char *host, char *key,char *value, char *lastlogsize)
{
	ZBX_SOCKET	s;
	char	tosend[MAX_STRING_LEN];
	char	result[MAX_STRING_LEN];
	struct hostent *hp;

	ZBX_SOCKADDR myaddr_in;
	ZBX_SOCKADDR servaddr_in;

	zabbix_log( LOG_LEVEL_DEBUG, "In send_value([%s])",lastlogsize);

	servaddr_in.sin_family=AF_INET;
	hp=gethostbyname(server);

	if(hp==NULL)
	{
		return	FAIL;
	}

	servaddr_in.sin_addr.s_addr=((struct in_addr *)(hp->h_addr))->s_addr;

	servaddr_in.sin_port = htons(port);

	s=socket(AF_INET,SOCK_STREAM,0);
	if(s == -1)
	{
		zabbix_log( LOG_LEVEL_WARNING, "Error in socket() [%s:%u] [%s]",server,port, strerror(errno));
		return	FAIL;
	}

/*	ling.l_onoff=1;*/
/*	ling.l_linger=0;*/
/*	if(setsockopt(s,SOL_SOCKET,SO_LINGER,&ling,sizeof(ling))==-1)*/
/*	{*/
/* Ignore */
/*	}*/
 
	myaddr_in.sin_family = AF_INET;
	myaddr_in.sin_port=0;
	myaddr_in.sin_addr.s_addr=INADDR_ANY;

	if( connect(s,(struct sockaddr *)&servaddr_in,sizeof(struct sockaddr_in)) == -1 )
	{
		zabbix_log( LOG_LEVEL_WARNING, "Error in connect() [%s:%u] [%s]",server, port, strerror(errno));
		zbx_sock_close(s);
		return	FAIL;
	}

	comms_create_request(host, key, value, lastlogsize, tosend, sizeof(tosend)-1);
/*	snprintf(tosend,sizeof(tosend)-1,"%s:%s\n",shortname,value); */
	zabbix_log( LOG_LEVEL_DEBUG, "XML before sonding [%s]",tosend);

	if( sendto(s,tosend,strlen(tosend),0,(struct sockaddr *)&servaddr_in,sizeof(struct sockaddr_in)) == -1 )
	{
		zabbix_log( LOG_LEVEL_WARNING, "Error in sendto() [%s:%u] [%s]",server, port, strerror(errno));
		zbx_sock_close(s);
		return	FAIL;
	} 
/*	i=sizeof(struct sockaddr_in);
	i=recvfrom(s,result,1023,0,(struct sockaddr *)&servaddr_in,(socklen_t *)&i);*/

#ifdef TODO

	int i = read(s,result,MAX_STRING_LEN-1);
	if(s==-1)
	{
		zabbix_log( LOG_LEVEL_WARNING, "Error in recvfrom() [%s:%u] [%s]",server,port, strerror(errno));
		close(s);
		return	FAIL;
	}

	result[i-1]=0;

#endif /* TODO */

	if(strcmp(result,"OK") == 0)
	{
		zabbix_log( LOG_LEVEL_DEBUG, "OK");
	}
	else
	{
		zabbix_log( LOG_LEVEL_DEBUG, "NOT OK [%s:%s]", host, key);
	}
 
	if(zbx_sock_close(s)!=0 )
	{
		zabbix_log( LOG_LEVEL_WARNING, "Error in close() [%s] [%s]",server, strerror(errno));
	}

	return SUCCEED;
}

int	process_active_checks(char *server, unsigned short port)
{
	char	value[MAX_STRING_LEN];
	char	lastlogsize[MAX_STRING_LEN];
	int	i, now, count;
	int	ret = SUCCEED;

	char	c[MAX_STRING_LEN];
	char	*filename;

	AGENT_RESULT	result;

	memset(&result, 0, sizeof(AGENT_RESULT));

	now=time(NULL);

	for(i=0;;i++)
	{
		if(metrics[i].key == NULL)			break;
		if(metrics[i].nextcheck>now)			continue;
		if(metrics[i].status!=ITEM_STATUS_ACTIVE)	continue;

		/* Special processing for log files */
		if(strncmp(metrics[i].key,"log[",4) == 0)
		{
			strscpy(c,metrics[i].key);
			filename=strtok(c,"[]");
			filename=strtok(NULL,"[]");

			count=0;
			while(process_log(filename,&metrics[i].lastlogsize,value) == 0)
			{
/*				snprintf(shortname, MAX_STRING_LEN-1,"%s:%s",CONFIG_HOSTNAME,metrics[i].key);
				zabbix_log( LOG_LEVEL_DEBUG, "%s",shortname); */
				snprintf(lastlogsize, MAX_STRING_LEN-1,"%d",metrics[i].lastlogsize);

				if(send_value(server,port,CONFIG_HOSTNAME,metrics[i].key,value,lastlogsize) == FAIL)
				{
					ret = FAIL;
					break;
				}
				if(strcmp(value,"ZBX_NOTSUPPORTED\n")==0)
				{
					metrics[i].status=ITEM_STATUS_NOTSUPPORTED;
					zabbix_log( LOG_LEVEL_WARNING, "Active check [%s] is not supported. Disabled.", metrics[i].key);
					break;
				}
				count++;
				/* Do not flood ZABBIX server if file grows too fast */
				if(count >= MAX_LINES_PER_SECOND*metrics[i].refresh)	break;
			}
		}
		else
		{
			lastlogsize[0]=0;
			
			process(metrics[i].key, 0, &result);
			if(result.type & AR_DOUBLE)
				 snprintf(value, MAX_STRING_LEN-1, ZBX_FS_DBL, result.dbl);
			else if(result.type & AR_UINT64)
                                 snprintf(value, MAX_STRING_LEN-1, ZBX_FS_UI64, result.ui64);
			else if(result.type & AR_STRING)
                                 snprintf(value, MAX_STRING_LEN-1, "%s", result.str);
			else if(result.type & AR_TEXT)
                                 snprintf(value, MAX_STRING_LEN-1, "%s", result.text);
			else if(result.type & AR_MESSAGE)
                                 snprintf(value, MAX_STRING_LEN-1, "%s", result.msg);
			free_result(&result);

/*			snprintf(shortname, MAX_STRING_LEN-1,"%s:%s",CONFIG_HOSTNAME,metrics[i].key);
			zabbix_log( LOG_LEVEL_DEBUG, "%s",shortname); */
			if(send_value(server,port,CONFIG_HOSTNAME,metrics[i].key,value,lastlogsize) == FAIL)
			{
				ret = FAIL;
				break;
			}

			if(strcmp(value,"ZBX_NOTSUPPORTED\n")==0)
			{
				metrics[i].status=ITEM_STATUS_NOTSUPPORTED;
				zabbix_log( LOG_LEVEL_WARNING, "Active check [%s] is not supported. Disabled.", metrics[i].key);
			}
		}

		metrics[i].nextcheck=time(NULL)+metrics[i].refresh;
	}
	return ret;
}

void	refresh_metrics(char *server, unsigned short port, char *error, int max_error_len)
{
	zabbix_log( LOG_LEVEL_DEBUG, "In refresh_metrics()");

	while(get_active_checks(server, port, error, sizeof(error)) != SUCCEED)
	{
		zabbix_log( LOG_LEVEL_WARNING, "Getting list of active checks failed. Will retry after 60 seconds");

		zbx_setproctitle("poller [sleeping for %d seconds]", 60);

		zbx_sleep(60);
	}
}

ZBX_THREAD_ENTRY(ActiveChecksThread, args)
{
	ZBX_THREAD_ACTIVECHK_ARGS *activechk_args = (ZBX_THREAD_ACTIVECHK_ARGS *)args;

	char	error[MAX_STRING_LEN];
	int	sleeptime, nextcheck;
	int	nextrefresh;

	zabbix_log( LOG_LEVEL_WARNING, "zabbix_agentd %ld started",(long)getpid());

	zbx_setproctitle("getting list of active checks");

	init_list();

	refresh_metrics(activechk_args->host, activechk_args->port, error, sizeof(error));
	nextrefresh = time(NULL) + CONFIG_REFRESH_ACTIVE_CHECKS;

	for(;;)
	{

		zbx_setproctitle("processing active checks");

		if(process_active_checks(activechk_args->host, activechk_args->port) == FAIL)
		{
			zbx_sleep(60);
			continue;
		}

		nextcheck = get_min_nextcheck();
		if(FAIL == nextcheck)
		{
			sleeptime = 60;
		}
		else
		{
			sleeptime = nextcheck - time(NULL);

			sleeptime = MAX(sleeptime, 0);
		}

		if(sleeptime > 0)
		{
			sleeptime = MIN(sleeptime, 60);

			zabbix_log(LOG_LEVEL_DEBUG, "Sleeping for %d seconds", sleeptime );

			zbx_setproctitle("poller [sleeping for %d seconds]", sleeptime);

			zbx_sleep( sleeptime );
		}
		else
		{
			zabbix_log(LOG_LEVEL_DEBUG, "No sleeping" );
		}

		if(time(NULL) >= nextrefresh)
		{
			refresh_metrics(activechk_args->host, activechk_args->port, error, sizeof(error));
			nextrefresh=time(NULL) + CONFIG_REFRESH_ACTIVE_CHECKS;
		}
	}

	FreeMetrics();

	zbx_tread_exit(0);

}

