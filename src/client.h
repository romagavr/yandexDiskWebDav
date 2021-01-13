#ifndef CLIENT_H
#define CLIENT_H

#include<openssl/md5.h>
#include<stdlib.h>
#include"network.h"

#define CLIENT_ID "9d2223ab8e334c92bf2584a1e9a9516b"
#define CLIENT_SECRET "ad81374363cf4a6ebd9aad69027615d5"
#define CODE "1449649"
#define TOKEN "AgAAAAAJfAwtAAaSXZEN657D4ETDiWSPzkL4oDE"
//#define MAXLINE 5000
//#define HEADER_LEN 1000

#define S_ITEM_LEN 256
#define MAX_NODES 300 
#define MAX_LEAFS 300 
#define INIT_QUEUE_SIZE 15

#define FIFO_PATH "./fifo"
#define DOWNLOAD_PATH "/home/roman/Documents/clang/yandexDiskWebDav/build"
#define MAX_PATH_LEN 500

#define MD5_UPDATE_LEN 1024 

//TODO: char href[S_ITEM_LEN] - S_ITEM_LEN must be variable
//int href_len
//char href[];
typedef struct QNode {
    char href[S_ITEM_LEN];
    char md5[MD5_DIGEST_LENGTH * 2 + 1];
    long fileLen;
    int isFile;
} QNode;

typedef struct Queue {
    size_t front, rear;
    size_t size, capasity;
    QNode *data;
} Queue;


typedef struct Node {
    struct Node* next;

    char href[S_ITEM_LEN];
} Node;

struct file_system{
    struct item *head;
    int free_space;
    int total_space; 
};

int getToken(void);
//void traverseXML(xmlNode *a_node, struct item *head);
int fileUpload(const char *filePath, const char *remotePath); 
int uploadFile(const char *localPath, const char *remotePath, struct network *net);


void treeTraverse(Node *node);

Queue* initQueue(void);
void destroyQueue(Queue *queue);
int changeQueueSize(Queue *queue);
int addToQueue(Queue *queue, QNode *node);
QNode* getFromQueue(Queue *queue);


int synchronize(const char *rootPath);

#endif
