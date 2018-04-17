#ifndef _PDCLIB_MASYSTD_H
#define _PDCLIB_MASYSTD_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned short fd_t;

/* Generic system call wrapper */
long long syscall( int number, ... );

void cease( int exitnum );
int sysping( int test );
int prove();
void * obtain( unsigned pages, unsigned flags );
int inscribe( fd_t fd, const char *data, unsigned sz, unsigned flags );

#ifdef __cplusplus
}
#endif

#endif /* end of include guard: _PDCLIB_MASYSTD_H */
