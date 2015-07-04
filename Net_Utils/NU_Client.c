#include <NU_Client.h>

#define MU_LOG_CLIENT(message, ...) MU_LOG_CUSTOM(logger, "CLIENT", message, ##__VA_ARGS__)

static MU_Logger_t *logger = NULL;

__attribute__((constructor)) static void init_logger(void){
	logger = malloc(sizeof(MU_Logger_t));
	if(!logger){
		MU_DEBUG("Unable to allocate memory for NU_Client's logger!!!");
		return;
	}
	MU_Logger_Init(logger, "NU_Client_Log.txt", "w", MU_ALL);
}

__attribute__((destructor)) static void destroy_logger(void){
	MU_Logger_Destroy(logger, 1);
	logger = NULL;
}

static int get_server_socket(const char *host, unsigned int port, unsigned int timeout){
	struct addrinfo hints, *results, *current;
	fd_set connect_set;
	struct timeval tv;
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	int retval, sockfd = 0, iteration = 0;
	char *port_str;
	asprintf(&port_str, "%u", port);
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if(retval = getaddrinfo(host, port_str, &hints, &results)){
		MU_LOG_WARNING(logger, "get_server_socket->getaddrinfo: %s\n", gai_strerror(retval));
		free(port_str);
		return 0;
	}
	free(port_str);
	// Loop through all potential results to find a valid connection.
	for(current = results; current; current = current->ai_next){
	    if((sockfd = TEMP_FAILURE_RETRY(socket(current->ai_family, current->ai_socktype, current->ai_protocol))) == -1){
	      MU_LOG_VERBOSE(logger, "get_server_socket->socket: \"%s\": Iteration #%d\n", strerror(errno), ++iteration);
	      continue;
	    }
	    MU_LOG_VERBOSE(logger, "get_server_socket: \"Received a socket!\": Iteration #%d\n", ++iteration);
	    FD_ZERO(&connect_set);
	    FD_SET(sockfd, &connect_set);
	    if((retval = TEMP_FAILURE_RETRY(select(sockfd + 1, &connect_set, NULL, NULL, &tv))) <= 0){
	    	if(!retval) MU_LOG_VERBOSE(logger, "get_server_socket->select: \"Timed out!\": Iteration: #%d\n", ++iteration);
	    	else MU_LOG_VERBOSE(logger, "get_server_socket->select: \"%s\": Iteration: #%d\n", strerror(errno), ++iteration);
	    	TEMP_FAILURE_RETRY(close(sockfd));
	    	continue;
	    }
	    if(TEMP_FAILURE_RETRY(connect(sockfd, current->ai_addr, current->ai_addrlen)) == -1){
	      MU_LOG_VERBOSE(logger, "get_server_socket->connect: \"%s\"; Iteration #%d\n", strerror(errno), ++iteration);
	      TEMP_FAILURE_RETRY(close(sockfd));
	      continue;
	    }
	    break;
	} 	
	/* 
		Note: If current is not NULL, then it succeeded in finding a socket, hence it is safe to return it.
		0 as a file descriptor represents stdin, which is impossible to have as an output fd.
	*/
	return current ? sockfd : 0;
}

NU_Client_t *NU_Client_create(void){
	NU_Client_t *client = calloc(1, sizeof(NU_Client_t));
	MU_ASSERT_RETURN(client, logger, "NU_Client_create->calloc: \"%s\"\n", strerror(errno));
	return client;
}

NU_Connection_t *NU_Client_connect(NU_Client_t *client, const char *ip_addr, unsigned int port, unsigned int timeout){
	if(!client || !ip_addr || !port) return NULL;
	// NU_Connection_reuse should return a new connection if there is none to find, else return a recycled one.
	NU_Connection_t *conn = NU_Connection_reuse(client->connections, logger);
	MU_ASSERT_RETURN(conn, logger, "NU_Client_connect->NU_Connection_reuse: \"Was unable to reuse or create a new connection!\"\n");
	if(!(connection->sockfd = get_server_socket(ip_addr, port, timeout))){
		MU_LOG_WARNING(logger, "NU_Client_connect->get_server_socket: \"Unable to form a connection!\"\n");
		return NULL;
	}
	conn->port = port;
	strcpy(conn->ip_addr, ip_addr);
	client->amount_of_servers++;
	MU_LOG_CLIENT("Connected to %s on port %u\n", ip_addr, port);
	return conn;
}

size_t NU_Client_send(NU_Client_t *client, NU_Connection_t *conn, const void *buffer, size_t buf_size, unsigned int timeout){
	if(!client || !conn || !conn->sockfd || !buffer || !buf_size) return 0;
	size_t result = NU_send_all(conn->sockfd, buffer, buf_size, timeout, logger);
	client->data.bytes_sent += result;
	client->data.messages_sent++;
	if(result != buf_size) MU_LOG_WARNING(logger, "Total Sent: %zu, Buffer Size: %zu\n", result, buf_size);
	return result;
}

const void *NU_Client_receive(NU_Client_t *client, NU_Connection_t *conn, size_t buf_size, unsigned int timeout){
	if(!client || !conn || !conn->sockfd || !buf_size) return NULL;
	NU_Buffer_resize(conn->buf, buf_size, logger);
	size_t result = NU_timed_receive(conn->sockfd, conn->bbuf, buf_size, timeout, logger);
	MU_LOG_VERBOSE(logger, "Total received: %zu, Buffer Size: %zu\n", result, buf_size);
	if(!result) return NULL;
	client->data.bytes_received += result;
	client->data.messages_received++;
	return (const void *)conn->bbuf->buffer;
}

size_t NU_Client_send_file(NU_Client_t *client, NU_Connection_t *conn, FILE *file, size_t buf_size, unsigned int timeout){
	if(!client || !conn || !conn->sockfd || !file || !buf_size) return 0;
	NUH_resize_buffer(server->bbuf, buffer_size, logger);
	size_t retval, total_sent = 0;
	char *str_retval;
	while((retval = fread(server->bbuf->buffer, 1, buffer_size, file)) > 0){
		if(NU_Client_send(client, server, server->bbuf->buffer, retval, timeout) == 0){
			MU_LOG_WARNING(logger, "client_send_file->client_send: \"%s\"\n", "Was unable to send all of message to %s!\n", conn->ip_addr);
			return total_sent;
		}
		total_sent += retval;
	}
	if(!total_sent) MU_LOG_WARNING(logger, "No data was sent to %s!\n", conn->ip_addr);
	else client->data.messages_sent++;
	client->data.bytes_sent += (size_t) total_sent;
	MU_LOG_VERBOSE(logger, "Sent file of total size %zu to %s!\n", total_sent, conn->ip_addr);
	return (size_t) total_sent;
}

size_t NU_Client_receive_to_file(NU_Client_t *client, NU_Connection_t *conn, FILE *file, size_t buf_size, unsigned int timeout){
	if(!server || !client || !server->sockfd || !file || !buffer_size) return 0;
	size_t result = NU_Connection_receive_to_file(conn, file, buf_size, timeout, logger);
	NUH_resize_buffer(server->bbuf, buffer_size, logger);
	size_t result, total_received = 0;
	while((result = NUH_timed_receive(server->sockfd, server->bbuf, buffer_size, timeout, logger)) > 0){
		fwrite(server->bbuf->buffer, 1, result, file);
		total_received += result;
	}
	client->data.bytes_received += total_received;
	client->data.messages_received++;
	MU_LOG_VERBOSE(logger, "Received file of total size %zu from %s!\n", total_received, conn->ip_addr);
	return total_received;
}

NU_Connection_t **NU_Client_select_receive(NU_Client_t *client, NU_Connection_t **connections, size_t *size, unsigned int timeout){
	if(!servers || !client || !size || !*size){
		*size = 0;
		return NULL;
	}
	fd_set receive_set;
	struct timeval tv;
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	FD_ZERO(&receive_set);
	int max_fd = 0, retval;
	size_t i = 0, new_size = 0;
	for(;i < *size; i++){
		NU_Connection_t *conn = servers[i];
		if(!server || !server->sockfd) continue;
		FD_SET(server->sockfd, &receive_set);
		new_size++;
		if(server->sockfd > max_fd) max_fd = server->sockfd;
	}
	if(!new_size) {
		*size = 0;
		return NULL;
	}
	if((retval = TEMP_FAILURE_RETRY(select(max_fd + 1, &receive_set, NULL, NULL, &tv))) <= 0){
		if(!retval) MU_LOG_INFO(logger, "select_receive->select: \"timeout\"\n");
		else MU_LOG_WARNING(logger, "select_receive->select: \"%s\"\n", strerror(errno));
		*size = 0;
		return NULL;
	}
	NU_Connection_t **ready_servers = malloc(sizeof(NU_Server_Socket_t *) * retval);
	new_size = 0;
	for(i = 0;i < *size;i++) if(FD_ISSET(servers[i]->sockfd, &receive_set)) ready_servers[new_size++] = servers[i];
	*size = new_size;
	return ready_servers;
}

NU_Connection_t **NU_Client_select_send(NU_Client_t *client, NU_Connection_t **connections, size_t *size, unsigned int timeout){
	if(!servers || !client || !size || !*size){
		*size = 0;
		return NULL;
	}
	fd_set send_set;
	struct timeval tv;
	tv.tv_sec = timeout;
	tv.tv_usec = 0;
	FD_ZERO(&send_set);
	int max_fd = 0, retval;
	size_t i = 0, new_size = 0;
	for(;i < *size; i++){
		NU_Connection_t *conn = servers[i];
		if(!server || !server->sockfd) continue;
		FD_SET(server->sockfd, &send_set);
		new_size++;
		if(server->sockfd > max_fd) max_fd = server->sockfd;
	}
	if(!new_size) {
		*size = 0;
		return NULL;
	}
	if((retval = TEMP_FAILURE_RETRY(select(max_fd + 1, NULL , &send_set, NULL, &tv))) <= 0){
		if(!retval) MU_LOG_VERBOSE(logger, "select_send->select: \"timeout\"\n");
		else MU_LOG_WARNING(logger, "select_send->select: \"%s\"\n", strerror(errno));
		*size = 0;
		return NULL;
	}
	NU_Connection_t **ready_servers = malloc(sizeof(NU_Server_Socket_t *) * retval);
	new_size = 0;
	for(i = 0;i < *size;i++) if(FD_ISSET(servers[i]->sockfd, &send_set)) ready_servers[new_size++] = servers[i];
	*size = new_size;
	return ready_servers;
}

char *NU_Client_about(NU_Client_t *client){
	return NULL;
}

int NU_Client_shutdown(NU_Client_t *client, const char *msg){
	return 0;
}

int NU_Client_destroy(NU_Client_t *client, const char *msg){
	return 0;
}