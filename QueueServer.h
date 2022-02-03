// QueueServer.h

#ifndef QUEUE_SERVER_H
#define QUEUE_SERVER_H

#include <string>

#define QS_HOSTNAME "localhost"
#define QS_PRODUCER_PORT 22000
#define QS_CONSUMER_PORT 22001
#define MAX_MSG_SIZE 80

typedef enum {
  ADD_ITEM,
  GET_ITEM,
  GET_ALL_ITEMS,
  REMOVE_ITEM
} QueueMessageType;

typedef struct {
  QueueMessageType messageType;
  std::string data;
} QueueMessage;

void resolve_name(const char *hostname, char *resolved_addr);

// This function sends a message to the queue.
// Returns 0 on success, -1 on error
int sendQueueMessage(const QueueMessage& msg);

// This function gets a message from the queue.
// Returns 0 on success, -1 on error including no messages to get
int getQueueMessage(QueueMessage& msg);

#endif

