#define _GNU_SOURCE
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int display(void) {
	int i;
	uid_t ruid, euid, suid;
	gid_t rgid, egid, sgid;
	gid_t *groups = NULL;
	int ngroups = 0;
	long ngroups_max = sysconf(_SC_NGROUPS_MAX) + 1;

	if (getresuid(&ruid, &euid, &suid) < 0) {
		perror("Could not getresuid");
		exit(1);
	}
	if (getresgid(&rgid, &egid, &sgid) < 0) {
		perror("Could not getresgid");
		exit(1);
	}

	/* Get our supplementary groups */
	groups = (gid_t *) malloc(ngroups_max * sizeof(gid_t));
	if (groups == NULL) {
		printf("Could not allocate memory\n");
		exit(EXIT_FAILURE);
	}
	ngroups = getgroups(ngroups_max, groups);
	if (ngroups < 0) {
		perror("getgroups");
		free(groups);
		exit(1);
	}

	/* Display dropped privileges */
	printf("ruid=%d, euid=%d, suid=%d, ", ruid, euid, suid);
	printf("rgid=%d, egid=%d, sgid=%d, ", rgid, egid, sgid);
	printf("groups=");
	for (i = 0; i < ngroups; i++) {
		printf("%d", groups[i]);
		if (i < ngroups - 1)
			printf(",");
	}
	printf("\n");

	free(groups);

	return 0;
}

int main(int argc, char *argv[])
{
	char *user = "snap_daemon";
	printf("Before: ");
	display();

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

	printf("After: ");
	display();

	exit(0);
 fail:
	exit(EXIT_FAILURE);
}
