#include <unistd.h>
#include <errno.h>
#include <stddef.h>

// FIXME: this stops visual studio code complaining about size_t not being defined
// proper fix is to probably set compiler flags
#if !defined(_SIZE_T)
typedef long unsigned int size_t;
#endif

ssize_t
read_line(int fd, void *buffer, size_t n);