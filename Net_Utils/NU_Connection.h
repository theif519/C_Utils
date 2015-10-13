#ifndef NU_CONNECTION_H
#define NU_CONNECTION_H

#include <MU_Logger.h>
#include <stdint.h>
#include <stdatomic.h>
#include <stdarg.h>
#include <stdbool.h>
#include <arpa/inet.h>


/**
 * Represents a socket file descriptor and all it's affiliated information, I.E the ones used
 * to initialize this structure (sockfd, ip_addr, port), as well as a meants to allow safe
 * concurrent access. This structure is better off created with it's abstraction managers/factory,
 * NU_Server and NU_Client.
 *
 * It also keeps track of the logger used to initialize it, which it logs any errors to. It also supports
 * concurrent R/W access for any send/receive calls. Any errors it encounters will be logged.
 */
typedef struct NU_Connection_t {
   /// Socket file descriptor associated with host.
   volatile int sockfd;
   /// The IP Address of the host connected to.
   char ip_addr[INET_ADDRSTRLEN];
   /// Port number that the host is bound to.
   unsigned int port;
   /// Read-Write lock to use for synchronization if initialized.
   pthread_rwlock_t *lock;
   /// A reusable buffer for each connection.
   volatile bool in_use;
   /// Logger associated with each connection.
   MU_Logger_t *logger;
} NU_Connection_t;

/**
 * Create a new instance that is not connected to an endpoint. 
 * @param init_locks If true, R/W lock will be used.
 * @param logger The logger to log any information to if not left NULL.
 * @return A new instance of NU_Connection_t, not connected.
 */
NU_Connection_t *NU_Connection_create(bool init_locks, MU_Logger_t *logger);

/**
 * If the conn is connected, it will attempt to send the contents in buffer up to buf_size
 * to it's connected end-point, until either it succeeds or timeout has ellapsed. Any flags passed
 * will be directly used in the call to send().
 * @param conn Connection instance.
 * @param buffer Buffer contents to send.
 * @param buf_size The size of the buffer, or rather how much of the buffer it will attempt to send.
 * @param timeout Max amount of time to attempt to send. Infinite if -1
 * @param flags Flags to pass to send().
 * @return Amount of bytes sent, 0 if it fails to send any (either timeout or socket error);
 */
size_t NU_Connection_send(NU_Connection_t *conn, const void *buffer, size_t buf_size, long long int timeout, int flags);

/**
 * If the conn is connected, it will attempt to fill it's buffer up to buf_size with what is received from
 * it's endpoint, until either it receives data on the connection or the timeout ellapses. Any flags passed will
 * be directly used in the call to recv().
 * @param conn Connection instance.
 * @param buffer Buffer to receive into.
 * @param buf_size The size of the buffer, or rather how much of the buffer it will attempt to fill.
 * @param timeout Max amount of time to attempt to receive. Infinite if -1
 * @param flags Flags to pass to recv().
 * @return Amount of bytes read, 0 if it fails to read any (either timeout or socket error or not connected);
 */
size_t NU_Connection_receive(NU_Connection_t *conn, void *buffer, size_t buf_size, long long int timeout, int flags);


/**
 * If the conn is connected to an endpoint, it will attempt to send all of the contents of the file until
 * succeeds or timeout ellapses. Any flags passed will be directly passed in the call to send().
 * @param conn Connection instance.
 * @param file File to send.
 * @param timeout Maximum timeout to attempt for. Infinite if -1
 * @param flags Flags to pass to send().
 * @return Amount of bytes sent, 0 if it fails to send any.
 */
size_t NU_Connection_send_file(NU_Connection_t *conn, FILE *file, long long int timeout, int flags);


/**
 * If the conn is connected to an endpoint, it will attempt to read as much information to the file as possible,
 * until either it receives data on the connection or the timeout ellapses. Any flags passed will be directly
 * used in the call to recv().
 * @param conn Connection instance
 * @param file File to receive into.
 * @param timeout Maximum timeout to attempt for. Infinite if -1
 * @param flags The flags to pass to recv().
 * @return Amount of bytes read, 0 if it fails to read any.
 */
size_t NU_Connection_receive_file(NU_Connection_t *conn, FILE *file, long long int timeout, int flags);

/**
 * A helper function meant to take an array of NU_Connection_t instance, and attempt to find one which is not currently
 * in use. If it does, it will take the already-connected sockfd and affiliated information and set it as it's current endpoint.
 * @param conn Connection instance.
 * @param size The size of the array.
 * @param sockfd The already-connected socket file descriptor.
 * @param port The port the sockfd is connected to.
 * @param ip_addr The IP address of the sockfd.
 * @param logger The new logger to log any and all errors to.
 * @return An unused connection fully initialized if possible, null on error or if there is no available to instance to reuse.
 */
NU_Connection_t *NU_Connection_reuse(NU_Connection_t **connections, size_t size, int sockfd, unsigned int port, const char *ip_addr, MU_Logger_t *logger);

/**
 * 
 * @param receivers
 * @param r_size
 * @param senders
 * @param s_size
 * @param timeout
 * @param logger
 * @return 
 */
int NU_Connection_select(NU_Connection_t ***receivers, size_t *r_size, NU_Connection_t ***senders, size_t *s_size, long long int timeout, MU_Logger_t *logger);

/**
 * 
 * @param conn
 * @return 
 */
int NU_Connection_get_sockfd(NU_Connection_t *conn);

/**
 * 
 * @param conn
 * @param sockfd
 * @return 
 */
bool NU_Connection_set_sockfd(NU_Connection_t *conn, int sockfd);

/**
 * 
 * @param conn
 * @param ip_addr
 * @return 
 */
const char *NU_Connection_get_ip_addr(NU_Connection_t *conn);

/**
 * 
 * @param conn
 * @param port
 * @return 
 */
bool NU_Connection_set_ip_addr(NU_Connection_t *conn, const char *ip_addr);

/**
 * 
 * @param conn
 * @param logger
 * @return 
 */
unsigned int NU_Connection_get_port(NU_Connection_t *conn);

/**
 * 
 * @param conn
 * @return 
 */
bool NU_Connection_set_port(NU_Connection_t *conn, unsigned int port);

/**
 * 
 * @param conn
 * @param sockfd
 * @param port
 * @param ip_addr
 * @param logger
 * @return 
 */
MU_Logger_t *NU_Connection_get_logger(NU_Connection_t *conn);

/**
 * 
 * @param conn
 * @return 
 */
bool NU_Connection_set_logger(NU_Connection_t *conn, MU_Logger_t *logger);

/**
 * 
 * @param conn
 * @return 
 */
bool NU_Connection_is_valid(NU_Connection_t *conn);

/**
 * 
 * @param conn
 * @return 
 */
bool NU_Connection_in_use(NU_Connection_t *conn);

/**
 * 
 * @param conn
 * @param sockfd
 * @param port
 * @param ip_addr
 * @param logger
 * @return 
 */
bool NU_Connection_init(NU_Connection_t *conn, int sockfd, unsigned int port, const char *ip_addr, MU_Logger_t *logger);

/**
 * 
 * @param conn
 * @return 
 */
bool NU_Connection_disconnect(NU_Connection_t *conn);

/**
 * 
 * @param conn
 * @return 
 */
bool NU_Connection_destroy(NU_Connection_t *conn);

#endif /* END NU_CONNECTION_H */
