#ifndef STEVENS_H
#define STEVENS_H

#include <unistd.h>

ssize_t Read(int fd, void *buf, size_t n_bytes);

ssize_t Write(int fd, void *buf, size_t n_bytes);

#endif
