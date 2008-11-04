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

#ifndef ZABBIX_ACTIVE_H
#define ZABBIX_ACTIVE_H

#include "threads.h"

extern char	*CONFIG_SOURCE_IP;
extern char	*CONFIG_HOSTNAME;
extern int	CONFIG_REFRESH_ACTIVE_CHECKS;
extern int	CONFIG_BUFFER_SEND;
extern int	CONFIG_BUFFER_SIZE;

#define MAX_LINES_PER_SECOND	100

typedef struct zbx_active_metric_type
{
	char	*key, *key_orig;
	int	refresh;
	int	nextcheck;
	int	status;
/* Must be long for fseek() */
	long	lastlogsize;
} ZBX_ACTIVE_METRIC;

typedef struct active_ckeck_args
{
	char		*host;
	unsigned short	port;
} ZBX_THREAD_ACTIVECHK_ARGS;

typedef struct zbx_active_buffer_element_type
{
	char	*host;
	char	*key;
	char	*value;
	int	timestamp;
	char	*source;
	int	severity;
	long	lastlogsize;
	int	clock;
} ZBX_ACTIVE_BUFFER_ELEMENT;

typedef struct zbx_active_buffer_type
{
	ZBX_ACTIVE_BUFFER_ELEMENT	*data;
	int	count;
} ZBX_ACTIVE_BUFFER;

ZBX_THREAD_ENTRY(active_checks_thread, args);

#endif /* ZABBIX_ACTIVE_H */
