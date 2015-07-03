// NU_* is the Net_Utils namespace
//NUH_* is the Net_Utils_Helper namespace, used for helper functions Client, Server and later HTTP can use.

// Basic data structures

typedef struct {
   size_t messages_sent;
   /// Amount of messages received.
   size_t messages_received;
   /// Total amount of data sent.
   size_t bytes_sent;
   /// Total amount of data received.
   size_t bytes_received;
} NU_Collective_Data_t;

// The bounded buffer is my wanna-be ring/circular buffer, expect linear. Each time I use it I just start at the beginning of the buffer.
typedef struct {
   /// Container for buffer.
   char *buffer;
   /// Current size of buffer.
   size_t size;
} NU_Bounded_Buffer_t;

// This is a reusable data structure, which gets reused when the client disconnects from a host, it gets recycled if available.
typedef struct NU_Server_Socket_t {
   /// Socket file descriptor associated with host.
   volatile int sockfd;
   /// The IP Address of the host connected to.
   char ip_addr[INET_ADDRSTRLEN];
   /// Port number that the host is bound to.
   unsigned int port;
   /// A reusable buffer for each connection.
   NU_Bounded_Buffer_t *bbuf;
   /// The next server socket in the list.
   struct NU_Server_Socket_t *next;
} NU_Server_Socket_t;

/* The reason for making such complex data structures to represent a simple socket is that it follows these philosophies.
1) Reusability: A client may connect to as many hosts as it wants, and when it disconnects from one, it is recycled.
2) Information: A server or even a client should maintain the information needed to maintain it.
3) Thread-Safety[Not Implemented]: A structure allows the client to keep track of it's own locks and atomic data.*/
typedef struct {
   /// Socket associated with this server.
   NU_Server_Socket_t *servers;
   /// Amount of servers currently connected to.
   size_t amount_of_servers;
   /// Keeps track of data used.
   NU_Collective_Data_t data;
} NU_Client_t;

// Function definitions below.

// Essentially attempts to retrieve as much as possible until either the sockfd disconnects, or a timeout ellapses.
// This function uses a sockfd and bbuf directly because both client and server use it.
size_t NUH_timed_receive(int sockfd, NU_Bounded_Buffer_t *bbuf, size_t buffer_size, unsigned int timeout, MU_Logger_t *logger){
   int retval;
   struct timeval tv;
   fd_set can_receive;
   tv.tv_sec = timeout;
   tv.tv_usec = 0;
   FD_ZERO(&can_receive);
   FD_SET(sockfd, &can_receive);
   if((retval = TEMP_FAILURE_RETRY(select(sockfd + 1, &can_receive, NULL, NULL, &tv))) <= 0){
      if(!retval) MU_LOG_INFO(logger, "timed_receive->select: \"timed out\"\n");
      else MU_LOG_ERROR(logger, "timed_receive->select: \"%s\"", strerror(errno));
      return 0;
   }
   if((retval = TEMP_FAILURE_RETRY(recv(sockfd, bbuf->buffer, buffer_size, 0))) <= 0){
      if(!retval) MU_LOG_INFO(logger, "receive_all->recv: \"disconnected from the stream\"\n");
      else MU_LOG_ERROR(logger, "timed_receive->recv: \"%s\"\n", strerror(errno));
      return 0;
   }
   return retval;
}

// Function that basically reads as much from a socket, into a buffer, then writes to file.
size_t NU_Client_receive_to_file(NU_Client_t *client, NU_Server_Socket_t *server, FILE *file, size_t buffer_size, unsigned int is_binary, unsigned int timeout){
	if(!server || !client || !server->sockfd || !file || !buffer_size) return 0;
	NUH_resize_buffer(server->bbuf, buffer_size, logger);
	size_t result, total_received = 0;
	while((result = NUH_timed_receive(server->sockfd, server->bbuf, buffer_size, timeout, logger)) > 0){
		if(is_binary) fwrite(server->bbuf->buffer, 1, result, file);
		else fprintf(file, "%.*s", (int)result, server->bbuf->buffer);
		total_received += result;
	}
	client->data.bytes_received += total_received;
	client->data.messages_received++;
	MU_LOG_VERBOSE(logger, "Received file of total size %zu from server!\n", total_received);
	return total_received;
}

// Basic function which sends all of a message, or until an error or timeout. 
size_t NUH_send_all(int sockfd, const char *message, size_t msg_size, unsigned int timeout, MU_Logger_t *logger){
   size_t buffer_size = msg_size, total_sent = 0, data_left = msg_size;
   int retval;
   struct timeval tv;
   fd_set can_send, can_send_copy;
   tv.tv_sec = timeout;
   tv.tv_usec = 0;
   FD_ZERO(&can_send);
   FD_SET(sockfd, &can_send);
   while(buffer_size > total_sent){
      can_send_copy = can_send;
      // Restart timeout.
      tv.tv_sec = timeout;
      if((retval = TEMP_FAILURE_RETRY(select(sockfd+1, NULL, &can_send_copy, NULL, &tv))) <= 0){
         if(!retval) MU_LOG_INFO(logger, "send_all->select: \"timed out\"\n");
         else MU_LOG_ERROR(logger, "send_all->select: \"%s\"", strerror(errno));
         break;
      }
      if((retval = TEMP_FAILURE_RETRY(send(sockfd, message + total_sent, data_left, 0))) <= 0){
         if(!retval) MU_LOG_INFO(logger, "send_all->send: \"disconnected from the stream\"\n");
         else MU_LOG_ERROR(logger, "send_all->send: \"%s\"\n", strerror(errno));
         break;
      }
      total_sent += retval;
      data_left -= retval;
   }
   return total_sent;
}

// The function that sends general messages to a host. Does nothing special.
size_t NU_Client_send(NU_Client_t *client, NU_Server_Socket_t *server, const char *message, size_t msg_size, unsigned int timeout){
	size_t result = NUH_send_all(server->sockfd, message, msg_size, timeout, logger);
	client->data.bytes_sent += result;
	client->data.messages_sent++;
	if(result != msg_size) MU_LOG_WARNING(logger, "Was unable to send all data to server!Total Sent: %zu, Message Size: %zu\n", result, msg_size);
	return result;
}

// The function that sends files, below, only loops until no more data can be read from the file into the buffer, or until a timeout ellapses.
size_t NU_Client_send_file(NU_Client_t *client, NU_Server_Socket_t *server, FILE *file, size_t buffer_size, unsigned int is_binary, unsigned int timeout){
	if(!server || !client || !server->sockfd || !file || !buffer_size) return 0;
	NUH_resize_buffer(server->bbuf, buffer_size, logger); // Assume it works
	size_t retval, total_sent = 0;
	char *str_retval;
	if(is_binary){ // Since text files and binary files are sent differently they're handled conditionally below.
	  while((retval = fread(server->bbuf->buffer, 1, buffer_size, file)) > 0){
		  if(NU_Client_send(client, server, server->bbuf->buffer, retval, timeout) == 0){
			  MU_LOG_WARNING(logger, "client_send_file->client_send: \"%s\"\n", "Was unable to send all of message to server!\n");
			  return total_sent;
		  }
		  total_sent += retval;
	  }
	} else {
	    while((str_retval = fgets(server->bbuf->buffer, buffer_size, file)) != NULL){
	      if(!NU_Client_send(client, server, server->bbuf->buffer, strlen(str_retval), timeout)){
		MU_LOG_WARNING(logger, "client_send_file->client_send: \"%s\"\n", "Was unable to send all of message to server!\n");
		return total_sent;
	      }
	      total_sent += strlen(str_retval);
	    }
	}
	if(!total_sent) MU_LOG_WARNING(logger, "No data was sent to server!\n");
	else client->data.messages_sent++;
	client->data.bytes_sent += (size_t) total_sent;
	MU_LOG_VERBOSE(logger, "Sent file of total size %zu to server!\n", total_sent);
	return (size_t) total_sent;
}