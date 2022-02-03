#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <set>
#include <queue>
#include "QueueServer.h"

#define BACKLOG 100

static int consumer_sockfd;
static int producer_sockfd;
std::set<int> consumer_fds;
std::set<int> producer_fds;
std::queue<QueueMessage> queueMessages;

int socket_bind(int port, const char *resolved_addr)
{
  //resolved_addr = IP
  int flag = 0;
  int listen_fd = -1;

  //setting the server details:
  struct sockaddr_in nodeAddr;
  memset(&nodeAddr, 0, sizeof(nodeAddr)); //cleaning the struct
  nodeAddr.sin_family = AF_INET;          //setting to IPv4

  //function converts the int netshort from
  //network byte order to host byte order. man htons(3)
  nodeAddr.sin_port = htons(port);
  //convert IPv4 and IPv6 addresses from text to binary form
  if ((flag = inet_pton(AF_INET, resolved_addr, &(nodeAddr.sin_addr))) < 1) {
    if (flag == 0)
      printf("server.c:connect_to:inet_pton:invalid addr\n");
    else
      printf("server.c:inet_pton error\n");
    exit(1); //due to invalid hostname issues
  }

  //create a socket, return of -1 is an error
  if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    printf("\nserver.c:socket creation failed\n");
    exit(1);
  }

  //Bind  - bind a name to a socket  man Bind(2)
  if (bind(listen_fd, (struct sockaddr *)&nodeAddr, sizeof(nodeAddr)) == -1) {
    printf("server.c: bind failed\n");
    close(listen_fd);
    exit(1);
  }

  return listen_fd;
}

int validate_message(const char *msg, QueueMessage& qm)
{
  if (!msg)
    return -1;
  std::string strMsgType;
  strMsgType = msg[0];
  int intMsgType = std::stoi(strMsgType);
  QueueMessageType qmType = (QueueMessageType) intMsgType;
  int flag = 0;

  qm.messageType = qmType;
  qm.data = std::string(msg+1);
  printf("server.c %d:validate message: (ADD_ITEM) q.data=%s\n",
    __LINE__, qm.data.c_str());
  switch (qmType) {
  case (ADD_ITEM):
  case (GET_ITEM):
  case (GET_ALL_ITEMS):
  case (REMOVE_ITEM):
    break;

  default:
    printf("server.c: %d:validate message: default case= error!\n", __LINE__);
    return -1;
  }
  queueMessages.push(qm);
  return 0;
}

int read_producer_fd(int sockfd)
{
  int read_ret = 0;
  char msg[MAX_MSG_SIZE] = {0};

  //printf("server.c %d:read_producer_fd:%d\n", __LINE__, sockfd);
  if ((read_ret = read(sockfd, msg, MAX_MSG_SIZE)) == -1) {
    printf("server.c: read_producer_fd, read error\n");
    exit(1); //error code
  } else if (read_ret == 0) {
    return -1;
  }

  //printf("server.c %d:read_producer_fd:%s\n", __LINE__, msg);
  QueueMessage qm;
  int parse_ret = validate_message(msg, qm);
  if (parse_ret != 0) {
    printf("read_producer_fd:incorrect msg type: failed parsing=%s\n", msg);
    return -1;
  }
  qm = queueMessages.back();
  printf("after producer message, back of the queue is now type=%d data=%s\n",
    qm.messageType, qm.data.c_str());

  return 0;
}

int handle_consumer(int sockfd)
{
  int send_ret = 0;
  char msg[MAX_MSG_SIZE] = {0};

  printf("server.c %d:handle_consumer: sockfd:%d\n", __LINE__, sockfd);
  if (queueMessages.empty()) {
    close(sockfd);
    printf("server.cd:handle_consumer: queue is empty\n");
    return -1;
  }
  QueueMessage qm = queueMessages.front();
  int intMessageType = (int) qm.messageType;
  std::string strMessage = std::to_string(intMessageType);
  strMessage += qm.data;

  send_ret = send(sockfd, strMessage.c_str(), strMessage.size(), 0);
  printf("server.c: handle_consumer sockfd=%d, send_ret=%d\n", sockfd, send_ret);
  if (send_ret == -1) {
    printf("server.c: handle_consumer send error\n");
    close(sockfd);
    exit(1); //error code
  } else if (send_ret == 0) {
  close(sockfd);
    return -1;
  }
  close(sockfd);
  queueMessages.pop();
  // debug
  qm = queueMessages.front();
  printf("after consumer message, front of the queue is now type=%d data=%s",
    qm.messageType, qm.data.c_str());

  return 0;
}


int accepting(int consumer_sockfd, int producer_sockfd,
  const fd_set *selected_fds, fd_set *open_sockets_fds)
{
  int sockfd = 0;
  int new_sockfd = 0;
  // TODO: consumer fd

  struct sockaddr_in addr;
  struct sockaddr_in clientAddr;
  socklen_t addrlen = sizeof(clientAddr);
  //printf("server.c %d:accepting: producer_sockfd: %d consumer_sockfd=%d\n",
  //  __LINE__, producer_sockfd, consumer_sockfd);

  for (sockfd = 0; sockfd < FD_SETSIZE; sockfd++) {
    // tests to see if a fd is part of the set
    if (FD_ISSET(sockfd, selected_fds)) {
      //printf("server.c %d:accepting\n", __LINE__);
      //listenfd is the new socket we wait to accept on
      if (sockfd == producer_sockfd || sockfd == consumer_sockfd) {
        //printf("server.c %d:accepting\n", __LINE__);
        //Accepting connection, man accept error code =-1
        if ((new_sockfd = accept(sockfd,
                                 (struct sockaddr *)&addr, &addrlen)) == -1) {
          printf("server.c: accept error");
          exit(0);
        }
        //printf("server.c %d:accept new_sockfd=%d\n", __LINE__, new_sockfd);
        if (sockfd == producer_sockfd) {
          FD_SET(new_sockfd, open_sockets_fds);
          producer_fds.insert(new_sockfd);
        } else {
          handle_consumer(new_sockfd);
        }
      } else { // arriving packet on an already accepted sockfd
        //printf("server.c %d:accepting:packet on an accepted sockfd %d\n",
        //       __LINE__, sockfd);
        int read_ret = -1;
        if (producer_fds.find(sockfd) != producer_fds.end()) {
          read_ret = read_producer_fd(sockfd);
          producer_fds.erase(sockfd);
        }
        //printf("server.c %d:closing sockfd sockfd=:%d\n",
        //       __LINE__, sockfd);
        close(sockfd);
        FD_CLR(sockfd, open_sockets_fds);
        break;
      }
    }
  }

  //printf("server.c:accepting: new sockfd:%d\n", new_sockfd);
  return 0;
}



/**
 * @brief listen and select
 * waiting on "select" to hear from a client
 * @param sockfd 
 */
void listen_select(int consumer_sockfd, int producer_sockfd)
{
  int flag = -1;
  fd_set selected_fds;
  fd_set open_sockets_fds;

  //clearing the set
  FD_ZERO(&selected_fds);
  FD_ZERO(&open_sockets_fds);
  //int new_sockfd = -1;

  //adding sockfd to our set.
  FD_SET(consumer_sockfd, &open_sockets_fds);
  FD_SET(producer_sockfd, &open_sockets_fds);

  while (1)
  {
    selected_fds = open_sockets_fds;
    //printf("\nserver:starting select process\n\n");

    struct timeval timeout = {10, 0};
    //return value of select: set size of active fds
    //for (int i=0; i<FD_SETSIZE; i++)
    //  if (FD_ISSET(i, &selected_fds))
    //    printf("pre select, fd %d is set\n", i);
    if ((select(FD_SETSIZE, &selected_fds, NULL, NULL, &timeout)) == -1)
    {
      printf("server.c:select errno=%d\n", errno);
      exit(1);
    }

    accepting(consumer_sockfd, producer_sockfd, &selected_fds, &open_sockets_fds);
  }
}

void init_net_services(int& consumer_sockfd, int& producer_sockfd) {
  int listen_ret;
  char hostname[80];

  resolve_name(QS_HOSTNAME, hostname);

  if (-1 == (consumer_sockfd = socket_bind(QS_CONSUMER_PORT, hostname)) ||
    -1 == (producer_sockfd = socket_bind(QS_PRODUCER_PORT, hostname))) {
    printf("unable to bind sockets\n");
    exit (1);  
  }

  //printf("bind succeeded, waiting...\n");
  if (-1 == (listen_ret = listen(consumer_sockfd, BACKLOG)) ||
    -1 == (listen_ret = listen(producer_sockfd, BACKLOG))) {
    printf("unable to bind sockets\n");
    exit (1);  
  }


}

int main() {
  init_net_services(consumer_sockfd, producer_sockfd);
  listen_select(consumer_sockfd, producer_sockfd);
  sleep(60);
}
