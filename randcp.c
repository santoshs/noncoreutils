/*
 * randcp.c - A random file copy program
 *
 * Copyright (C) 2013  Santosh Sivaraj <santosh@fossix.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301, USA
 */

#define _ISOC99_SOURCE
#define _BSD_SOURCE		/* for d_type constants */

#include <stdio.h>
#include <stdlib.h>
#include <argp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/time.h>
#include <string.h>
#include <fcntl.h>
#include <err.h>

#define SRC 0
#define DEST 1

const char *argp_program_version = "randcp 0.1";
const char *argp_program_bug_address = "<santosh@fossix.org>";
static char doc[] = "randcp -- Copy random files";
static char args_doc[] =
	"SOURCE DEST";

static struct argp_option options[] = {
	{"limit", 'l', "LIMIT", 0, "Limit number of copied files"},
};

struct arguments {
	unsigned limit;
	char *paths[2];
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
	struct arguments *arguments = state->input;

	switch (key) {
	case 'l':
		arguments->limit = strtoul(arg, NULL, 10);
		if (errno)
			err(errno, "Invalid limit");
		break;

	case ARGP_KEY_ARG:
		if (state->arg_num >= 2)
			argp_usage(state);

		arguments->paths[state->arg_num] = arg;

		break;

	case ARGP_KEY_END:
		if (state->arg_num < 2)
			argp_usage(state);

		break;

	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

int random_sort(const struct dirent **e1, const struct dirent **e2)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);

	return random() % 2;
}

int cp(const char * const spath, const char * const dpath)
{
	char buf[1024];
	int s, d, l;
	struct stat st;

	if (stat(spath, &st)) {
		warn("%s", spath);
		return 1;
	}

	if ((s = open(spath, O_RDONLY)) < 0) {
		warn("%s", spath);
		return 1;
	}

	if ((d = open(dpath, O_CREAT|O_EXCL|O_WRONLY, st.st_mode)) < 0) {
		warn("%s: This shouldn't happen, its a BUG!", dpath);
		close(s);
		return 1;
	}

	while ((l = read(s, buf, 512)) > 0) {
		if (l < 0) {
			warn("%s", spath);
			goto cp_err;
		}
		l = write(d, buf, l);
		if (l < 0) {
			warn("%s", dpath);
			goto cp_err;
		}
	}

cp_err:
	close(s);
	close(d);
	if (l < 0) {
		unlink(dpath);
		return 2;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	struct arguments args;
	DIR *src, *dst;
	struct dirent **list;
	int n, cstatus;
	unsigned limit;
	char *spath, *dpath;
	struct stat status;
	pid_t pid;

	args.limit = 1;
	args.paths[SRC] = NULL;
	args.paths[DEST] = NULL;

	argp_parse(&argp, argc, argv, 0, 0, &args);

	src = opendir(args.paths[SRC]);
	if (!src)
		err(errno, "%s", args.paths[SRC]);

	dst = opendir(args.paths[DEST]);
	if (!dst)
		err(errno, "%s", args.paths[DEST]);

	/* Both the arguments are directories, lets close the src, since
	 * scandir will open it */
	closedir(src);

	dpath = malloc(PATH_MAX + 1); /* should we take care of \0? */
	if (!dpath)
		err(errno, "Memory");

	spath = malloc(PATH_MAX + 1);
	if (!spath)
		err(errno, "Memory");

	/* remove trailing '/' if any */
	if (args.paths[SRC][strlen(args.paths[SRC]) - 1] == '/')
		args.paths[SRC][strlen(args.paths[SRC]) - 1] = '\0';

	if (args.paths[DEST][strlen(args.paths[DEST]) - 1] == '/')
		args.paths[DEST][strlen(args.paths[DEST]) - 1] = '\0';

	/* we define the filer inline to access the arguments variable for
	 * paths */
	int dir_filter(const struct dirent *ent)
	{
	#ifdef _DIRENT_HAVE_D_TYPE
		if (ent->d_type == DT_REG || ent->d_type == DT_LNK)
			return 1;
	#else

		/* have to use stat!! */
		sprintf(spath, "%s/%s", args.paths[SRC], ent->d_name);
		if (stat(spath, &status) < 0) {
			warn("%s", ent->d_name);
			return 0;
		}

		if (S_ISREG(status.st_mode))
			return 1;
	#endif
		return 0;
	}

	/* Now we get a list of the files in the src directory */
	n = scandir(args.paths[SRC], &list, dir_filter, random_sort);
	if (n < 0) {
		err(errno, "%s: Error reading directory", args.paths[SRC]);
	}

	limit = args.limit;

	if ((pid = fork()) == 0) {
		char spinner[] = {'\\', '|', '/', '-'};
		int i = 0;
		while (1) {
			printf("\r%d [%c]", limit, spinner[i]);
			fflush(stdout);
			i = i >= 3?0:i+1;
			sleep(1);
		}
	} else {
		while (n-- && limit) {
			sprintf(spath, "%s/%s", args.paths[SRC], list[n]->d_name);
			sprintf(dpath, "%s/%s", args.paths[DEST], list[n]->d_name);
			if (!stat(dpath, &status))
				/* file exists, don't overwrite */
				continue;

			if (!cp(spath, dpath)) {
				limit--;
				free(list[n]);
			}
		}
		free(list);

		kill(pid, 9);
		wait(&cstatus);
		printf("\rCopied %d files\n", limit);
	}

	return 0;
}
