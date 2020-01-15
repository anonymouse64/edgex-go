#define _GNU_SOURCE
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
 * - https://github.com/incopa/su-exec.git
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
int main(int argc, char *argv[])
{
	char **cmdargv;
	char *user = "snap_daemon";


	if (argc < 2) {
	  printf("Usage: %s command [args]\n", argv[0]);
	  exit(0);
	}

	cmdargv = &argv[1];

	/* Convert our username to a passwd entry */
	struct passwd *pwd = getpwnam(user);
	if (pwd == NULL) {
		printf("'%s' not found\n", user);
		exit(EXIT_FAILURE);
	}

	/* Drop supplementary groups first if can, using portable method
	 * (should fail without LD_PRELOAD)
	 */
	gid_t gid_list[1];
	gid_list[0] = pwd->pw_gid;
	if (geteuid() == 0 && setgroups(1, gid_list) < 0) {
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
