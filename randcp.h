#ifndef _RANDCP_H_
#define _RANDCP_H_


#define SRC 0
#define DEST 1

#define TRUE 1
#define FALSE 0

#define REGERR_BUFSIZ 512

struct arguments {
	unsigned long limit;
	unsigned icase;
	char *paths[2];
	char *pattern;
};

struct node {
	int type;
	char *name;
	struct node *parent;
};

#endif /* _RANDCP_H_ */
