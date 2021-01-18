/*
 * libusb synchronization using POSIX Threads
 *
 * Copyright © 2011 Vitali Lovich <vlovich@aliph.com>
 * Copyright © 2011 Peter Stuge <peter@stuge.se>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libusbi.h"

#if defined(__ANDROID__)
# include <unistd.h>
#elif defined(__linux__) || defined(__OpenBSD__)
# if defined(__OpenBSD__)
#  define _BSD_SOURCE
# endif
# include <sys/syscall.h>
#endif

int usbi_cond_timedwait(pthread_cond_t *cond,
	pthread_mutex_t *mutex, const struct timeval *tv)
{
	struct timespec timeout;
	int r;

	r = usbi_clock_gettime(USBI_CLOCK_REALTIME, &timeout);
	if (r < 0)
		return r;

	timeout.tv_sec += tv->tv_sec;
	timeout.tv_nsec += tv->tv_usec * 1000;
	while (timeout.tv_nsec >= 1000000000L) {
		timeout.tv_nsec -= 1000000000L;
		timeout.tv_sec++;
	}

	return pthread_cond_timedwait(cond, mutex, &timeout);
}

int usbi_get_tid(void)
{
#ifdef HAVE_CC_THREAD_LOCAL
	static _Thread_local int tid;

	if (tid)
		return tid;
#else
	int tid;
#endif

#if defined(__ANDROID__)
	tid = gettid();
#elif defined(__linux__)
	tid = syscall(SYS_gettid);
#elif defined(__OpenBSD__)
	/* The following only works with OpenBSD > 5.1 as it requires
	   real thread support. For 5.1 and earlier, -1 is returned. */
	tid = syscall(SYS_getthrid);
#elif defined(__APPLE__)
	tid = (int)pthread_mach_thread_np(pthread_self());
#elif defined(__CYGWIN__)
	tid = GetCurrentThreadId();
#else
	tid = -1;
#endif
/* TODO: NetBSD thread ID support */
	return tid;
}
