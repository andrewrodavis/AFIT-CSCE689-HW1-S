#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <sstream>

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
    int valid_commands = 0;

    // Setup fd_set vars
    FD_ZERO(&this->master_fd_List);
    FD_ZERO(&this->read_fds);

    //  add server FD to list
    FD_SET(this->serverSocketFD ,&this->master_fd_List);
    //  Initial note of largest FD
    this->max_fd = this->serverSocketFD;

    while(1){
        // Reset appropriate variables
        obj_index = 0;
        valid_commands = 0;
        this->command_list.clear();
        this->requested_data.str(std::string());
        memset(this->buffer, '\0', sizeof(this->buffer));

        this->read_fds = this->master_fd_List;      // Copy

        select_ret_val = (select(this->max_fd + 1, &this->read_fds, NULL, NULL, NULL) == -1);
        if(select_ret_val < 0){
            perror("Error: On Select");
            exit(1);    // Why 1?
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
                        // At this point, we have the input from the client: number
                    else {
                        obj_index = this->getVectorIndex(i);
                        // Get the data from the client --> Stored in buffer already
//                        this->buffer[numberOfBytesRcvd + 1] = '\n';
//                        std::cout << buffer << std::endl;
                        valid_commands = this->checkForFullInput();

                        // Check for partial/multiple commands
                        for(int i = 0; i < valid_commands; i++){
                            this->process_request(this->command_list[i], this->tcpConnList[obj_index]);
                        }
                        this->tcpConnList[obj_index].sendText(this->requested_data.str().c_str());
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

/**********************************************************************************************
* checkForFullInput - Checks the validity of the user input, including for multiple commands (returns how many commands), partial commands,
*              and what the command is
*
*    Throws: returns 0 on partial command, error
*    https://thispointer.com/c-how-to-find-an-element-in-vector-and-get-its-index/
 *   **Work to be done: handle partial commands. I only handle multiple ones at this point
**********************************************************************************************/
int TCPServer::checkForFullInput() {
    int num_of_commands = 0;
    int temp_begin = 0;
    int temp_end = 0;
    std::string converted_buffer = this->buffer;

    for(int i = 0; i < sizeof(this->buffer); i++){
        // Checks if user types \n character
        if(this->buffer[i] == '\\'){
            if(this->buffer[i + 1] == 'n'){
                // Indicates a new command, note
                num_of_commands += 1;
                // Need to add the command to the list of command
                // Need to add the command to the list of command
                temp_end = i;
                this->command_list.push_back(converted_buffer.substr(temp_begin, (temp_end - temp_begin)));
                temp_begin = i + 2;
            }
        }
    }

    // No need, assume that the user inputs a command given how the client is setup
    // But, I do need to add the single command to the list
    num_of_commands += 1;   // Need to add 1 for the implicit \n
    if(num_of_commands == 1){
        this->command_list.push_back(converted_buffer);
    }
    else{
        this->command_list.push_back(converted_buffer.substr(temp_begin, (temp_end - temp_begin)));
    }
    // Debugging
//    for(auto it = begin(this->command_list); it != end(this->command_list); it++){
//        std::cout << *it << "\n";
//    }

    // At this point, the num_of_command list holds however many commands the user input, as well as a populated this->command_list
    return num_of_commands;
}
/**********************************************************************************************
 * process_request - This processes each request when multiple commands, or one command, are/is sent.
 *
 *
 *    Throws:
 *    Joke Source: https://unijokes.com/air-force-jokes/
 **********************************************************************************************/
void TCPServer::process_request(std::string command, TCPConn tcpConn) {
     if(command == "1"){
         this->requested_data << "-----Request Fulfilled-----\n";
         this->requested_data << "Your file descriptor is: ";
         this->requested_data << std::to_string(tcpConn.getSocket());
         this->requested_data << "\n\n";
     }
     else if(command == "2"){
         this->requested_data << "-----Request Fulfilled-----\n";
         this->requested_data << "4 + 5 = 9\n\n";
     }
     else if(command == "3"){
         this->requested_data << "-----Request Fulfilled-----\n";
         this->requested_data << "The letter is B\n\n";
     }
     else if(command == "4"){
         this->requested_data << "-----Request Fulfilled-----\n";
         this->requested_data << "The number is 4\n\n";
     }
     else if(command == "5"){
         this->requested_data << "-----Request Fulfilled-----\n";
         this->requested_data << "An airman in a bar leans over to the guy next to him and says, \"Wanna hear a marine joke?\"\n";
         this->requested_data << "The guy next to him replies, \"Well before you tell that joke, you should know something.\"\n";
         this->requested_data << "I'm 6' tall, 200lbs, and I'm a a Marine.\n";
         this->requested_data << "The guy sitting next to me is 6'2\", weights 225, and he's a Marine.\n";
         this->requested_data << "The fella next to him is 6'5\", weighs 250, and he's a Marine.\n";
         this->requested_data << "Now, you still wanna tell that joke?\"\n";
         this->requested_data << "The Airman says, \"Nah, I don't want to have to explain it three times.\n\n";
     }
     else if(command == "passwd"){
         this->requested_data << "-----Request Fulfilled-----\n";
         this->requested_data << "passwd command does nothing yet\n\n";
     }
     else if(command == "exit"){
         this->requested_data << "-----Request Fulfilled-----\n";
         this->requested_data << "Goodbye\n\n";
         tcpConn.sendText(requested_data.str().c_str());
         close(tcpConn.getSocket());
     }
     else if(command == "menu"){
         tcpConn.sendMenu();
     }
     else{
         this->requested_data << "Invalid Command. Refer to the menu and try again.";
     }
 }