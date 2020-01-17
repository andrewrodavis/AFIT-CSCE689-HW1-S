#include "TCPConn.h"

#include <iomanip>
#include <sstream>
#include <cstring>
#include <sys/socket.h>

TCPConn::TCPConn(){}
TCPConn::~TCPConn(){}

TCPConn::TCPConn(int server_fd, int client_fd){
    this->client_socket_fd = client_fd;
    this->server_socket_fd = server_fd;
}

int TCPConn::getSocket() { return this->client_socket_fd; }

int TCPConn::sendText(const char *msg, int size) {
    return(send(this->client_socket_fd, msg, size, MSG_DONTWAIT));
}

// Send one big string
void TCPConn::sendMenu() {
    int length = 0;
    int bytes_sent = 0;
    std::stringstream menu;

    menu << "<========== Welcome to the Server! ==========>\n";
    menu <<	"| Options " << std::right << std::setw(37) << "|\n";
    menu << "|   1: Your socket file descriptor " << std::setw(12) <<  "|\n";
    menu << "|   2: The sum of 4 and 5 " << std::setw(21) <<  "|\n";
    menu << "|   3: The letter B " << std::setw(27) << "|\n";
    menu << "|   4: The number 4 " << std::setw(27) << "|\n";
    menu << "|   5: A joke " << std::setw(33) << "|\n";
    menu << "|   passwd: " << std::setw(35) << "|\n";
    menu << "|   exit: " << std::setw(37) << "|\n";
    menu << "|   menu: " << std::setw(37) << "|\n";
    menu << "<============================================>\n";
    menu << "\nEnter your choice: 0";
//    std::cout << menu.str().c_str();

    bytes_sent = this->sendText(menu.str().c_str(), strlen(menu.str().c_str()));
    if(bytes_sent == -1){
        perror("Error: Sending to _____");
    }
}