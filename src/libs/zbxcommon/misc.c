#include "common.h"

#include <stdlib.h>
#include <math.h>
#include "log.h"

/******************************************************************************
 *                                                                            *
 * Function: calculate_item_nextcheck                                         *
 *                                                                            *
 * Purpose: calculate nextcheck timespamp for item                            *
 *                                                                            *
 * Parameters: delay - item's refresh rate in sec                             *
 *             now - current timestamp                                        *
 *                                                                            *
 * Return value: nextcheck value                                              *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments: Old algorithm: now+delay                                         *
 *           New one: preserve period, if delay==5, nextcheck = 0,5,10,15,... *
 *                                                                            *
 ******************************************************************************/
int	calculate_item_nextcheck(int itemid, int delay, char *delay_flex, time_t now)
{
	int	i;
	char	*p;
	char	delay_period[30];
	int	delay_val;

	zabbix_log( LOG_LEVEL_DEBUG, "In calculate_item_nextcheck [%d, %d, %s, %d]",itemid,delay,delay_flex,now);

	if(delay_flex)
	{
		do
		{
			p = strchr(delay_flex, ';');
			if(p) *p = '\0';
			
			zabbix_log( LOG_LEVEL_DEBUG, "Delay period [%s]", delay_flex);

			if(sscanf(delay_flex, "%d/%29s",&delay_val,delay_period) == 2)
			{
				zabbix_log( LOG_LEVEL_DEBUG, "%d sec at %s",delay_val,delay_period);
				if(check_time_period(delay_period, now))
				{
					delay = delay_val;
					break;
				}
			}
			else
			{
				zabbix_log( LOG_LEVEL_ERR, "Delay period format is wrong [%s]",delay_flex);
			}
			if(p)
			{
				*p = ';'; /* restore source string */
				delay_flex = p+1;
			}
		}while(p);
		
	}

/*	Old algorithm */
/*	i=delay*(int)(now/delay);*/
	
	zabbix_log( LOG_LEVEL_DEBUG, "Delay is [%d]",delay);

	i=delay*(int)(now/delay)+(itemid % delay);

	while(i<=now)	i+=delay;

	return i;
}

/******************************************************************************
 *                                                                            *
 * Function: check_time_period                                                *
 *                                                                            *
 * Purpose: check if current time is within given period                      *
 *                                                                            *
 * Parameters: period - time period in format [d1-d2,hh:mm-hh:mm]*            *
 *             now    - timestamp for comporation                             *
 *                      if NULL - use current timestamp.                      *
 *                                                                            *
 * Return value: 0 - out of period, 1 - within the period                     *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
int	check_time_period(const char *period, time_t now)
{
	//time_t	now;
	char	tmp[MAX_STRING_LEN];
	char	*s;
	int	d1,d2,h1,h2,m1,m2;
	int	day, hour, min;
	struct  tm      *tm;
	int	ret = 0;


	zabbix_log( LOG_LEVEL_DEBUG, "In check_time_period(%s)",period);

	if(now == (time_t)NULL)	now = time(NULL);
	
	tm = localtime(&now);

	day=tm->tm_wday;
	if(0 == day)	day=7;
	hour = tm->tm_hour;
	min = tm->tm_min;

	strscpy(tmp,period);
       	s=(char *)strtok(tmp,";");
	while(s!=NULL)
	{
		zabbix_log( LOG_LEVEL_DEBUG, "Period [%s]",s);

		if(sscanf(s,"%d-%d,%d:%d-%d:%d",&d1,&d2,&h1,&m1,&h2,&m2) == 6)
		{
			zabbix_log( LOG_LEVEL_DEBUG, "%d-%d,%d:%d-%d:%d",d1,d2,h1,m1,h2,m2);
			if( (day>=d1) && (day<=d2) && (60*hour+min>=60*h1+m1) && (60*hour+min<=60*h2+m2))
			{
				ret = 1;
				break;
			}
		}
		else
		{
			zabbix_log( LOG_LEVEL_ERR, "Time period format is wrong [%s]",period);
		}

       		s=(char *)strtok(NULL,";");
	}
	return ret;
}

/******************************************************************************
 *                                                                            *
 * Function: cmp_double                                                       *
 *                                                                            *
 * Purpose: compares two float values                                         *
 *                                                                            *
 * Parameters: a,b - floats to compare                                        *
 *                                                                            *
 * Return value:  0 - the values are equal                                    *
 *                1 - otherwise                                               *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments: equal == differs less than 0.000001                              *
 *                                                                            *
 ******************************************************************************/
int	cmp_double(double a,double b)
{
	if(fabs(a-b)<0.000001)
	{
		return	0;
	}
	return	1;
}

/******************************************************************************
 *                                                                            *
 * Function: is_double_prefix                                                 *
 *                                                                            *
 * Purpose: check if the string is float                                      *
 *                                                                            *
 * Parameters: c - string to check                                            *
 *                                                                            *
 * Return value:  SUCCEED - the string is float                               *
 *                FAIL - otherwise                                            *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments: the functions support prefixes K,M,G                             *
 *                                                                            *
 ******************************************************************************/
int	is_double_prefix(char *c)
{
	int i;
	int dot=-1;

	for(i=0;c[i]!=0;i++)
	{
		if((c[i]>='0')&&(c[i]<='9'))
		{
			continue;
		}

		if((c[i]=='.')&&(dot==-1))
		{
			dot=i;

			if((dot!=0)&&(dot!=(int)strlen(c)-1))
			{
				continue;
			}
		}
		/* Last digit is prefix 'K', 'M', 'G' */
		if( ((c[i]=='K')||(c[i]=='M')||(c[i]=='G')) && (i == (int)strlen(c)-1))
		{
			continue;
		}

		return FAIL;
	}
	return SUCCEED;
}

/******************************************************************************
 *                                                                            *
 * Function: is_double                                                        *
 *                                                                            *
 * Purpose: check if the string is float                                      *
 *                                                                            *
 * Parameters: c - string to check                                            *
 *                                                                            *
 * Return value:  SUCCEED - the string is float                               *
 *                FAIL - otherwise                                            *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
/*int is_double(char *str)
{
	const char *endstr = str + strlen(str);
	char *endptr = NULL;
	double x;
       
	x = strtod(str, &endptr);

	if(endptr == str || errno != 0)
		return FAIL;
	if (endptr == endstr)
		return SUCCEED;
	return FAIL;
}*/

int	is_double(char *c)
{
	int i;
	int dot=-1;
	int len;

	for(i=0; c[i]==' ' && c[i]!=0;i++); /* trim left spaces */

	for(len=0; c[i]!=0; i++, len++)
	{
		if((c[i]>='0')&&(c[i]<='9'))
		{
			continue;
		}

		if((c[i]=='.')&&(dot==-1))
		{
			dot=i;
			continue;
		}

		if(c[i]==' ') /* check right spaces */
		{
			for( ; c[i]==' ' && c[i]!=0;i++); /* trim right spaces */
			
			if(c[i]==0) break; /* SUCCEED */
		}

		return FAIL;
	}
	
	if(len <= 0) return FAIL;

	if(len == 1 && dot!=-1) return FAIL;

	return SUCCEED;
}

/******************************************************************************
 *                                                                            *
 * Function: is_uint                                                          *
 *                                                                            *
 * Purpose: check if the string is unsigned integer                           *
 *                                                                            *
 * Parameters: c - string to check                                            *
 *                                                                            *
 * Return value:  SUCCEED - the string is unsigned integer                    *
 *                FAIL - otherwise                                            *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
int	is_uint(char *c)
{
	int	i;
	int	len;

	for(i=0; c[i]==' ' && c[i]!=0;i++); /* trim left spaces */
	
	for(len=0; c[i]!=0; i++,len++)
	{
		if((c[i]>='0')&&(c[i]<='9'))
		{
			continue;
		}
		
		if(c[i]==' ') /* check right spaces */
		{
			for( ; c[i]==' ' && c[i]!=0;i++); /* trim right spaces */
			
			if(c[i]==0) break; /* SUCCEED */
		}
		return FAIL;
	}

	if(len <= 0) return FAIL;
	
	return SUCCEED;
}

int	set_result_type(AGENT_RESULT *result, int value_type, char *c)
{
	int ret = FAIL;

	if(value_type == ITEM_VALUE_TYPE_UINT64)
	{
		del_zeroes(c);
		if(is_uint(c) == SUCCEED)
		{
#ifdef HAVE_ATOLL
			SET_UI64_RESULT(result, atoll(c));
#else
			SET_UI64_RESULT(result, atol(c));
#endif
			ret = SUCCEED;
		}
	}
	else if(value_type == ITEM_VALUE_TYPE_FLOAT)
	{
		if(is_double(c) == SUCCEED)
		{
			SET_DBL_RESULT(result, atof(c));
			ret = SUCCEED;
		}
		else if(is_uint(c) == SUCCEED)
		{
#ifdef HAVE_ATOLL
			SET_DBL_RESULT(result, (double)atoll(c));
#else
			SET_DBL_RESULT(result, (double)atol(c));
#endif
			ret = SUCCEED;
		}
	}
	else if(value_type == ITEM_VALUE_TYPE_STR)
	{
		SET_STR_RESULT(result, strdup(c));
		ret = SUCCEED;
	}
	else if(value_type == ITEM_VALUE_TYPE_TEXT)
	{
		SET_TEXT_RESULT(result, strdup(c));
		ret = SUCCEED;
	}
	else if(value_type == ITEM_VALUE_TYPE_LOG)
	{
		SET_STR_RESULT(result, strdup(c));
		ret = SUCCEED;
	}

	return ret;
}
