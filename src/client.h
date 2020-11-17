#include<sys/types.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/stat.h>

// TODO: Add ifndef ?
#include<openssl/ssl.h>
#include<openssl/err.h>
#include<openssl/md5.h>

#include<libxml/parser.h>
#include<libxml/tree.h>

#include"../lib/http-parser/http_parser.h"
#include"network.h"

#define CLIENT_ID "9d2223ab8e334c92bf2584a1e9a9516b"
#define CLIENT_SECRET "ad81374363cf4a6ebd9aad69027615d5"
#define CODE "1449649"
#define TOKEN "AgAAAAAJfAwtAAaSXZEN657D4ETDiWSPzkL4oDE"
#define MAXLINE 5000
#define HEADER_LEN 1000

#define S_ITEM_LEN 256
#define MAX_NODES 300 
#define MAX_LEAFS 300 
#define INIT_QUEUE_SIZE 15

#define FIFO_PATH "./fifo"
#define DOWNLOAD_PATH "/home/roman/Documents/clang/yandexDiskWebDav/build"
#define MAX_PATH_LEN 500

typedef struct QNode {
    char href[S_ITEM_LEN];
    int isFile;
} QNode;

typedef struct Queue {
    size_t front, rear;
    size_t size, capasity;
    QNode *data;
} Queue;


typedef struct Node {
    char href[S_ITEM_LEN];

    struct Node* nodes[MAX_NODES];
    size_t nodes_count;
} Node;

struct file_system{
    struct item *head;
    int free_space;
    int total_space; 
};

int getToken(void);
//void traverseXML(xmlNode *a_node, struct item *head);
int fileUpload(const char *file, long int file_size, const char *remPath, struct network *net); 
int uploadFile(const char *localPath, const char *remotePath, struct network *net);


void treeTraverse(Node *node);

Queue* initQueue(void);
void destroyQueue(Queue *queue);
int changeQueueSize(Queue *queue);
int addToQueue(Queue *queue, QNode *node);
QNode* getFromQueue(Queue *queue);


int synchronize(const char *rootPath);

