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

/******************************************************************************
 *                                                                            *
 * Function: app_title                                                        *
 *                                                                            *
 * Purpose: print title of application on stdout                              *
 *                                                                            *
 * Author: Eugene Grigorjev                                                   *
 *                                                                            *
 * Comments:  title_message - is global variable which must be initialized    *
 *                            in each zabbix application                      *
 *                                                                            *
 ******************************************************************************/
static void app_title()
{
	printf("%s v%s (%s)\n", title_message, ZABBIX_VERSION, ZABBIX_REVDATE);
}

/******************************************************************************
 *                                                                            *
 * Function: version                                                          *
 *                                                                            *
 * Purpose: print version and compilation time of application on stdout       *
 *          by application request with parameter '-v'                        *
 *                                                                            *
 * Author: Eugene Grigorjev                                                   *
 *                                                                            *
 ******************************************************************************/
void version()
{
	app_title();
	printf("Compilation time:  %s %s\n", __DATE__, __TIME__);
}

/******************************************************************************
 *                                                                            *
 * Function: usage                                                            *
 *                                                                            *
 * Purpose: print applicatin parameters on stdout                             *
 *                                                                            *
 * Author: Eugene Grigorjev                                                   *
 *                                                                            *
 * Comments:  usage_message - is global variable which must be initialized    *
 *                            in each zabbix application                      *
 *                                                                            *
 ******************************************************************************/
void usage()
{
	printf("usage: %s %s\n", progname, usage_message);
}

/******************************************************************************
 *                                                                            *
 * Function: help                                                             *
 *                                                                            *
 * Purpose: print help of applicatin parameters on stdout by application      *
 *          request with parameter '-h'                                       *
 *                                                                            *
 * Author: Eugene Grigorjev                                                   *
 *                                                                            *
 * Comments:  help_message - is global variable which must be initialized     *
 *                            in each zabbix application                      *
 *                                                                            *
 ******************************************************************************/
void help()
{
	char **p = help_message;
	
	app_title();
	usage();
	while (*p) printf("%s\n", *p++);
}

/******************************************************************************
 *                                                                            *
 * Function: find_char                                                        *
 *                                                                            *
 * Purpose: locate a character in the string                                  *
 *                                                                            *
 * Parameters: str - string                                                   *
 *             c   - character to find                                        *
 *                                                                            *
 * Return value:  position of the character                                   *
 *                FAIL - otherwise                                            *
 *                                                                            *
 * Author: Eugene Grigorjev                                                   *
 *                                                                            *
 * Comments: !!! beter use system functions like 'strchr' !!!                 *
 *                                                                            *
 ******************************************************************************/
int	find_char(char *str,char c)
{
	char *p;
	for(p = str; *p; p++) 
		if(*p == c) return (p - str);

	return	FAIL;
}

/******************************************************************************
 *                                                                            *
 * Function: zbx_error                                                        *
 *                                                                            *
 * Purpose: Print error text to the stderr                                    *
 *                                                                            *
 * Parameters: fmt - format of mesage                                         *
 *                                                                            *
 * Return value:                                                              *
 *                                                                            *
 * Author: Eugene Grigorjev                                                   *
 *                                                                            *
 ******************************************************************************/
void zbx_error(const char *fmt, ...)
{
	va_list args;
    
	va_start(args, fmt);

	fprintf(stderr, "%s: ",progname);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\n");

	va_end(args);
}

/* Has to be rewritten to avoi malloc */
char *string_replace(char *str, const char *sub_str1, const char *sub_str2)
{
        char *new_str;
        const char *p;
        const char *q;
        const char *r;
        char *t;
        signed long len;
        signed long diff;
        unsigned long count = 0;

        if ( (p=strstr(str, sub_str1)) == NULL )
                return str;
        ++count;

        len = strlen(sub_str1);

        /* count the number of occurances of sub_str1 */
        for ( p+=len; (p=strstr(p, sub_str1)) != NULL; p+=len )
                ++count;

        diff = strlen(sub_str2) - len;

        /* allocate new memory */
        if ( (new_str=(char *)malloc((size_t)(strlen(str) + count*diff)*sizeof(char)))
                        == NULL )
                return NULL;

        q = str;
        t = new_str;
        for (p=strstr(str, sub_str1); p!=NULL; p=strstr(p, sub_str1))
        {
                /* copy until next occurance of sub_str1 */
                for ( ; q < p; *t++ = *q++)
                        ;
                q += len;
                p = q;
                for ( r = sub_str2; (*t++ = *r++); )
                        ;
                --t;
        }
        /* copy the tail of str */
        while ( (*t++ = *q++) )
                ;
        return new_str;

}

/******************************************************************************
 *                                                                            *
 * Function: del_zeroes                                                       *
 *                                                                            *
 * Purpose: delete all right '0' and '.' for the string                       *
 *                                                                            *
 * Parameters: s - string to trim '0'                                         *
 *                                                                            *
 * Return value: string without right '0'                                     *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:  10.0100 => 10.01, 10. => 10                                                                 *
 *                                                                            *
 ******************************************************************************/
void del_zeroes(char *s)
{
	int     i;

	if(strchr(s,'.')!=NULL)
	{
		for(i=strlen(s)-1;;i--)
		{
			if(s[i]=='0')
			{
				s[i]=0;
			}
			else if(s[i]=='.')
			{
				s[i]=0;
				break;
			}
			else
			{
				break;
			}
		}
	}
}


/******************************************************************************
 *                                                                            *
 * Function: get_param                                                        *
 *                                                                            *
 * Purpose: return parameter by index (num) from parameter list (param)       *
 *                                                                            *
 * Parameters:                                                                *
 * 	param  - parameter list                                               *
 *      num    - requested parameter index                                    *
 *      buf    - pointer of output buffer                                     *
 *      maxlem - size of output buffer                                        *
 *                                                                            *
 * Return value:                                                              *
 *      1 - requested parameter missed                                        *
 *      0 - requested parameter founded (value - 'buf' can be empty string)   *
 *                                                                            *
 * Author: Eugene Grigorjev                                                   *
 *                                                                            *
 * Comments:  delimeter vor parameters is ','                                 *
 *                                                                            *
 ******************************************************************************/
int	get_param(const char *param, int num, char *buf, int maxlen)
{
	char	tmp[MAX_STRING_LEN];
	char	*s;
	int	ret = 1;
	int	i = 0;
	int	idx = 0;

	strscpy(tmp,param);

	s = &tmp[0];
	
	for(i=0; tmp[i] != '\0'; i++)
	{
		if(tmp[i] == ',')
		{
			idx++;
			if(idx == num)
			{
				tmp[i]='\0';
				strncpy(buf, s, maxlen);
				tmp[i]=','; /* restore source string */
				ret = 0;
				break;
				
			}
			s = &tmp[i+1];
		}
	}

	if(ret != 0)
	{
		idx++;
		if(idx == num)
		{
			strncpy(buf, s, maxlen);
			ret = 0;
		}
	}
	
	return ret;
}

/******************************************************************************
 *                                                                            *
 * Function: num_param                                                        *
 *                                                                            *
 * Purpose: calculate count of parameters from parameter list (param)         *
 *                                                                            *
 * Parameters:                                                                *
 * 	param  - parameter list                                               *
 *                                                                            *
 * Return value: count of parameters                                          *
 *                                                                            *
 * Author: Eugene Grigorjev                                                   *
 *                                                                            *
 * Comments:  delimeter vor parameters is ','                                 *
 *                                                                            *
 ******************************************************************************/
int	num_param(const char *param)
{
	int	i;
	int	ret = 1;

	if(param == NULL) 
		return 0;
	
	for(i=0;param[i]!=0;i++)
	{
		if(param[i]==',')	ret++;
	}

	return ret;
}

/******************************************************************************
 *                                                                            *
 * Function: delete_reol                                                      *
 *                                                                            *
 * Purpose: delete all right EOL characters                                   *
 *                                                                            *
 * Parameters: c - string to delete EOL                                       *
 *                                                                            *
 * Return value:  the string wtihout EOL                                      *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
void	delete_reol(char *c)
{
	int i;

	for(i=(int)strlen(c)-1;i>=0;i--)
	{
		if( c[i] != '\n')	break;
		c[i]=0;
	}
}

/******************************************************************************
 *                                                                            *
 * Function: rtrim_spaces                                                     *
 *                                                                            *
 * Purpose: delete all right spaces for the string                            *
 *                                                                            *
 * Parameters: c - string to trim spaces                                      *
 *                                                                            *
 * Return value: string without right spaces                                  *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
void	rtrim_spaces(char *c)
{
	int i,len;

	len=strlen(c);
	for(i=len-1;i>=0;i--)
	{
		if( c[i] == ' ')
		{
			c[i]=0;
		}
		else	break;
	}
}

/******************************************************************************
 *                                                                            *
 * Function: ltrim_spaces                                                     *
 *                                                                            *
 * Purpose: delete all left spaces for the string                             *
 *                                                                            *
 * Parameters: c - string to trim spaces                                      *
 *                                                                            *
 * Return value: string without left spaces                                   *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
void	ltrim_spaces(char *c)
{
	int i;
/* Number of left spaces */
	int spaces=0;

	for(i=0;c[i]!=0;i++)
	{
		if( c[i] == ' ')
		{
			spaces++;
		}
		else	break;
	}
	for(i=0;c[i+spaces]!=0;i++)
	{
		c[i]=c[i+spaces];
	}

	c[strlen(c)-spaces]=0;
}

/******************************************************************************
 *                                                                            *
 * Function: lrtrim_spaces                                                    *
 *                                                                            *
 * Purpose: delete all left and right spaces for the string                   *
 *                                                                            *
 * Parameters: c - string to trim spaces                                      *
 *                                                                            *
 * Return value: string without left and right spaces                         *
 *                                                                            *
 * Author: Alexei Vladishev                                                   *
 *                                                                            *
 * Comments:                                                                  *
 *                                                                            *
 ******************************************************************************/
void	lrtrim_spaces(char *c)
{
	ltrim_spaces(c);
	rtrim_spaces(c);
}
