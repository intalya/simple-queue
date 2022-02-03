// QueueServerLibrary.cpp

#include <netdb.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <unistd.h>
#include "QueueServer.h"

void resolve_name(const char *hostname, char *resolved_addr) {
  struct hostent *h;
  char **pp;
  char str[INET_ADDRSTRLEN];
  if ((h = gethostbyname(hostname)) == NULL) {
    herror("ss");
    exit(1);
  }
  pp = h->h_addr_list;
  inet_ntop(h->h_addrtype, *pp, str, sizeof(str));
  strcpy(resolved_addr, str);
}

static int socket_connect(int port, const char *resolved_addr) {
  int flag = 0;
  int sockfd = -1;

  //setting the server details:
  struct sockaddr_in nodeAddr;
  memset(&nodeAddr, 0, sizeof(nodeAddr)); //cleaning the struct
  nodeAddr.sin_family = AF_INET;          //setting to IPv4

  //function converts the int netshort from
  //network byte order to host byte order. man htons(3)
  nodeAddr.sin_port = htons(port);
  // printf("QSL:connect_to __%s__\n", resolved_addr);

  //convert IPv4 and IPv6 addresses from text to binary form
  if ((flag = inet_pton(AF_INET, resolved_addr, &(nodeAddr.sin_addr))) < 1) {
    if (flag == 0)
      std::cout << "QSL:connect_to:inet_pton:invalid addr\n";
    else
      std::cout << ("QSL:inet_pton error\n");
    exit(1); //due to invalid hostname issues
  }
  //create a socket, return of -1 is an error
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    std::cout << ("\nQSL:socket creation failed\n");
    exit(1);
  }

  //connect - initiate a connection on a socket: man connect(2)
  if (connect(sockfd, (struct sockaddr *)&nodeAddr, sizeof(nodeAddr)) == -1) {
    printf("QSL:connect failed\n");
    exit(1);
  }

  return sockfd;
}

int qsSend(const std::string& buf) {
  //creating a socket
  char resolved_addr[INET_ADDRSTRLEN] = {0};
  //changing the original arg to real IP
  resolve_name(QS_HOSTNAME, (char *)resolved_addr);
  // printf("QSL: resolved_addr: %s\n", resolved_addr);

  int flag = 0;
  int sockfd = -1;

  sockfd = socket_connect(QS_PRODUCER_PORT, resolved_addr);
  // printf("QSL: sockfd: %d\n", sockfd);

  //Start communication with the server:
  flag = send(sockfd, buf.c_str(), buf.length(), 0);

  // printf("QSL: communication protocol ended, flag=%d, send msg=%s\n",
  //   flag, buf.c_str());
  close(sockfd);
  return 0;
}


int encodeMessage(const QueueMessage& msg) {
  // first byte is message type
  // rest of data is string of message
  std::string buf;
  buf = std::to_string(msg.messageType);
  buf += msg.data;

  return qsSend(buf);
}


int sendQueueMessage(const QueueMessage& msg) {
  // to client
  
	return encodeMessage(msg);
}

int qsRecv(std::string& buf) {
  buf.clear();
  //creating a socket
  char resolved_addr[INET_ADDRSTRLEN] = {0};
  //changing the original arg to real IP
  resolve_name(QS_HOSTNAME, (char *)resolved_addr);
  //printf("client: resolved_addr: %s\n", resolved_addr);

  int received = 0;
  int sockfd = -1;

  sockfd = socket_connect(QS_CONSUMER_PORT, resolved_addr);
  //printf("client: sockfd: %d\n", sockfd);

  
  char recv_buf[MAX_MSG_SIZE]; // MAX message size 80? 
  //Start communication with the server:
  do {
    received = recv(sockfd, recv_buf, MAX_MSG_SIZE-1, 0);
    printf("QSL: qsRecv ended received=%d\n", received);
    if (received == -1)
      break;
    recv_buf[received] = '\0';
    buf += recv_buf;
    // printf("QSL: qsRecv ended %d bytes, recv msg=%s\n",
    //   received, buf.c_str());
  } while (received > 0);

  close(sockfd);
  printf("QSL: qsRecv ended buf size=%lu, buf=__%s__\n", buf.size(), buf.c_str());
  if (buf.size() == 0)
    return -1;
  return received;
}

int decodeMessage(QueueMessage& msg) {
  // first byte is message type
  // rest of data is string of message
  std::string buf;
  if (qsRecv(buf) == -1)
    return -1;
  msg.messageType = (QueueMessageType) std::stoi(buf.substr(0,1));
  msg.data = buf.substr(1);
  return 0;
}

int getQueueMessage(QueueMessage& msg) {
  return decodeMessage(msg);
}

