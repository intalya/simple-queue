/**
 * @file Client.cpp
 * @author intalya
 * @brief Sends structed message to a simple queue.
 * using the queue interface: sendQueueMessage. 
 * @version 0.1
 * @date 2022-02-02
 *
 * @copyright Copyright (c) 2022
 *  configured from a command line.
 *  USAGE: ./client <type> [data]
 *  types: a [data] to add item
           g [data] to get item by id
           l no data added to get all items
           r [data] to remove item by id

 */

//#include "Client.h"
#include <iostream>
#include "QueueServer.h"

int sendMsgToQueue(std::string type, std::string data);
QueueMessageType createType(std::string type);
int checkType(std::string type);
void usage(int line);

int main(int argc, char *argv[])
{
  std::cout << ("Client: Hello World!\n");

  // usage check: 

  if (!(argc == 3 || argc == 2))
  {
    usage(__LINE__);
    return 1; // usage error
  }

  std::string type = argv[1];
  std::string data;
  
  if (argc == 3)
  {
    if (type.at(0) == 'l') {
      usage(__LINE__);
      return 1;
    }
    data = argv[2];
  }
  int flag = checkType(type);
  
  if (flag == -1)
    return 1;
  else 
    sendMsgToQueue(type,data);

  std::cout << "Clinet: communication protocol ended\n";
  return 0;
}

void usage(int line) {
  std::cout << line 
            << ": USAGE: $./ client<type>[data]\n"
            << "  a [data](to add item)\n"
            << "  g [data](get item by id)\n"
            << "  l (get all items)\n"
            << "  r [data] (remove item by id)\n";
}

int checkType(std::string type){
  if (type.length() == 1) {
    if (type.at(0) != 'a' && type.at(0) != 'g' && type.at(0) != 'l' && type.at(0) != 'r') {
      usage(__LINE__);
      return -1; 
    }
    return 1;
  }
  usage(__LINE__);
  return -1;
}

int sendMsgToQueue(std::string type, std::string data) {
  std::cout << "Client command: " << type << " " << data << std::endl;
  QueueMessage msg;
  msg.messageType = createType(type);
  msg.data = data;
  sendQueueMessage(msg);
}

QueueMessageType createType(std::string type)
{
  std::cout << "Client type: " << type << std::endl;
  switch (type.at(0))
  {
  case 'a':
    return ADD_ITEM;
  case 'r':
    return  REMOVE_ITEM; 
  case 'g':
    return GET_ITEM;
  case 'l':
    return GET_ALL_ITEMS;
  default:
    usage(__LINE__);
    exit(1); //TODO
  }
}

