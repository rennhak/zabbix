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
#include "log.h"

static	char log_filename[MAX_STRING_LEN];

static	int log_type = LOG_TYPE_UNDEFINED;
static	int log_level;

int zabbix_open_log(int type,int level, const char *filename)
{
#ifdef TODO

	FILE *log_file = NULL;
/* Just return if we do not want to write debug */
	log_level = level;

	if(level == LOG_LEVEL_EMPTY)
	{
		return	SUCCEED;
	}

	if(type == LOG_TYPE_SYSLOG)
	{
        	openlog("zabbix_suckerd",LOG_PID,LOG_USER);
        	setlogmask(LOG_UPTO(LOG_WARNING));
		log_type = LOG_TYPE_SYSLOG;
	}
	else if(type == LOG_TYPE_FILE)
	{
		log_file = fopen(filename,"a+");
		if(log_file == NULL)
		{
			fprintf(stderr, "Unable to open log file [%s] [%s]\n", filename, strerror(errno));
			return	FAIL;
		}
		log_type = LOG_TYPE_FILE;
		strscpy(log_filename,filename);
		fclose(log_file);
	}
	else
	{
/* Not supported logging type */
		fprintf(stderr, "Not supported loggin type [%d]\n", type);
		return	FAIL;
	}

#endif /* TODO */

	return	SUCCEED;
}

void zabbix_set_log_level(int level)
{
	log_level = level;
}

void zabbix_log(int level, const char *fmt, ...)
{
	FILE *log_file = NULL;

	char	str[MAX_STRING_LEN];
	char	str2[MAX_STRING_LEN];
	time_t	t;
	struct	tm	*tm;
	va_list ap;

	struct stat	buf;
	char	filename_old[MAX_STRING_LEN];
	
	if( (level>log_level) || (level == LOG_LEVEL_EMPTY))
	{
		return;
	}

	if(log_type == LOG_TYPE_SYSLOG)
	{
#ifdef TODO
		va_start(ap,fmt);
		vsprintf(str,fmt,ap);
		strncat(str,"\n",MAX_STRING_LEN);
		str[MAX_STRING_LEN-1]=0;
		syslog(LOG_DEBUG,str);
		va_end(ap);
#endif /* TODO */
	}
	else if(log_type == LOG_TYPE_FILE)
	{
		t=time(NULL);
		tm=localtime(&t);
		snprintf(str2,sizeof(str2)-1,"%.6d:%.4d%.2d%.2d:%.2d%.2d%.2d ",(int)getpid(),tm->tm_year+1900,tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);

		va_start(ap,fmt);
		vsnprintf(str,MAX_STRING_LEN,fmt,ap);
		va_end(ap);

		log_file = fopen(log_filename,"a+");
		if(log_file == NULL)
		{
			return;
		}
		fprintf(log_file,"%s",str2);
		fprintf(log_file,"%s",str);
		fprintf(log_file,"\n");
		fclose(log_file);


		if(stat(log_filename,&buf) == 0)
		{
			if(buf.st_size > MAX_LOG_FILE_LEN)
			{
				strscpy(filename_old,log_filename);
				strncat(filename_old,".old",MAX_STRING_LEN);
				if(rename(log_filename,filename_old) != 0)
				{
/*					exit(1);*/
				}
			}
		}
	}
	else
	{
		/* Log is not opened */
	}	
        return;
}

