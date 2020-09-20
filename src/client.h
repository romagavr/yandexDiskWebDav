#include<sys/types.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

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

#define S_ITEM_LEN 200
#define MAX_NODES 300 
#define MAX_LEAFS 300 

struct info {
    char href[S_ITEM_LEN];
    char creationdate[S_ITEM_LEN];
    char displayname[S_ITEM_LEN];
    char getlastmodified[S_ITEM_LEN];
};

typedef struct Node {
    struct info *info;
    
    struct Node* nodes[MAX_NODES];
    struct Leaf* leafs[MAX_LEAFS];

    size_t leafs_count;
    size_t nodes_count;
} Node;

typedef struct Leaf {
    struct info *info;
    struct fileInfo *fileinfo;
} Leaf;


struct fileInfo {
    char file_url[S_ITEM_LEN];
    char getetag[S_ITEM_LEN];
    char mulca_file_url[S_ITEM_LEN];
    char getcontenttype[S_ITEM_LEN];
    char getcontentlength[S_ITEM_LEN];
    char mulca_digest_url[S_ITEM_LEN];
};

struct file_system{
    struct item *head;
    int free_space;
    int total_space; 
};

int getToken(void);
void print_element_names(xmlNode *a_node);
//void traverseXML(xmlNode *a_node, struct item *head);
ssize_t getFolderStruct(const char *folder, struct network *net);
int fileUpload(const char *file, long int file_size, const char *remPath, struct network *net); 
int uploadFile(const char *localPath, const char *remotePath, struct network *net);

static xmlNode* getFolderXml(const char *folder, struct network *net);
static Leaf* createNewLeaf(void);
static void parseXML(xmlNode *a_node, Node *node, Leaf *leaf);
static void createFolderNode(Node *node, struct network *net);
Node* getRemoteFSTree(const char *rootPath, struct network *net);

void treeTraverse(Node *node);