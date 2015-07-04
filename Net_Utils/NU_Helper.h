#ifndef NET_UTILS_HELPER_H
#define NET_UTILS_HELPER_H

/* 
* As many system calls may fail due to an async signal, whether it be select, send, recv, or even close,
* a system call that fails due to EINTR should be restarted, which is why I use TEMP_FAILURE_RETRY glibc macro.
*/
#define _GNU_SOURCE 1

#include <unistd.h>
#include <MU_Logger.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>

/* Client-Server general data structures below */

typedef struct {
   /// Amount of messages sent.
   size_t messages_sent;
   /// Amount of messages received.
   size_t messages_received;
   /// Total amount of data sent.
   size_t bytes_sent;
   /// Total amount of data received.
   size_t bytes_received;
} NU_Collective_Data_t;

typedef struct {
   /// Container for buffer.
   void *buffer;
   /// Current size of buffer.
   size_t size;
} NU_Buffer_t;

/// Helper to make determining buffer and file sizes a lot easier.
typedef enum {
   /// Represents a byte.
   NU_BYTE = 1,
   /// Represents a kilobyte.
   NU_KILOBYTE = 1024,
   /// Represents a megabyte
   NU_MEGABYTE = 1,048,576,
   /// Represents a gigabyte
   NU_GIGABYTE = 1,073,741,824
} NU_Data_Size_t;

int NU_Buffer_resize(NU_Buffer_t *bbuf, size_t new_size, MU_Logger_t *logger);

size_t NU_timed_receive(int sockfd, NU_Buffer_t *buf, size_t buf_size, unsigned int timeout, MU_Logger_t *logger);

int NU_timed_accept(int sockfd, char **ip_addr, unsigned int timeout, MU_Logger_t *logger);

// Implement
char *NU_Collective_Data_to_string(NU_Collective_Data_t data);

int NU_is_selected(int flags, int mask);

#endif /* END NET_UTILS_HELPER_H */