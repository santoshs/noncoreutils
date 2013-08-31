#ifndef _RANDCP_H_
#define _RANDCP_H_


#define SRC 0
#define DEST 1

#define TRUE 1
#define FALSE 0

#define REGERR_BUFSIZ 512

struct arguments {
	unsigned long limit, depth;
	unsigned icase:1;
	unsigned recursive:1;
	unsigned dry_run:1;
	unsigned echo:1;
	char *paths[2];
	char *pattern;
};

struct node {
	int type;
	char *name;
	struct node *parent;
};

struct progress_args {
	struct arguments *args;
	struct node **leaves;
};

#endif /* _RANDCP_H_ */
