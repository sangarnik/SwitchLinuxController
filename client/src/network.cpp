#include <network.hpp>

namespace net {
int initClient(int *sckt, const std::string& addr, const std::string& port) {
	struct sockaddr_in serv_addr;
	socketInitializeDefault();
	
	if ((*sckt = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return 1; // Error opening socket
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	inet_aton(addr.c_str(), &serv_addr.sin_addr);
	serv_addr.sin_port = htons(atoi(port.c_str()));
	
	if (connect(*sckt, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
		return 2; // Error connecting
	
	return 0;
}
}
