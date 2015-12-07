#ifndef CFILEOPS_H
#define CFILEOPS_H

#include "options.h"
#include "cheader.h"

size_t SIZE(struct cfile *c);

/* Creates/opens new file - will overwrite existing */
struct cfile *NewCircularFile(struct options *o);
struct cfile *NewCircularMmap(struct options *o);

/* Opens existing file - for "appending" */
struct cfile *OpenCircularFile(struct options *o);
struct cfile *OpenCircularMmap(struct options *o);


int CloseCircularFile(struct cfile *c);
int CloseCircularMmap(struct cfile *c);

ssize_t WriteToCircularFile(struct cfile *c, char *buf, size_t size);
ssize_t WriteToCircularMmap(struct cfile *c, char *buf, size_t size);

int DumpHeader(struct options *o);

int DumpCircularFile(struct options *o);

#endif
