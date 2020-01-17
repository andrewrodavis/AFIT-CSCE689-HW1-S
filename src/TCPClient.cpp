#include "TCPClient.h"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>


/**********************************************************************************************
 * TCPClient (constructor) - Creates a Stdin file descriptor to simplify handling of user input. 
 *
 **********************************************************************************************/

TCPClient::TCPClient() {
}

/**********************************************************************************************
 * TCPClient (destructor) - No cleanup right now
 *
 **********************************************************************************************/

TCPClient::~TCPClient() {

}

/**********************************************************************************************
 * connectTo - Opens a File Descriptor socket to the IP address and port given in the
 *             parameters using a TCP connection.
 *
 *    Throws: socket_error exception if failed. socket_error is a child class of runtime_error
 *    Questions: serverIpAddr.sin_port = ??? How to get right port AND address. This may be
 *          a basic misunderstanding of socket programming from me
 **********************************************************************************************/

void TCPClient::connectTo(const char *ip_addr, unsigned short port) {
    if((clientSocketFD = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("Error: On Client Socket Creation");
    }
    serverIpAddr.sin_family = AF_INET;
    serverIpAddr.sin_port = htons(port);
    serverIpAddr.sin_addr.s_addr = inet_addr(ip_addr);

    if(connect(clientSocketFD, (struct sockaddr *)&serverIpAddr, sizeof(serverIpAddr)) < 0){
        perror("Error: On Port and IP Problems");
    }
    printf("Connected to a server\n");
    std::cout << "server port: " << this->serverIpAddr.sin_port << "\n";
}

/**********************************************************************************************
 * handleConnection - Performs a loop that checks if the connection is still open, then 
 *                    looks for user input and sends it if available. Finally, looks for data
 *                    on the socket and sends it.
 * 
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPClient::handleConnection() {
    while(1) {
        memset(this->buffer, '\0', sizeof(this->buffer));
        memset(this->response, '\0', sizeof(this->response));

        this->num_bytes_recv = recv(this->clientSocketFD, this->buffer, sizeof(this->buffer), 0);
        if(this->num_bytes_recv == -1){
            perror("Error: On recv()");
            exit(1);
        }
        // This indicates if input is needed from the user
        if((buffer[this->num_bytes_recv - 1]) == '0'){
            buffer[this->num_bytes_recv - 1] = '\0';
            std::cout << buffer;
            std::cin >> response;
            this->num_bytes_sent = send(this->clientSocketFD, this->response, sizeof(this->response), MSG_DONTWAIT);
        }
    }
}

/**********************************************************************************************
 * closeConnection - Your comments here
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPClient::closeConn() {
}


