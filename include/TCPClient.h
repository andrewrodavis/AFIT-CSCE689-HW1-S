#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#pragma once

#include <string>
#include <netinet/in.h>
#include "Client.h"

/**********************************************************************************************
 * https://www.thecrazyprogrammer.com/2017/06/socket-programming.html
 *
 **********************************************************************************************/
// The amount to read in before we send a packet
const unsigned int stdin_bufsize = 50;
const unsigned int socket_bufsize = 100;

class TCPClient : public Client
{
public:
   TCPClient();
   ~TCPClient();

   virtual void connectTo(const char *ip_addr, unsigned short port);
   virtual void handleConnection();

   virtual void closeConn();

private:
    struct sockaddr_in serverIpAddr;
    int clientSocketFD = 0;
    int num_bytes_recv = 0;
    int num_bytes_sent = 0;

    char buffer[2048];
    char response[2048];

};


#endif
