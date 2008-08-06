/*
 * ** ZABBIX
 * ** Copyright (C) 2000-2005 SIA Zabbix
 * **
 * ** This program is free software; you can redistribute it and/or modify
 * ** it under the terms of the GNU General Public License as published by
 * ** the Free Software Foundation; either version 2 of the License, or
 * ** (at your option) any later version.
 * **
 * ** This program is distributed in the hope that it will be useful,
 * ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 * ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * ** GNU General Public License for more details.
 * **
 * ** You should have received a copy of the GNU General Public License
 * ** along with this program; if not, write to the Free Software
 * ** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * **/

#include "common.h"
#include "sysinfo.h"

#include <sys/sysctl.h>

#define DO_SUM 0
#define DO_MAX 1
#define DO_MIN 2
#define DO_AVG 3

#define ZBX_PROC_STAT_ALL 0
#define ZBX_PROC_STAT_RUN 1
#define ZBX_PROC_STAT_SLEEP 2
#define ZBX_PROC_STAT_ZOMB 3
	
static kvm_t	*kd = NULL;

static char	*get_commandline(struct kinfo_proc *proc)
{
	struct pargs	pa;
	size_t		sz;
	char		*p;
	static char	*args = NULL;
	static int	args_alloc = 128;

	if (NULL == kd)
		return NULL;

	sz = sizeof(pa);

	if (kvm_read(kd, (unsigned long)proc->ki_args, &pa, sz) != sz)
		return NULL;

	if (NULL == args)
		args = zbx_malloc(args, args_alloc);

	if (args_alloc < pa.ar_length)
	{
		args_alloc = pa.ar_length;
		args = zbx_realloc(args, args_alloc);
	}

	if (pa.ar_length != kvm_read(kd, (unsigned long)proc->ki_args
			+ sizeof(pa.ar_ref) + sizeof(pa.ar_length), args, pa.ar_length))
		return NULL;

	p = args;
	sz = 1; /* do not change last '\0' */

	do {
		if (*p == '\0')
			*p = ' ';
		p++;
	} while (++sz < pa.ar_length);

	return args;
}

static void	init_kernel_access()
{
	static int	already = 0;

	if (1 == already)
		return;

	kd = kvm_open(NULL, NULL, NULL, O_RDONLY, NULL);

	already = 1;
}

/*
 *	proc.mem[<process_name><,user_name><,mode><,command_line>]
 *		<mode> : *sum, avg, max, min
 *
 *	Tested: FreeBSD 7.0
 */

int     PROC_MEMORY(const char *cmd, const char *param, unsigned flags, AGENT_RESULT *result)
{
	char	procname[MAX_STRING_LEN],
		buffer[MAX_STRING_LEN],
		proccomm[MAX_STRING_LEN], *args;
	int	do_task, pagesize, count, i,
		proc_ok, comm_ok,
		mib[4], mibs;

	double	value = 0.0,
		memsize = 0;
	int	proccount = 0;

	size_t	sz;

	struct kinfo_proc	*proc = NULL;
	struct passwd		*usrinfo;

	assert(result);

	init_result(result);

	init_kernel_access();

	if (num_param(param) > 4)
		return SYSINFO_RET_FAIL;

	if (0 != get_param(param, 1, procname, sizeof(procname)))
		*procname = '\0';
	else if (strlen(procname) > COMMLEN)
		procname[COMMLEN] = '\0';

	if (0 != get_param(param, 2, buffer, sizeof(buffer)))
		*buffer = '\0';

	if (*buffer != '\0') {
		usrinfo = getpwnam(buffer);
		if (usrinfo == NULL)	/* incorrect user name */
			return SYSINFO_RET_FAIL;
	} else
		usrinfo = NULL;

	if (0 != get_param(param, 3, buffer, sizeof(buffer)))
		*buffer = '\0';

	if (*buffer != '\0') {
		if (0 == strcmp(buffer, "avg"))
			do_task = DO_AVG;
		else if (0 == strcmp(buffer, "max"))
			do_task = DO_MAX;
		else if (0 == strcmp(buffer, "min"))
			do_task = DO_MIN;
		else if (0 == strcmp(buffer, "sum"))
			do_task = DO_SUM;
		else
			return SYSINFO_RET_FAIL;
	} else
		do_task = DO_SUM;

	if (0 != get_param(param, 4, proccomm, sizeof(proccomm)))
		*proccomm = '\0';

	if (*proccomm != '\0' && kd == NULL)
		return SYSINFO_RET_FAIL;

	pagesize = getpagesize();

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	if (NULL != usrinfo) {
		mib[2] = KERN_PROC_UID;
		mib[3] = usrinfo->pw_uid;
		mibs = 4;
	} else {
		mib[2] = KERN_PROC_ALL;
		mib[3] = 0;
		mibs = 3;
	}

	sz = 0;
	if (0 != sysctl(mib, mibs, NULL, &sz, NULL, 0))
		return SYSINFO_RET_FAIL;

	proc = (struct kinfo_proc *)zbx_malloc(proc, sz);
	if (0 != sysctl(mib, mibs, proc, &sz, NULL, 0)) {
		zbx_free(proc);
		return SYSINFO_RET_FAIL;
	}

	count = sz / sizeof(struct kinfo_proc);

	for (i = 0; i < count; i++) {
		proc_ok = 0;
		comm_ok = 0;
		if (*procname == '\0' || 0 == strcmp(procname, proc[i].ki_comm))
			proc_ok = 1;

		if (*proccomm != '\0') {
			if (NULL != (args = get_commandline(&proc[i])))
				if (zbx_regexp_match(args, proccomm, NULL) != NULL)
					comm_ok = 1;
		} else
			comm_ok = 1;

		if (proc_ok && comm_ok) {
			value = proc[i].ki_tsize
				+ proc[i].ki_dsize
				+ proc[i].ki_ssize;
			value *= pagesize;

			if (0 == proccount++)
				memsize = value;
			else {
				if (do_task == DO_MAX)
					memsize = MAX(memsize, value);
				else if (do_task == DO_MIN)
					memsize = MIN(memsize, value);
				else
					memsize += value;
			}
		}
	}
	zbx_free(proc);

	if (do_task == DO_AVG) {
		SET_DBL_RESULT(result, proccount == 0 ? 0 : memsize/proccount);
	} else {
		SET_UI64_RESULT(result, memsize);
	}

	return SYSINFO_RET_OK;
}

/*
 *	proc.num[<process_name><,user_name><,state><,command_line>]
 *		<state> : *all, sleep, zomb, run
 *
 *	Tested: FreeBSD 7.0
 */

int	PROC_NUM(const char *cmd, const char *param, unsigned flags, AGENT_RESULT *result)
{
	char	procname[MAX_STRING_LEN],
		buffer[MAX_STRING_LEN],
		proccomm[MAX_STRING_LEN], *args;
	int	zbx_proc_stat, count, i,
		proc_ok, stat_ok, comm_ok,
		mib[4], mibs;

	int	proccount = 0;

	size_t	sz;

	struct kinfo_proc	*proc = NULL;
	struct passwd		*usrinfo;

	assert(result);

	init_result(result);

	init_kernel_access();

	if (num_param(param) > 4)
		return SYSINFO_RET_FAIL;

	if (0 != get_param(param, 1, procname, sizeof(procname)))
		*procname = '\0';
	else if (strlen(procname) > COMMLEN)
		procname[COMMLEN] = '\0';

	if (0 != get_param(param, 2, buffer, sizeof(buffer)))
		*buffer = '\0';

	if (*buffer != '\0') {
		usrinfo = getpwnam(buffer);
		if (usrinfo == NULL)	/* incorrect user name */
			return SYSINFO_RET_FAIL;
	} else
		usrinfo = NULL;
    
	if (0 != get_param(param, 3, buffer, sizeof(buffer)))
		*buffer = '\0';
		
	if (*buffer != '\0') {
		if (0 == strcmp(buffer, "run"))
			zbx_proc_stat = ZBX_PROC_STAT_RUN;
		else if (0 == strcmp(buffer, "sleep"))
			zbx_proc_stat = ZBX_PROC_STAT_SLEEP;
		else if (0 == strcmp(buffer, "zomb"))
			zbx_proc_stat = ZBX_PROC_STAT_ZOMB;
		else if (0 == strcmp(buffer, "all"))
			zbx_proc_stat = ZBX_PROC_STAT_ALL;
		else
			return SYSINFO_RET_FAIL;
	} else
		zbx_proc_stat = ZBX_PROC_STAT_ALL;

	if (0 != get_param(param, 4, proccomm, sizeof(proccomm)))
		*proccomm = '\0';

	if (*proccomm != '\0' && kd == NULL)
		return SYSINFO_RET_FAIL;

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	if (NULL != usrinfo) {
		mib[2] = KERN_PROC_UID;
		mib[3] = usrinfo->pw_uid;
		mibs = 4;
	} else {
		mib[2] = KERN_PROC_ALL;
		mib[3] = 0;
		mibs = 3;
	}

	sz = 0;
	if (0 != sysctl(mib, mibs, NULL, &sz, NULL, 0))
		return SYSINFO_RET_FAIL;

	proc = (struct kinfo_proc *)zbx_malloc(proc, sz);
	if (0 != sysctl(mib, mibs, proc, &sz, NULL, 0)) {
		zbx_free(proc);
		return SYSINFO_RET_FAIL;
	}

	count = sz / sizeof(struct kinfo_proc);

	for (i = 0; i < count; i++) {
		proc_ok = 0;
		stat_ok = 0;
		comm_ok = 0;

		if (*procname == '\0' || 0 == strcmp(procname, proc[i].ki_comm))
			proc_ok = 1;

		if (zbx_proc_stat != ZBX_PROC_STAT_ALL) {
			switch (zbx_proc_stat) {
			case ZBX_PROC_STAT_RUN:
				if (proc[i].ki_stat == SRUN)
					stat_ok = 1;
				break;
			case ZBX_PROC_STAT_SLEEP:
				if (proc[i].ki_stat == SSLEEP)
					stat_ok = 1;
				break;
			case ZBX_PROC_STAT_ZOMB:
				if (proc[i].ki_stat == SZOMB)
					stat_ok = 1;
				break;
			}
		} else
			stat_ok = 1;

		if (*proccomm != '\0') {
			if (NULL != (args = get_commandline(&proc[i])))
				if (zbx_regexp_match(args, proccomm, NULL) != NULL)
					comm_ok = 1;
		} else
			comm_ok = 1;
		
		if (proc_ok && stat_ok && comm_ok)
			proccount++;
	}
	zbx_free(proc);

	SET_UI64_RESULT(result, proccount);

	return SYSINFO_RET_OK;
}
