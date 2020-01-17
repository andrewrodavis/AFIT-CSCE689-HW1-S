#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <cstring>

#include "TCPServer.h"
#include "TCPConn.h"

TCPServer::TCPServer() {}
TCPServer::~TCPServer() {}

/**********************************************************************************************
 * bindSvr - Creates a network socket and sets it nonblocking so we can loop through looking for
 *           data. Then binds it to the ip address and port
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types ---> What are these?
 *    https://stackoverflow.com/questions/1150635/unix-nonblocking-i-o-o-nonblock-vs-fionbio
 *    https://beej.us/guide/bgnet/html/
 **********************************************************************************************/
// accept4 for nonbinding portion stuff

void TCPServer::bindSvr(const char *ip_addr, short unsigned int port) {
    // Create a network socket file descriptor
    // AF_INET = ipv4
    // SOCK_STREAM = TCP
    // 0 = default protocol
    // Create socket
    serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    int flags = fcntl(serverSocketFD, F_GETFL, 0);    // Gets the current flag for the file descriptor
    if(serverSocketFD < 0){
        perror("Error: Socket Description");
        exit(1);
    }

    if(setsockopt(serverSocketFD, SOL_SOCKET, (SO_REUSEADDR | SO_REUSEPORT), &option, sizeof(option))){
        perror("Error: setsockopt");
        exit(1);
    }
    fcntl(serverSocketFD, F_SETFL, flags | O_NONBLOCK);    // Error checking?

    //Bind server
    // Does this change the whole struct for later use? It should, it's a member variable
    myAddress.sin_family = AF_INET;
    myAddress.sin_addr.s_addr = inet_addr(ip_addr); //INADDR_ANY; --> This is local ip address, yeah?
    myAddress.sin_port = htons(port);

    if(bind(serverSocketFD, (struct sockaddr *)&myAddress, sizeof(myAddress))){
        perror("Error: Binding");
        exit(1);
    }
    if(listen(serverSocketFD, 10)){
        perror("Error: On Listen");
        exit(1);
    }
}

/**********************************************************************************************
 * listenSvr - Performs a loop to look for connections and create TCPConn objects to handle
 *             them. Also loops through the list of connections and handles data received and
 *             sending of data.
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types *
 **********************************************************************************************/

void TCPServer::listenSvr() {
    int addressLength = 0;
    int numberOfBytesRcvd = 0;
    int obj_index = 0;
    int select_ret_val = 0;

    // Setup fd_set vars
    FD_ZERO(&this->master_fd_List);
    FD_ZERO(&this->read_fds);

    //  add server FD to list
    FD_SET(this->serverSocketFD ,&this->master_fd_List);
    //  Initial note of largest FD
    this->max_fd = this->serverSocketFD;

    while(1){
        std::cout << "In loop\n";
        // Reset appropriate variables
        obj_index = 0;
        memset(this->buffer, '\0', sizeof(this->buffer));
        this->read_fds = this->master_fd_List;      // Copy

        select_ret_val = (select(this->max_fd + 1, &this->read_fds, NULL, NULL, NULL) == -1);
        if(select_ret_val < 0){
            perror("Error: On Select");
            exit(1);    // Why 1?
        }

        // Check this server's socket
        if(FD_ISSET(this->serverSocketFD, &this->read_fds)){

            std::cout << "New connection\n";
        }

        // Check existing connections to do something with
        for(int i = 0; i < *this->read_fds.fds_bits + 1; i++){
            // This indicates that there is a FDescriptor to do something with with socketFD i*** IMPORTANT
            if(FD_ISSET(i, &read_fds)){
                // Check if it is a new connection ------------------------
                //  If it is, need to send hello message
                if(i == this->serverSocketFD){
                    addressLength = sizeof(this->theirAddr);
                    this->clientFD = accept4(serverSocketFD, (struct sockaddr *)&this->theirAddr, (socklen_t *)&addressLength, SOCK_NONBLOCK);
                    // If there is some issue with accept
                    if(this->clientFD == -1){
                        perror("Error: On Accept");
                        continue;
                    }
                        // Otherwise,
                    else{
                        // Add the newly connected client and their file descriptor to the master set of FDSet
                        FD_SET(this->clientFD, &this->master_fd_List);
                        // Need to update the max FD
                        if(this->clientFD > this->max_fd){ this->max_fd = this->clientFD; }
                    }
                    TCPConn tcpConn(this->serverSocketFD, this->clientFD);
                    // TO DO: Send greeting ---***

                    tcpConn.sendMenu();
                    // Add this object to a list
                    this->tcpConnList.push_back(tcpConn);
                }
                    // Otherwise, it is an existing connection to do something with ------------------------
                    //      Since it is an existing FD, I need to pull the corresponding TCPConn object
                    // Check for error on receiving data from current active socketFD.
                    //  Do I need a non-blocking flag? e.g. MSG_DONTWAIT
                else {
                    // This checks for closed connection or error on read
                    if ((numberOfBytesRcvd = recv(i, this->buffer, sizeof(this->buffer), 0)) <= 0) {
                        // Why does this indicate a closed connection?
                        if (numberOfBytesRcvd == 0) {
                            std::cout << "A client has disconnected\n";
                        } else {
                            perror("Error: On recv()");
                        }
                        close(i);       // Close the connection. Buy Why?
                        FD_CLR(i, &this->master_fd_List);
                    }
                        // Otherwise, data is received correctly as noted by select() ----------------------
                        // At this point, we have the input from the client: numbe
                    else {
                        obj_index = this->getVectorIndex(i);
                        // Get the data from the client --> Stored in buffer already
                        this->buffer[numberOfBytesRcvd] = '\0';
                        std::cout << "Message received from client: " << this->buffer << "\n";
                        // Check for partial/multiple commands
                    }
                }
            }
        }
        // Create helper connection --> Is this what I should store in the list?
        // Here, connection is received. Now I need to do stuff with it. What though?
        //  I need to add it to the list of connected clients
        //  I need to send it the message
        // On top of these things, I need to check for new responses and whatnot
    }
}

/**********************************************************************************************
 * shutdown - Cleanly closes the socket FD.
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::shutdown() {
    close(this->serverSocketFD);
    // shutdown(serverFileDescriptor, 2);
}

/**********************************************************************************************
 * getVectorIndes - This is a helper function to get the location of the TCPConn object that
 *              corresponds to the current socket file descriptor that is needed to do stuff with
 *
 *    Throws: Should throw an error if the object does not exist
 *    https://thispointer.com/c-how-to-find-an-element-in-vector-and-get-its-index/
 **********************************************************************************************/
 int TCPServer::getVectorIndex(int fd) {
     for(int i = 0; i < this->tcpConnList.size(); i++){
         if(this->tcpConnList[i].getSocket() == fd){
             return i;
         }
     }
     perror("Error: TCPConn object DNE");
 }