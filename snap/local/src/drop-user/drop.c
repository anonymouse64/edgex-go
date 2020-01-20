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

#define _GNU_SOURCE
#include <dlfcn.h>
#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* This code is based on example example code published om launchpad and github:
 *
 * - https://git.launchpad.net/~jdstrand/+git/test-setgroups
 * - https://github.com/ncopa/su-exec.git
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

static int (*original_setgroups) (size_t, const gid_t[]);

int main(int argc, char *argv[])
{
	char **cmdargv;
	char *user = "snap_daemon";

	if (argc < 2) {
	  printf("Usage: %s command [args]\n", argv[0]);
	  exit(0);
	}

	cmdargv = &argv[1];

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

	/* Convert our username to a passwd entry */
	struct passwd *pwd = getpwnam(user);
	if (pwd == NULL) {
		printf("'%s' not found\n", user);
		exit(EXIT_FAILURE);
	}

	/* Drop supplementary groups first if can, using portable method
	 * (should fail without LD_PRELOAD)
	 */
	if (geteuid() == 0 && original_setgroups(0, NULL) < 0) {
		perror("setgroups");
		goto fail;
	}

	/* Drop gid after supplementary groups */
	if (setgid(pwd->pw_gid) < 0) {
		perror("setgid");
		goto fail;
	}

	/* Drop uid after gid */
	if (setuid(pwd->pw_uid) < 0) {
		perror("setuid");
		goto fail;
	}

	execvp(cmdargv[0], cmdargv);
	err(1, "%s", cmdargv[0]);
	exit (1);

 fail:
	exit(EXIT_FAILURE);
}
