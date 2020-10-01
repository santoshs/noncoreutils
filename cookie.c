/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/*
 * cookie.c
 * Display random quote cookie
 *
 * Copyright (C) Santosh 2010 <santosh@fossix.org>
 *
 * cookie is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * cookie is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <unistd.h>
#include <argp.h>

#define INRANGE(a, b, c) (((a) >= (b)) && ((a) <= (c)))
#define DEFAULT_END_COLUMN 70

enum {
	YEAR = 1,
	MONTH,
	WEEK,
	DAY,
	HOUR,
	MINUTE,
	SECOND,
};

struct arguments;
static char *get_cookie(FILE *, struct arguments *, char *);
void version_hook(FILE *, struct argp_state *);
void (*argp_program_version_hook) (FILE *, struct argp_state *) = version_hook;
static error_t parser(int, char *, struct argp_state *);
static char *prgname;

const char *argp_program_version = "cookie 2.0";
const char *argp_program_bug_address = "santosh@fossix.org";
const char *argp_doc = "Displays random quotes while login or at command.";
static char doc[] = "cookie -- a program to print a random quote from a file.";
const char *copyright =
    "Copyright (c) 2009-2012 Santosh Sivaraj.\n"
    "This software comes with NO WARRANTY, to the extent permitted by law."
    "\nYou may redistribute copies of this software under the terms of the"
    "\nGNU General Public License.";

struct arguments {
	char *file;
	int hz;
	int index;
	int wait;
	int column;
	int time;
};

struct argp_option options[] = {
	{"file", 'f', "FILE", 0, "File from where to read the strings. "
	 "Default: $HOME/.quotes"},
	{"frequency", 'q', "HZ", 0, "The Frequency of updation - "
	 "1: Year 2: Month 3: Week 4: Day 5: Hour 6: Minute, 7: Seconds"},
	{"absolute", 'a', "ABSOLUTE", 0,
	 "The absolute position of line in file"},
	{"wait", 'w', NULL, 0, "Wait few seconds before quitting."
	 "seconds autocalculated based on the number of words"},
	{"seconds", 't', "TIME", 0, "Number of seconds to wait"},
	{"column", 'c', "COLUMN", 0,
	 "COLUMN where line text should wrap around. Default: 70"},
	{0}
};

struct argp argp = { options, parser, 0, doc };

static error_t parser(int key, char *arg, struct argp_state *state)
{
	struct arguments *args = state->input;
	switch (key) {
	case 'f':
		args->file = arg;
		break;
	case 'q':
		args->hz = atoi(arg);
		if (!INRANGE(args->hz, 1, 7)) {
			fprintf(stderr, "%s: Invalid range for option "
				"frequency. Enter between 1 and 7\n", prgname);
			return EINVAL;
		}
		break;
	case 'a':
		args->index = atoi(arg);	/* TODO: Error check */
		break;
	case 'w':
		args->wait = 1;
		args->time = 0;
		break;

	case 't':
		args->time = atoi(arg);	/* TODO: Error check */
		args->wait = 1;
		break;
	case 'c':
		args->column = atoi(arg);	/* TODO: Error check */
		break;
	default:
		return ARGP_ERR_UNKNOWN;
		break;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	char buf[BUFSIZ];
	FILE *f;
	struct arguments args = { 0, 0, -1, 0, DEFAULT_END_COLUMN, 0 };

	prgname = argv[0];

	argp_parse(&argp, argc, argv, 0, 0, &args);

	if (args.file) {
		strncpy(buf, args.file, BUFSIZ);
	} else {
		strcpy(buf, getenv("HOME"));
		strncat(buf, "/.quotes", BUFSIZ - 1);
	}
	f = fopen(buf, "r");
	if (!f) {
		perror(prgname);
		return -1;
	}

	if (get_cookie(f, &args, buf)) {
		printf("%s", buf);

		if (args.wait)
			sleep(args.time);
	}

	return 0;
}

int get_week_of_year(time_t * t)
{
	return 0;
}

/*
 * remove new lines
 */
void del_slashn(char *str)
{
	char *s = str;
	int len = strlen(str), i = 0;

	while (*s) {
		if (*s == '\n')
			memmove(s, s + 1, len - i);
		i++, s++;
	}
}

/*
 * Whitespace cleanup
 */
char *strip(char *str)
{
	register char *s, *t;

	for (s = str; isspace(*s); s++) ;

	if (*s == 0)
		return (s);

	t = s + strlen(s) - 1;
	while (t > s && isspace(*t))
		t--;
	*++t = '\0';

	del_slashn(s);

	return s;
}

static char *get_cookie(FILE * f, struct arguments *args, char *buf)
{
	char *str = NULL;
	int i = 0, j = 0, c, words = 0;
	size_t sz = 0;
	ssize_t l = 0;
	time_t t;

	if (f == NULL)
		return NULL;

	while ((l = getline(&str, &sz, f)) > 0) {
		if (l == 1)
			continue;
		i++;
	}

	if (i == 0) {
		return NULL;		/* no lines in file */
	}

	if (args->index < 0) {
		int seed;
		struct timeval tv;
		struct timezone tz;

		gettimeofday(&tv, &tz);
		seed = tv.tv_usec;
		t = tv.tv_sec;

		if (args->hz) {
			switch (args->hz) {
			case YEAR:
				seed = localtime(&t)->tm_year;
				break;
			case MONTH:
				seed = localtime(&t)->tm_mon;
				break;
			case WEEK:
				seed = get_week_of_year(&t);
				break;
			case DAY:
				seed = localtime(&t)->tm_mday;
				break;
			case HOUR:
				seed = localtime(&t)->tm_hour;
				break;
			case MINUTE:
				seed = localtime(&t)->tm_min;
				break;
			case SECOND:
				seed = localtime(&t)->tm_sec;
				break;
			default:
				break;
			}
		}
		srandom((unsigned)(seed));
		j = random();
		j = j % (i - 1);
	} else {
		j = args->index;
		if (j > i)
			return NULL;
	}

	i = 0;
	rewind(f);
	while ((l = getline(&str, &sz, f))) {
		if (l == 1)
			continue;
		if (i == j)
			break;
		i++;
	}

	if (str) {
		char *ptr, *s, *author = NULL;
		int size = 0;

		s = str;

		s = strip(s);
		ptr = strtok(s, "|");
		author = strtok(NULL, "|");

		s = ptr;
		ptr = strtok(s, " ");
		c = 0;
		while (ptr) {
			words++;
			c += strlen(ptr) + 1;
			if (c > args->column) {
				size += sprintf(buf + size, "\n");
				c = 0;
			}

			ptr = strtok(NULL, " ");
			if (ptr)
				size += sprintf(buf + size, "%s ", ptr);
		}
		if (author)
			author = strip(author);

		if (!author || !*author)
			author = "Anon";

		words++;

		/* TODO Calculate time based on average reading speed of 250
		 * WPM */
		if (!args->time)
			args->time = words / 2;

		sprintf(buf + size, "\n%*s %s\n",
			(int)(args->column - strlen(author) - 3), "--", author);
	}

	free(str);

	return buf;
}

void version_hook(FILE * stream, struct argp_state *state)
{
	fprintf(stream, "%s\n", argp_program_version);
	fprintf(stream, "%s\n", copyright);
	return;
}
