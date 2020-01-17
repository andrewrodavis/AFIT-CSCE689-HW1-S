#ifndef TCPSERVER_H
#define TCPSERVER_H

#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <sstream>

#include "Server.h"
#include "TCPConn.h"

class TCPServer : public Server 
{
public:
   TCPServer();
   ~TCPServer();

   void bindSvr(const char *ip_addr, unsigned short port);
   void listenSvr();
   void shutdown();
   void process_request(std::string command, TCPConn tcpConn);

   int getVectorIndex(int fd);
   int checkForFullInput();

private:
    struct sockaddr_in myAddress;   // This is the struct of the server
    struct sockaddr_in theirAddr;   // This is the struct of the client. Eventually this needs to be in a list, i think.
    int serverSocketFD = 0;
    int clientFD = 0;   // Eventually this needs to be stored in a list for multiple connections
    int option = 1;     // This is used for setsockopt. 1 means yes

    fd_set master_fd_List;
    fd_set read_fds;
    int max_fd;
    char buffer[2048];   // For client data

    std::vector<std::string> command_list;
    std::vector<TCPConn> tcpConnList;

    std::stringstream requested_data;
};


#endif
