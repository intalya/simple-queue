#include "QueueServer.h"


int main(int argc, char* argv[]) {
  QueueMessage qm;
  if (getQueueMessage(qm) == -1) {
    printf("error getQueueMessage\n");
    return -1;
  }
  printf("got message type=%d, data=%s\n", qm.messageType, qm.data.c_str());
  return 0;
}


