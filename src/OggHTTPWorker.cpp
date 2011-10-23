#include "OggHTTPWorker.h"
#include "OggHTTPClient.h"

OggHTTPWorker::OggHTTPWorker() {
}

OggHTTPWorker::~OggHTTPWorker() {
}

void OggHTTPWorker::setup(int nPort) {
	port = nPort;
}

void OggHTTPWorker::run() {
	// create server socket.
	server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(server_socket == -1) {
		printf("Can't create server socket\n");
	}	
	
	// make sure to resuse the address.
	int result = 0;
	int val;
	//SO_NOSIGPIPE
	result = setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR , &val, sizeof(val));
	if(result < 0) {
		printf("Cannot reuse address\n");
		exit(1);
	}

	// bind.	
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);
	result = bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
	if(result != 0) {
		printf("Cannot bind...\n");
		exit(1);
	}
				
	// listen
	result = listen(server_socket, 15);
	if(result < 0) {
		printf("Cannot set listen mode to socket\n");
		close(server_socket);
		exit(1);
	}
	
	// some sets
	fd_set socket_readset, test_set;
	FD_ZERO(&socket_readset);
	FD_SET(server_socket, &socket_readset);
 
	
	printf("Start listening at %d\n", port);
	
	int highest_fd = 0;
	while(true) {
		// get notified by readset.
		test_set = socket_readset;
		result = select(FD_SETSIZE, &test_set, NULL, NULL, NULL);
		if(result < 1) {
			printf("Server error\n");
			exit(1);
		}
		//++++++++++++++
		for(int i = 0; i < FD_SETSIZE; ++i) {
			//printf("! sock:%d\n", i);
			if(FD_ISSET(i, &test_set)) {
				if(i == server_socket) {
					// listen for a new connection.
					struct sockaddr_in client_addr = {0};
					int child_socket = 0; 
					int client_addr_len = sizeof(client_addr);
				
					child_socket = accept(server_socket, (struct sockaddr*)&client_addr, (socklen_t*) &client_addr_len);
					int n = 1;
					setsockopt(child_socket, SOL_SOCKET, SO_NOSIGPIPE, &n, sizeof(n));
					printf("! main no sigpipe: %d\n", n);
					
					printf("! main loop: new socket: %d\n", child_socket);
			
					if(child_socket == -1) {
						printf("! main loop: cannot accept connections\n");
						close(server_socket);
						exit(1);
					}
					printf("..");
					// store our new client.
					FD_SET(child_socket, &socket_readset);
					OggHTTPClient* client_connection = new OggHTTPClient(*this, child_socket);
					
					mutex.lock();
						clients.insert(std::pair<int, OggHTTPClient*>(child_socket, client_connection));
					mutex.unlock();
					//client_connection->sendHTTPResponse();
					client_connection->header_page = header_page; 
				}	
				else {
					//printf("# .. ");
					//mutex.lock();
					map<int, OggHTTPClient*>::iterator it = clients.find(i);
					if(it != clients.end()) {
						it->second->readFromSocket();
					}
					//mutex.unlock();
				}
			}
		}
	}
	close(server_socket);
}

void OggHTTPWorker::removeClient(OggHTTPClient* client) {
	//mutex.lock();
	printf("! remove client disconnected with socket: %d\n", client->sock);
	map<int, OggHTTPClient*>::iterator it = clients.find(client->sock);
	if(it == clients.end()) {
		printf("! remove client error: no client found\n");		
	}	
	else {
		printf("! remove client success: removed client - %d\n", client->sock);
		//close(client->sock);
		//clients.erase(it);
		//delete client;
		
	}
	//mutex.unlock();
}

void OggHTTPWorker::sendToClients(IOBuffer newbuf) {

	//mutex.lock();
	map<int, OggHTTPClient*>::iterator it = clients.begin();
	while(it != clients.end()) {
		it->second->sendToClient(newbuf);
		++it;
	}
	//mutex.unlock();
}

void OggHTTPWorker::shutdown() {
}