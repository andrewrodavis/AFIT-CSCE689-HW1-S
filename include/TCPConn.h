#ifndef TCPCONN_H
#define TCPCONN_H

#pragma once

#include <iostream>

const int max_attempts = 2;

class TCPConn 
{
public:
    TCPConn();
   ~TCPConn();
    TCPConn(int server_socket_fd, int client_socket_fd);

   //bool accept(SocketFD &server);

   // Why two different ones?
   int sendText(const char *msg);
   int sendText(const char *msg, int size);

   void handleConnection();
   void startAuthentication();
   void getUsername();
   void getPasswd();
   void sendMenu();
   void getMenuChoice();
   void setPassword();
   void changePassword();
   
   bool getUserInput(std::string &cmd);

   void disconnect();
   bool isConnected();

   void setSocket(int socketFD);
   int getSocket();

   //unsigned long getIPAddr() { return _connfd.getIPAddr(); };

   // _variable == private variables
private:
    int server_socket_fd = 0;
    int client_socket_fd = 0;
    int id = 0;
   enum statustype { s_username, s_changepwd, s_confirmpwd, s_passwd, s_menu };
   statustype _status = s_username;
   //SocketFD _connfd;
   std::string _username; // The username this connection is associated with
   std::string _inputbuf;
   std::string _newpwd; // Used to store user input for changing passwords
   int _pwd_attempts = 0;
};


#endif
