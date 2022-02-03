/**
 * @file Server.cpp
 * @author intalya
 * @brief Has a data structure that holds the data in the memory (Ordered Map for C++);
 * 1) Server should be able to:
 *  - add an item
 *  - remove an item
 *  - get a single
 *  - get all items from the data structure;
 * 2) Server is reading requests from the queue and processing them.
 *  the output of the server is written to a log file serversLog.txt
 * 3) Server should be able to process items in parallel
 * 4) log messages (debug, error) are written to stdout
 * @version 0.1
 * @date 2022-02-03
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <bits/stdc++.h>
#include <thread>
#include <mutex>
#include <unistd.h>
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include "QueueServer.h"

struct item {  
  std::string id;
  std::string data;
};

std::mutex mapMutex;
std::map<std::string, std::string> itemsMap;
std::ofstream logFile("serversLog.txt");

int addItem(std::string data);
int removeItem(std::string id);
int getItem(std::string id);
int getAllItems();
void initiateServices(const QueueMessage &msg);

int addLog(std::string logText);
std::string createId();

int main(int argc, char **argv) {
  
  if (!logFile) {
    std::cout << "File create ERROR" << std::endl;
    exit(1);
  }
  
  QueueMessage msg;
  int flag = 0;

  std::cout << "Server: Hello,World!" << std::endl;
  addLog("Server: Hello,World!");
  std::cout << "Server: log added" << std::endl;

  while (true) {
    flag = getQueueMessage(msg);
    if (flag == -1){
      sleep(1);
      continue;
    }
  
    std::cout << "getQueueMessage: " + flag << std::endl;
    std::thread (initiateServices, msg).detach();

  }

  logFile.close();
 return 1;
}

void initiateServices(const QueueMessage& msg) {
  int flag = 0;
  switch (msg.messageType) {
  case ADD_ITEM:
    flag = addItem(msg.data);
    break;
  case REMOVE_ITEM:
    flag = removeItem(msg.data);
    break;
  case GET_ITEM:
    flag = getItem(msg.data);
    break;
  case GET_ALL_ITEMS:
    flag = getAllItems();
    break;
  default:
    std::cout << "ERROR_MSG_TYPE";
    flag = addLog("ERROR_MSG_TYPE");
    if (flag == -1) {
      exit(1);
    }
  }
}

int addItem(std::string data) {
  std::cout << "Server: addItem(std::string data)" << std::endl;
  mapMutex.lock();
  std::string newID = createId();
  itemsMap.insert(std::pair<std::string, std::string> (newID, data));
  mapMutex.unlock();

  std::string buf;
  buf = "ADD_ITEM " + data + "\n";
  buf += "\t added item:(" + newID;
  buf += ", " + data + ")";

  addLog(buf);
}


std::string createId() {
  std::cout << "Server: createId" << std::endl;
  static int counter = 0;
  counter++;
  return std::to_string(counter);
}


int removeItem(std::string id) {
  std::cout << "Server: removeItem" << std::endl;
  std::string buf;
  mapMutex.lock();
  if (itemsMap.find(id) != itemsMap.end()) {
    itemsMap.erase(id);
    mapMutex.unlock();

    buf = "id " + id + " removed.";

    buf = "REMOVE_ITEM " + id + "\n";
    buf += "\t removed item:(" + id;
    buf += ")";
    addLog(buf);
    return 1;
  }
  else {
    mapMutex.unlock();
    buf = "REMOVE_ITEM:\n\t id " + id + " not found!.";
    addLog(buf);
    return 0;
  }
}


int getItem(std::string id) {
  std::cout << "Server: getItem" << std::endl;
  std::map<std::string, std::string>::iterator it;
  std::string buf;

  mapMutex.lock();
  it = itemsMap.find(id);
  if (it != itemsMap.end()) {
    buf = "GET_ITEM " + id + "\n";
    buf += "\t get item:(" + it->first;
    buf += ", " + it->second + ")";
    mapMutex.unlock();
    addLog(buf);
    return 1;
  }
  else {
    mapMutex.unlock();
    buf = "GET_ITEM:\n\t id " + id + " not found!.";
    addLog(buf);
    return 0;
  }
}


int getAllItems() {
  std::cout << "Server: getAllItems" << std::endl;
  std::string buf;
  std::map<std::string, std::string>::iterator it;
  mapMutex.lock();
  if(itemsMap.empty()) {
    mapMutex.unlock();
    buf += "GET_ALL_ITEMS: map is empty";  
  }
  else {
    buf += "GET_ALL_ITEMS:\n";
    for (it = itemsMap.begin(); it != itemsMap.end(); ++it) {
      buf +="\t (" + it->first + ", " + it->second + ")\n";
    }
    mapMutex.unlock();
  }
  addLog(buf);
}

/**
 * @brief print logs to stdout and logFile
 * 
 * @param logFile 
 * @param logText 
 * @return int 
 */
int addLog(std::string logText) {
  std::cout << "Server: addLog" << std::endl;
  static std::mutex logFileMutex;
  logFileMutex.lock();
  logFile << logText << std::endl;
  logFileMutex.unlock();
  std::cout << logText << std::endl;
}