#include "TCPClient.h"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>


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
    std::string input;
    int tracker = 0;
    int total_bytes_sent = 0;
    int remaining_bytes_to_send = 0;

    while(1) {
        tracker = 0;
        total_bytes_sent = 0;
        this->num_bytes_recv = 0;
        this->num_bytes_sent = 0;
        remaining_bytes_to_send = 0;
        memset(this->buffer, '\0', sizeof(this->buffer));
        memset(this->response, '\0', sizeof(this->response));

        this->num_bytes_recv = recv(this->clientSocketFD, this->buffer, sizeof(this->buffer), 0);
        if(this->num_bytes_recv == -1){
            perror("Error: On recv()");
            exit(1);
        }
        else{
            std::cout << buffer << "\n";
        }
        std::cin >> this->response;
        remaining_bytes_to_send = strlen(this->response);
//        this->num_bytes_sent = send(this->clientSocketFD, this->response, std::strlen(this->response), MSG_DONTWAIT);
        while(total_bytes_sent < strlen(this->response)){
            this->num_bytes_sent = send(this->clientSocketFD, (this->response + total_bytes_sent), remaining_bytes_to_send, MSG_DONTWAIT);
            if (this->num_bytes_sent == -1){
                perror("Error: On Send");
                break;
            }
            total_bytes_sent += this->num_bytes_sent;
            remaining_bytes_to_send -= this->num_bytes_sent;
        }
//        this->num_bytes_sent = send(this->clientSocketFD, this->response, std::strlen(this->response), MSG_DONTWAIT);
    }
}

/**********************************************************************************************
 * closeConnection - Your comments here
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPClient::closeConn() {
    close(this->clientSocketFD);
    exit(0);
}


