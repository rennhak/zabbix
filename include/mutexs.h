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

#ifndef ZABBIX_MUTEXS_H
#define ZABBIX_MUTEXS_H

#if defined(_WINDOWS)

#	define ZBX_MUTEX		HANDLE
#	define ZBX_MUTEX_ERROR		(0)
#	define ZBX_MUTEX_OK		(1)

#	define ZBX_MUTEX_NAME		char*

#	define ZBX_MUTEX_LOG    	"ZBX_MUTEX_LOG"
#	define ZBX_MUTEX_NODE_SYNC    	"ZBX_MUTEX_NODE_SYNC"

#else /* not _WINDOWS */

#	define ZBX_MUTEX		int

#	define ZBX_MUTEX_ERROR		(-1)
#	define ZBX_MUTEX_OK		(1)
	
#	define ZBX_MUTEX_NAME		int

#	define ZBX_MUTEX_LOG		0
#	define ZBX_MUTEX_NODE_SYNC	1

#	define ZBX_MUTEX_COUNT		2

#	define ZBX_MUTEX_MAX_TRIES	20 /* seconds */

#	if defined(HAVE_SQLITE3)

#	endif

#endif /* _WINDOWS */

#define zbx_mutex_create(mutex, name)		zbx_mutex_create_ext(mutex, name, 0)
#define zbx_mutex_create_force(mutex, name)	zbx_mutex_create_ext(mutex, name, 1)
int zbx_mutex_create_ext(ZBX_MUTEX	*mutex, ZBX_MUTEX_NAME name, unsigned char forced);
int zbx_mutex_lock(ZBX_MUTEX	*mutex);
int zbx_mutex_unlock(ZBX_MUTEX	*mutex);
int zbx_mutex_destroy(ZBX_MUTEX	*mutex);

/*********************************************************/
/*** PHP Semaphore functions using System V semaphores ***/
/*********************************************************/

#define PHP_MUTEX_OK	1
#define PHP_MUTEX_ERROR	0

typedef struct {
	int semid;
	int count;
} PHP_MUTEX;

#if defined(HAVE_SQLITE3) && !defined(_WINDOWS)

int php_sem_get(PHP_MUTEX* sem_ptr, char* path_name);
int php_sem_acquire(PHP_MUTEX* sem_ptr);
int php_sem_release(PHP_MUTEX* sem_ptr);
int php_sem_remove(PHP_MUTEX* sem_ptr);

#else /* !HAVE_SQLITE3 || _WINDOWS */

#	define php_sem_get(sem_ptr, path_name) PHP_MUTEX_OK
#	define php_sem_acquire(sem_ptr)        PHP_MUTEX_OK
#	define php_sem_release(sem_ptr)        PHP_MUTEX_OK
#	define php_sem_remove(sem_ptr)         PHP_MUTEX_OK

#endif  /* HAVE_SQLITE3 && !_WINDOWS */

#endif /* ZABBIX_MUTEXS_H */
