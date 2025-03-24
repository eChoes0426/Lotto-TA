#ifdef __OPTEE__

#ifndef ENOSYS
#define ENOSYS 38
#endif


#ifndef __ssize_t_defined
typedef long ssize_t;
#define __ssize_t_defined
#endif

#ifndef __off_t_defined
typedef long off_t;
#define __off_t_defined
#endif

#include <tee_internal_api.h>
#include <stdlib.h>
#include <string.h>

int errno;


int *__errno_location(void)
{
    return &errno;
}

void __assert_fail(const char *assertion,
                   const char *file,
                   unsigned int line,
                   const char *function)
{
    DMSG("Assertion failed: %s, function %s, file %s, line %u\n",
         assertion, function, file, line);
    TEE_Panic(0);
}

int mlock(const void *addr, size_t len)
{
    (void) addr; (void) len;
    return 0;
}
int munlock(const void *addr, size_t len)
{
    (void) addr; (void) len;
    return 0;
}
int mprotect(void *addr, size_t len, int prot)
{
    (void) addr; (void) len; (void) prot;
    return 0;
}
int madvise(void *addr, size_t length, int advice)
{
    (void) addr; (void) length; (void) advice;
    return 0;
}

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    (void) addr; (void) prot; (void) flags; (void) fd; (void) offset;
    return TEE_Malloc(length, 0);
}
int munmap(void *addr, size_t length)
{
    (void) length;
    TEE_Free(addr);
    return 0;
}

int open(const char *pathname, int flags, ...)
{
    (void) pathname; (void) flags;
    errno = ENOSYS;
    return -1;
}
int close(int fd)
{
    (void) fd;
    errno = ENOSYS;
    return -1;
}
ssize_t read(int fd, void *buf, size_t count)
{
    (void) fd; (void) buf; (void) count;
    errno = ENOSYS;
    return -1;
}
int poll(void *fds, unsigned int nfds, int timeout)
{
    (void) fds; (void) nfds; (void) timeout;
    errno = ENOSYS;
    return -1;
}
int fstat(int fd, void *statbuf)
{
    (void) fd; (void) statbuf;
    errno = ENOSYS;
    return -1;
}

long syscall(long number, ...)
{
    (void) number;
    errno = ENOSYS;
    return -1;
}

long sysconf(int name)
{
    if (name == 30) {
        return 4096;
    }
    errno = ENOSYS;
    return -1;
}

void explicit_bzero(void *p, size_t s)
{
    memset(p, 0, s);
}

int raise(int sig)
{
    (void)sig;
    TEE_Panic(0);
    return -1;
}

int fcntl(int fd, int cmd, ...)
{
    (void)fd; (void)cmd;
    errno = ENOSYS;
    return -1;
}

#endif
