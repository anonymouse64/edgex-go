/* -*- Mode: C; indent-tabs-mode: nil; tab-width: 4 -*-
 *
 * Copyright (C) 2019 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

/*
 * $ gcc -Wall -fPIC -shared -o mylib.so ./lib.c -ldl
 * $ LD_PRELOAD=./mylib.so ...
 */

/* This code is based on example example code published om launchpad:
 *
 * - https://git.launchpad.net/~jdstrand/+git/test-setgroups
 *
 * It's meant as a temporary solution for the Fuji version of the edgexfoundry
 * snap, until the following PR (based on this code) lands in snap-preload:
 *
 * https://github.com/sergiusens/snapcraft-preload/pull/39
 *
 * The Fuji snap originally used gosu command to run postgres commands as the
 * 'snap_daemon' user, but as goso doesn't support the extrausers passwd db
 * extension used on Ubuntu Core, the snap couldn't be installed on a Core system.
*/
#include <dlfcn.h>
#include <stdio.h>
#include <grp.h>

static int (*original_setgroups) (size_t, const gid_t[]);
static int (*original_initgroups) (const char *, const gid_t);

int setgroups(size_t size, const gid_t *list) {
	// lookup the libc's setgroups() if we haven't already
	if (!original_setgroups) {
		dlerror();
		original_setgroups = dlsym(RTLD_NEXT, "setgroups");
		if (!original_setgroups) {
			fprintf(stderr, "could not find setgroups in libc");
			return -1;
		}
		dlerror();
	}

	return original_setgroups(0, NULL);
}

int initgroups(const char *user, const gid_t group) {
	// lookup the libc's setgroups() if we haven't already
	if (!original_initgroups) {
		dlerror();
		original_initgroups = dlsym(RTLD_NEXT, "initgroups");
		if (!original_initgroups) {
			fprintf(stderr, "could not find initgroups in libc");
			return -1;
		}
		dlerror();
	}

	return setgroups(0, NULL);
}
