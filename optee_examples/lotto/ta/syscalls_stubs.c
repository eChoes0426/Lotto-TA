#ifdef __OPTEE__

/* Define ENOSYS if not defined */
#ifndef ENOSYS
#define ENOSYS 38
#endif

/* Define ssize_t if not defined.
 * Typically, ssize_t is a signed version of size_t. For our purposes, we can define it as long.
 */
#ifndef __ssize_t_defined
typedef long ssize_t;
#define __ssize_t_defined
#endif

/* Define off_t if not defined.
 * This type represents file offsets; we can define it as long.
 */
#ifndef __off_t_defined
typedef long off_t;
#define __off_t_defined
#endif

#include <tee_internal_api.h>  /* Provides TEE_Panic(), etc. */
#include <stdlib.h>
#include <string.h>

int errno; // OP-TEE doesn’t have a global errno by default

/* Stub for __errno_location */
int *__errno_location(void)
{
    // Return address of our static int errno
    return &errno;
}

/* Stub for __assert_fail */
void __assert_fail(const char *assertion,
                   const char *file,
                   unsigned int line,
                   const char *function)
{
    // Possibly log something here
    DMSG("Assertion failed: %s, function %s, file %s, line %u\n",
         assertion, function, file, line);
    // Then panic, because an assertion means something went very wrong
    TEE_Panic(0);
}

/* Stub for mlock, munlock, mprotect, etc. */
int mlock(const void *addr, size_t len)
{
    (void) addr; (void) len;
    return 0; // no-op
}
int munlock(const void *addr, size_t len)
{
    (void) addr; (void) len;
    return 0; // no-op
}
int mprotect(void *addr, size_t len, int prot)
{
    (void) addr; (void) len; (void) prot;
    return 0; // no-op
}
int madvise(void *addr, size_t length, int advice)
{
    (void) addr; (void) length; (void) advice;
    return 0; // no-op
}

/* Stub for mmap/munmap */
void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
    (void) addr; (void) prot; (void) flags; (void) fd; (void) offset;
    // Possibly just do a TEE_Malloc or standard malloc
    return TEE_Malloc(length, 0);
}
int munmap(void *addr, size_t length)
{
    (void) length;
    TEE_Free(addr);
    return 0;
}

/* Stub for open, close, read, poll, fstat, etc. if randombytes_sysrandom uses them */
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

/* Stub for syscall() if randombytes calls getrandom() via syscall */
long syscall(long number, ...)
{
    (void) number;
    errno = ENOSYS;
    return -1;
}

/* Stub for sysconf */
long sysconf(int name)
{
    // If libsodium calls sysconf(_SC_PAGESIZE), return 4096 or something
    if (name == 30) { // on some systems, _SC_PAGESIZE = 30
        return 4096;
    }
    errno = ENOSYS;
    return -1;
}

void explicit_bzero(void *p, size_t s)
{
    // Minimal version: just do a normal memzero.
    // For extra security, consider volatile operations, etc.
    memset(p, 0, s);
}

int raise(int sig)
{
    (void)sig;
    // Possibly log or panic
    TEE_Panic(0);
    return -1; // not reached
}

int fcntl(int fd, int cmd, ...)
{
    (void)fd; (void)cmd;
    errno = ENOSYS;
    return -1;
}

#endif /* __OPTEE__ */
