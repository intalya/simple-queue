#include "QueueServer.h"


int main(int argc, char* argv[]) {
  QueueMessage qm;
  qm.messageType = QueueMessageType::ADD_ITEM;
  qm.data = "YO YO";
  if (argc > 1)
    qm.data += argv[1];
  sendQueueMessage(qm);

  return 0;
}


