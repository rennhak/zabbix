#include "zabbixw32.h"

#define MAX_B64_SIZE 16*1024

static char base64_set [] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void str_base64_encode(char *p_str, char *p_b64str, int in_size);
void str_base64_decode(char *p_b64str, char *p_str, int *p_out_size);
static char char_base64_encode(unsigned char uc);
static unsigned char char_base64_decode(char c);
static int is_base64 (char c);

/*------------------------------------------------------------------------
 *
 * Function	:  is_base64
 *
 * Purpose	:  Is the character passed in a base64 character ?
 *
 * Parameters	:  
 *
 * Returns	:  
 *
 * Comments	:
 *
 *----------------------------------------------------------------------*/
static int is_base64 (char c)
{
	if ( (c >= '0' && c <= '9')
	  || (c >= 'a' && c <= 'z')
	  || (c >= 'A' && c <= 'Z')
	  || c == '/'
	  || c == '+'
	  || c == '='		) 

	{
		return 1;
	}
	
	return 0;
}
/*------------------------------------------------------------------------
 *
 * Function	:  char_base64_encode
 *
 * Purpose	:  Encode a byte into a base64 character
 *
 * Parameters	:  
 *
 * Returns	:  
 *
 * Comments	:
 *
 *----------------------------------------------------------------------*/
static char char_base64_encode(unsigned char uc)
{
	return base64_set[uc];
}

/*------------------------------------------------------------------------
 *
 * Function	:  char_base64_decode
 *
 * Purpose	:  Decode a base64 character into a byte
 *
 * Parameters	:  
 *
 * Returns	:  
 *
 * Comments	:
 *
 *----------------------------------------------------------------------*/
static unsigned char char_base64_decode(char c)
{
	if (c >= 'A' && c <= 'Z')
	{
		return (unsigned char)(c - 'A');
	}
	
	if (c >= 'a' && c <= 'z')
	{
		return (unsigned char)(c - 'a' + 26);
	}
	
	if (c >= '0' && c <= '9')
	{
		return (unsigned char)(c - '0' + 52);
	}
	
	if (c == '+')
	{
		return (unsigned char)(62);
	}
	
	return (unsigned char)(63);
}
/*------------------------------------------------------------------------
 *
 * Function	:  str_base64_encode
 *
 * Purpose	:  Encode a string into a base64 string
 *
 * Parameters	:  p_str (in)		- the string to encode
 *		   p_b64str (out)	- the encoded str to return
 *		   in_size (in)		- size (length) of input str
 * Returns	:  
 *
 * Comments	:
 *
 *----------------------------------------------------------------------*/
void str_base64_encode(char *p_str, char *p_b64str, int in_size)
{
	int 	i;
	unsigned char from1=0,from2=0,from3=0;
	unsigned char to1=0,to2=0,to3=0,to4=0;
	
	if ( 0 == in_size )
	{
		return;
	};
	
	for ( i = 0; i < in_size ; i += 3 )
	{
		from1 = from2 = from3 = 0;
		from1 = p_str[i];
		if (i+1 < in_size)
		{
			from2 = p_str[i+1];
		}
		if (i+2 < in_size)
		{
			from3 = p_str[i+2];
		}

		to1 = to2 = to3 = to4 = 0;
		to1 = (unsigned char)((from1>>2) & 0x3f);
		to2 = (unsigned char)(((from1&0x3)<<4)|(from2>>4));
		to3 = (unsigned char)(((from2&0xf)<<2)|(from3>>6));
		to4 = (unsigned char)(from3&0x3f);

		*(p_b64str++) = char_base64_encode(to1);
		*(p_b64str++) = char_base64_encode(to2);

		if (i+1 < in_size)
		{
			*(p_b64str++) = char_base64_encode(to3);
		}
		else
		{
			*(p_b64str++) = '=';	/* Padding */
		}
		if (i+2 < in_size)
		{
			*(p_b64str++) = char_base64_encode(to4);
		}
		else
		{
			*(p_b64str++) = '=';	/* Padding */
		};

/*		if ( i % (76/4*3) == 0)
		{
			*(p_b64str++) = '\r';
			*(p_b64str++) = '\n';
		}*/
	};
	
	return;
}
/*------------------------------------------------------------------------
 *
 * Function	:  str_base64_decode
 *
 * Purpose	:  Decode a base64 string into a string
 *
 * Parameters	:  p_b64str (in)	- the base64 string to decode
 *		   p_str (out)		- the encoded str to return
 *		   p_out_size (out)	- the size (len) of the str decoded
 *
 * Returns	:  
 *
 * Comments	:
 *
 *----------------------------------------------------------------------*/
void str_base64_decode(char *p_b64str, char *p_str, int *p_out_size)
{
	int i;
	int j = 0;
	int	in_size;
	char from1='A',from2='A',from3='A',from4='A';
	unsigned char to1=0,to2=0,to3=0,to4=0;
	char	str_clean[MAX_B64_SIZE];/* str_clean is the string 
					* after removing the non-base64 
					* characters
					*/
		
	in_size = (int)strlen(p_b64str);
	memset(str_clean, 0, sizeof(str_clean));
	*p_out_size = 0;
	
	/* Clean-up input string */
	for ( i=0; i < in_size; i++ )
	{
		if (is_base64(p_b64str[i]))
		{
			str_clean[j++] = p_b64str[i];
		}
	}
	
	/* Re-define in_size after clean-up */
	in_size = (int)strlen(str_clean);
	
	if ( 0 == in_size )
	{
		return;
	}

	for ( i=0; i < in_size ;i+=4)
	{	
		from1 = from2 = from3 = from4 = 'A';
		from1 = str_clean[i];
		if ( i+1 < in_size )
		{
			from2 = str_clean[i+1];
		}		
		if ( i+2 < in_size )
		{
			from3 = str_clean[i+2];
		}
		if ( i+3 < in_size )
		{
			from4 = str_clean[i+3];
		};

		to1 = to2 = to3 = to4 = 0;
		to1 = char_base64_decode(from1);
		to2 = char_base64_decode(from2);
		to3 = char_base64_decode(from3);
		to4 = char_base64_decode(from4);

		*(p_str++) = (char)((to1<<2)|(to2>>4));
		(*p_out_size)++;
		if (from3 != '=')
		{
			*(p_str++) = (char)(((to2&0xf)<<4)|(to3>>2));
			(*p_out_size)++;
		}
		if (from4 != '=')
		{
			*(p_str++) =  (char)(((to3&0x3)<<6)|to4);
			(*p_out_size)++;
		}
	}
	
	return;
}
