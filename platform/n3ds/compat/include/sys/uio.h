// Nintendo 3DS compatibility: scatter/gather I/O vectors (libctru's sockets
// take plain buffers; the engine only declares the type).
#ifndef N3DS_COMPAT_SYS_UIO_H
#define N3DS_COMPAT_SYS_UIO_H
#include <sys/types.h>
struct iovec
{
	void *iov_base;
	size_t iov_len;
};
#endif
