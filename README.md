Clinet Server application with an external queue. 
I'll be using simple queue which includes some old client-server code from a project I wrote


Clients:
* Should be configured from a command line;
* Can read data from a file or from a command line (you decide);
* Can request server to:
  - AddItem()
  - RemoveItem()
  - GetItem()
  - GetAllItems()
* Data is in the form of strings;

 USAGE: ./client <type> [data]
 *  types: a [data] to add item
           g [data] to get item by id
           l no data added to get all items
           r [data] to remove item by id


Server:
* Has a data structure that holds the data in the memory (Ordered Map for C++);
* Server should be able to add an item, remove an item, get a single or all item from the data structure;
USGAE: ./server


How to run my project:
======================
1) make all (to compile the project).
2) use at least 3 terminals (Server, Client, Queue service).
  2.1) enter the bin directory: $ cd bin
  2.2) Run the queue service, server and client:  
  $ ./qs
  $ ./server
  $ ./client <type> [data]



SQS Assumptions: 
=================
Choosing FIFO queue. Even though it limits the batch working size it allowes exactly- once proccessing (vs At-least-once by the standard queue).
And it resolves the best-effort ordering of the standard queue.
- Settings: Dead Letter Queue for better error handling "later".
- queue server is running on "localhost" listening to port 22000 and 22001
-defines QS_HOSTNAME "localhost"
-defines QS_PRODUCER_PORT 22000
-defines QS_CONSUMER_PORT 22001
-defines MAX_MSG_SIZE 80



Design decisions: 
=================

Queue Server API: 

etypedef enum {
  ADD_ITEM,
  GET_ITEM,
  GET_ALL_ITEMS,
  REMOVE_ITEM
} QueueMessageType;

typedef struct {
  QueueMessageType messageType;
  std::string data;
} QueueMessage;

int sendQueueMessage(const QueueMessage msg);
int getQueueMessage(QueueMessage& msg);


To poll new msg from the queue:
- return value: if empty -1.
- Using short polling. For large scale usage a long polling is better.

Server- Map: 
 - I'll be using static counter to set the keys, as we're using a small scale 
   map. For larger map we will need a different, more robust mathod.
- If two different msg attempt to add the same "data", it will get a new ID 
  and will be considered a new and different mapItem. Removing an item will be done by item's ID.

Since it is multiple thread system I used mutex to lock the map/log file when needed to avoid clashes.
The server is using queue's API to get messages.
A new message creates a new thread for processing. As this a small scale project I didn't limit the amount of threads. 
When the queue is empty I will poll again afater 1 sec, both server and queue are running at all times. 
And the data is not persistent.  
