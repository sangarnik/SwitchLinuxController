#pragma once

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <util.hpp>

namespace net {
int initClient(int* sckt, const std::string& addr, const std::string& port);
}
