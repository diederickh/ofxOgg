#include "OggHTTPClient.h"
#include "OggHTTPWorker.h"

OggHTTPClient::OggHTTPClient(OggHTTPWorker& server, int sockNum) 
:server(server)
,sock(sockNum)
,send_header(false)
{
}

OggHTTPClient::~OggHTTPClient() {
}

void OggHTTPClient::run() {
	done = false;
	while(true) {
		printf("client thread...\n");
		//::sleep(500);
	}
}

void OggHTTPClient::readFromSocket() {
	if(send_header) {
		return;
	}
	//return;
	char tmp_buf[4*4096];
	ssize_t bytes_read = read(sock, tmp_buf, 4096);
	if(bytes_read == -1) {
		server.removeClient(this);
		return;
	}
	int total_read = bytes_read;
	// check if we have a complete http request.
	
	if(bytes_read > 0) {
//		printf("++++++++++++++++++++++++++++++\n");
	}
	std::copy(tmp_buf, tmp_buf+bytes_read, std::back_inserter(read_buffer));
	for(int i = 0; i < read_buffer.size(); ++i) {
//		printf("%c", read_buffer[i]);
	}
	if(bytes_read > 0) {
//		printf("---------------------------------\n\n");
	}
	sendHTTPResponse();
}

void OggHTTPClient::sendHTTPResponse() {
	if(send_header) {
		printf("already sent header\n");
		return;
	}
	stringstream ss;
	/*
	ss	<< "HTTP/1.1 206 Partial Content\r\n"
		<< "Server: Apache/2.2.20 (Ubuntu)\r\n"
		<< "Keep-Alive: timeout=5, max=10000\r\n"
		<< "Connection: Keep-Alive\r\n"
		<< "Accept-Ranges: bytes\r\n"
		<< "Transfer-Encoding: chunked\r\n"
		<< "Content-Type: video/ogg\r\n"
		<< "\r\n";
		
		//		<< "ice-audio-info: bitrate=128;channels=2;samplerate=44100;quality=1%2e0\"
	*/
	//<< "HTTP/1.1 206 Partial Content\r\n"
	ss	<<	"HTTP/1.0 200 OK\r\n"
		<<	"Content-Type: application/ogg\r\n"
		//<< "Content-Type: video/ogg\r\n"
//		<<	"icy-br:128\r\n"
//		<< "icy-description:Diffusion en broadcast de toutes informations qui concernent le Logiciel Libre ainsi que des cr..ations vid..o sous Licence Libre.\r\n"
//		<< "icy-genre:WebTV, Libre, France, Bordeaux\r\n"
//		<< "icy-name:WebTV - T..l..vision Libre\r\n"
//		<< "icy-pub:1\r\n"
//		<< "icy-url:http://modulix.org\r\n"
//		<< "Server: icecast 2.3.2\r\n"
		<< "Connection: Keep-Alive\r\n"
		<< "Cache-Control: no-cache\r\n"
		<< "\r\n";
			
	string http_header		= ss.str();
	int http_len			= http_header.size();
	const char* http_data	= http_header.c_str();
	int http_to_send		= http_len;
	int http_total			= 0;
	send_header = true;
	while(http_to_send > 0) {
		int http_done = write(sock, http_data+http_total, http_to_send);
		
		if(http_done == -1) {
			server.removeClient(this);
			return;
		}
		http_to_send -= http_done;
		http_total += http_done;
	}

	
	// now send theora headers.

	sendToClient(server.header_buffer);

}

void OggHTTPClient::sendToClient(IOBuffer newbuf) {
	if(!send_header) {
		return ;
	}

	int to_send = newbuf.getNumBytesStored();
	//printf("send page: %d\n", to_send);
	int bytes_left = to_send;
	int bytes_send = 0;
	while(bytes_left > 0) {
		int done = write(sock, newbuf.getPtr()+bytes_send, bytes_left);
		if(done == -1) {	
		//	printf("==================== removing client!!!\n");
			server.removeClient(this);
			return;
		}
		//printf(">>>>>>>>>>>>>> %d\n", done);
		bytes_left -= done;
		bytes_send += done;
	}
	
}
