#ifndef LIBWBFS_OS_H
#define LIBWBFS_OS_H

// this file abstract the os integration
// libwbfs_glue.h for segher tools env.

// standard u8, u32 and co types, + fatal
#include "tools.h"
#include <stdio.h>

#ifdef WIN32
#define wbfs_fatal non_fatal
#define wbfs_error non_fatal
#define wbfs_warning non_fatal
#else
#define wbfs_fatal non_fatal
#define wbfs_error non_fatal
#define wbfs_warning non_fatal
#endif

#include <stdlib.h>
#define wbfs_malloc(x) malloc(x)
#define wbfs_free(x) free(x)

// alloc memory space suitable for disk io
#define wbfs_ioalloc(x) malloc(x)
#define wbfs_iofree(x) free(x)

#ifdef WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

// endianess tools
#define wbfs_ntohl(x) ntohl(x)
#define wbfs_ntohs(x) ntohs(x)
#define wbfs_htonl(x) htonl(x)
#define wbfs_htons(x) htons(x)

#include <string.h>
#define wbfs_memcmp(x,y,z) memcmp(x,y,z)
#define wbfs_memcpy(x,y,z) memcpy(x,y,z)
#define wbfs_memset(x,y,z) memset(x,y,z)

#endif
