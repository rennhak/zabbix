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

#ifndef ZABBIX_ALIAS_H
#define ZABBIX_ALIAS_H

#define MAX_ALIAS_NAME        120

struct zbx_alias
{
   struct zbx_alias *next;
   char name[MAX_ALIAS_NAME];
   char *value;
};

typedef struct zbx_alias ALIAS;

int 	AddAlias(char *name,char *value);
void	FreeAliasList(void);
void	ExpandAlias(char *orig,char *expanded);


#endif /* ZABBIX_ALIAS_H */
