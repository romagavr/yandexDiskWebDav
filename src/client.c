#include<sys/types.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/stat.h>

#include<openssl/ssl.h>
#include<openssl/err.h>
#include<openssl/md5.h>

#include<libxml/parser.h>
#include<libxml/tree.h>

#include"../lib/http-parser/http_parser.h"
#include"network.h"
#include"client.h"
#include"error.h"

static int webdavGet(struct network *net, const char *remotePath, char **resp);
static int webdavGet1(struct network *net, const char *remotePath, char *remoteMD5);
static int webdavPropfind(struct network *net, const char *remotePath, char **resp);

static int estConnection(struct network **net);

static void parseXML(xmlNode *a_node, Node *node, QNode *qnode, int fifo);
static int createFolderNode(Node *node, struct network *net, int fifo);
static char* getMD5sum(const char *path);

static int webdavGet(struct network *net, const char *remotePath, char **resp) {
    const char *req = "GET %s HTTP/1.1\r\n"
		              "Host: %s\r\n"
                      "Accept: */*\r\n"
                      "Authorization: OAuth %s\r\n\r\n";
    ssize_t headerLen = snprintf(NULL, 0, req, remotePath, WHOST, TOKEN) + 1; 
    char *header = malloc(headerLen);
    MALLOC_ERROR_CHECK(header);
    int len = snprintf(header, headerLen, req, remotePath, WHOST, TOKEN);
    if (len < 0 || len >= headerLen)
        return E_SPRINTF;

    Response *r = requestMM(header, net);
    if (r->status == 200) {
        *resp = r->data;
        len = r->dataLen;
    } else len = r->status;
    
    free(header);
    return len;
}

static int webdavGet1(struct network *net, const char *remotePath, char *remoteMD5) {
    char localPath[1000] = DOWNLOAD_PATH;
    memcpy(localPath + strlen(DOWNLOAD_PATH), remotePath, strlen(remotePath) + 1);
    FILE *file = fopen(localPath, "wbx");
    char exist = 0;
    int last;
    char newPath[600];
    if (file == 0) {
        printf("File exists %s\n", remotePath);
        //file exists -> check md5 sum
        if (remoteMD5 != 0) {
            char *md5str = getMD5sum(localPath);
            if (md5str == 0){
                logMessage("Getting MD5sum error");
                return -1111;
            }
            // TODO: check if remoteMD5 is null-term
            printf("remote: %s\n", remoteMD5);
            printf("local: %s\n", md5str);
            if (strcmp(md5str, remoteMD5) == 0) {
                printf("File %s not changed\n", remotePath);
                //file the same
                free(md5str);
                return 111;
            }
        }
        printf("**File %s changed, downloaded\n", remotePath);
        exist = 1;
        memcpy(newPath, localPath, strlen(localPath) + 1);
        last = strrchr(localPath, '/') - localPath + 1;
        memcpy(newPath + last, ".ttmp", strlen(".ttmp") + 1);
        file = fopen(newPath, "wb");
    }

    const char *req = "GET %s HTTP/1.1\r\n"
		              "Host: %s\r\n"
                      "Accept: */*\r\n"
                      "Authorization: OAuth %s\r\n\r\n";
    ssize_t headerLen = snprintf(NULL, 0, req, remotePath, WHOST, TOKEN) + 1; 
    char *header = malloc(headerLen);
    MALLOC_ERROR_CHECK(header);
    int len = snprintf(header, headerLen, req, remotePath, WHOST, TOKEN);
    if (len < 0 || len >= headerLen)
        return E_SPRINTF;

    Response *r = requestMF(header, net, file);

    fclose(file);
    free(header);

    if (exist){
        remove(localPath);
        rename(newPath, localPath);
    }
    return (r->status == 200) ? 1 : r->status;
}

static int webdavPropfind(struct network *net, const char *remotePath, char **resp) {
    const char *req = "PROPFIND %s HTTP/1.1\r\n"
                      "Host: %s\r\n"
                      "Accept: */*\r\n"
                      "Depth: 1\r\n"
                      "Authorization: OAuth %s\r\n\r\n";
    ssize_t headerLen = snprintf(NULL, 0, req, remotePath, WHOST, TOKEN) + 1; 
    char *header = malloc(headerLen);
    MALLOC_ERROR_CHECK(header);
    int len = snprintf(header, headerLen, req, remotePath, WHOST, TOKEN);
    if (len < 0 || len >= headerLen)
        return E_SPRINTF;

    Response *r = requestMM(header, net);
    if (r->status == 200) {
        *resp = r->data;
        len = r->dataLen;
    } else len = r->status;

    free(header);
    return len;
}
/*
int getSpaceInfo(struct network *net) {
    char *body = "<?xml version=\"1.0\" ?>"
                 "<D:propfind xmlns:D=\"DAV:\">"
                 "<D:allprop />"
                    "<D:prop>"
                        "<D:quota-available-bytes/>"
                        "<D:quota-used-bytes/>"
                    "</D:prop>"
                 "</D:propfind>";
    
    char *sendline = malloc(MAXLINE+1);
    snprintf(sendline, MAXLINE,
		"PROPFIND / HTTP/1.1\r\n"
		"Host: %s\r\n"
        "Accept: **\r\n" // не забыть добавить черту
        "Depth: 0\r\n"
        "Content-Type: text/xml\r\n"
        "Content-Length: %ld\r\n"
        "Authorization: OAuth %s\r\n\r\n%s",  WHOST, strlen(body), TOKEN, body);

    int res = send_to(sendline, net, 0);
    free(sendline);
    if (res != E_SUCCESS) {
        return 0;
    }
    struct message *m = (struct message *)net->parser->data;
    printf("Body: %s\n", m->body);
    if (m->status == 404 || m->status == 400)
        return 0;
    return 0;
}

int getToken(){
    SSL *ssl = 0;
    SSL_CTX *ctx = 0; 
    int socket_peer = 0;

    //estTcpConn(&ssl, &ctx, &socket_peer,  WHOST, "https");

    char sendline[MAXLINE+1];
    char read[MAXLINE+1];
    int bytes_sent, bytes_received; 
*/
    /*snprintf(sendline, MAXLINE,
		"GET /authorize?response_type=code&client_id=%s HTTP/1.1\r\n"
		"Host: %s\r\n\r\n", CLIENT_ID, HOST);
    bytes_sent = SSL_write(ssl, sendline, strlen(sendline));
    //printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(sendline));

    bytes_received = SSL_read(ssl, read, 10000);
    if (bytes_received < 1) 
	    printf("Connection closed by peer.\n");
    printf("Received (%d bytes): %.*s", bytes_received, bytes_received, read);*/
/*
    const char *body = "grant_type=authorization_code&code="CODE
                       "&client_id="CLIENT_ID"&client_secret="CLIENT_SECRET;
    snprintf(sendline, MAXLINE,
		"POST /token http/1.1\r\n"
		"Host: %s\r\n"
        "Content-type: application/x-www-form-urlencoded\r\n"
        "Content-length: %d\r\n\r\n%s", HOST, (int)strlen(body), body);
    //printf("\n%s\n", sendline);
    bytes_sent = SSL_write(ssl, sendline, strlen(sendline));
    printf("Sent %d of %d bytes.\n", bytes_sent, (int)strlen(sendline));

    bytes_received = SSL_read(ssl, read, 10000);
    if (bytes_received < 1) 
	    printf("Connection closed by peer.\n");
    printf("Received (%d bytes): %.*s", bytes_received, bytes_received, read);

    printf("Closing socket...\n");
    SSL_free(ssl);
    close(socket_peer);
    SSL_CTX_free(ctx);

    return 0;
}*/

Queue* initQueue(void){
    Queue *queue = malloc(sizeof *queue);
    memset(queue, 0, sizeof *queue);
    queue->data = malloc(INIT_QUEUE_SIZE * sizeof(QNode));
    memset(queue->data, 0, sizeof *queue->data);
    queue->capasity = INIT_QUEUE_SIZE;
    return queue;
}

void destroyQueue(Queue *queue) {
    free(queue->data);
    free(queue); 
}

//TODO: realloc ??? - обработка выделения
int changeQueueSize(Queue *queue) {
    QNode *new = realloc(queue->data, queue->capasity * 2);
    if (new == 0){
        return 1;
    }
    queue->data = new;
    queue->capasity *= 2;
    return 0;
}

int addToQueue(Queue *queue, QNode *node) {
    if (queue->size == queue->capasity)
        return 1;
    if (queue->size == 0){
        queue->rear = -1;
        queue->front = 0;
    };
    queue->rear = (queue->rear + 1) % queue->capasity;
    queue->size++;
    memset(&(queue->data[queue->rear]), 0, sizeof queue->data[queue->rear]);
    memcpy((queue->data[queue->rear]).href, node->href, strlen(node->href));
    memcpy((queue->data[queue->rear]).md5, node->md5, strlen(node->md5));
    (queue->data[queue->rear]).isFile = node->isFile;
    (queue->data[queue->rear]).fileLen = node->fileLen;
    return 0;
}

QNode* getFromQueue(Queue *queue) {
    if (queue->size == 0)
        return 0;
    size_t t = queue->front;
    //printf("Getting: %d\n", t);
    queue->front = (queue->front + 1) % queue->capasity;
    //printf("New front: %d\n", queue->front);
    queue->size--;
    return &queue->data[t];
}

static void parseXML(xmlNode *a_node, Node *node, QNode *qnode, int fifo) {
    if (!qnode){
        qnode = malloc(sizeof *qnode);
        MALLOC_ERROR_CHECK(qnode);
    }

    int resp = (strcmp((const char *)a_node->name, "response") == 0) ? 1 : 0;

    for (xmlNode *xmlnd = a_node; xmlnd; xmlnd = xmlnd->next) {
        if (resp && xmlnd->children){
            memset(qnode, '\0', sizeof *qnode);
            parseXML(xmlnd->children, node, qnode, fifo);

            if (xmlnd != a_node) {
                //Eror checking
                write(fifo, qnode, sizeof *qnode);
                if (!qnode->isFile) {
                    Node *n = malloc(sizeof *n);
                    memset(n, '\0', sizeof *n);
                    strcpy(n->href, qnode->href);
                    n->next = node->next;
                    node->next = n;
                }
            }
        } else {
            if (strcmp((const char *)xmlnd->name, "href") == 0) { 
                strcpy((char *)qnode->href, (const char *)xmlNodeGetContent(xmlnd));
            } else if (strcmp((const char *)xmlnd->name, "getetag") == 0) { 
                strcpy((char *)qnode->md5, (const char *)xmlNodeGetContent(xmlnd));
            } else if (strcmp((const char *)xmlnd->name, "getcontentlength") == 0) { 
                qnode->fileLen = atol((const char *)xmlNodeGetContent(xmlnd));
            } else if (strcmp((const char *)xmlnd->name, "resourcetype") == 0 && !xmlnd->children) { 
                qnode->isFile = 1;
            }

            if (xmlnd->children)
                parseXML(xmlnd->children, node, qnode, fifo);
        }
    }

    if (resp) free(qnode);
}

static int createFolderNode(Node *node, struct network *net, int fifo) {
    if (!node)
        return E_SYNC_ERROR;

    char *body = 0;
    int res = 0;
    if ((res = webdavPropfind(net, node->href, &body)) < 0)
        return res;
    //printf("%.10s\n", body);
    //exit(EXIT_FAILURE);
    // TODO: Free *doc
    //
    LIBXML_TEST_VERSION
    xmlDoc *doc = xmlParseDoc((const unsigned char*)body);
    free(body);
    xmlNode *root = xmlDocGetRootElement(doc);
    parseXML(root, node, 0, fifo); 
    xmlFreeNode(root);

    Node *tnode = 0;
    while ((tnode = node->next)) {
       if ((res = createFolderNode(tnode, net, fifo)) < 0)
            logSyncErr(tnode->href, res);
       node->next = tnode->next;
       free(tnode);
    }
    return 0;
}

static char* getMD5sum(const char *path){
    FILE *filefd = fopen(path, "rb");
    if (filefd == 0){
        fprintf(stderr, "fopen failed. (%d)\n", errno);
        return 0;
    }

    char *md5_string = malloc(MD5_DIGEST_LENGTH * 2 + 1);
    MALLOC_ERROR_CHECK(md5_string);

    MD5_CTX md5;
    if (!MD5_Init(&md5)) return 0;

    size_t bytes;
    char raw[MD5_UPDATE_LEN];
    while ((bytes = fread(raw, 1, MD5_UPDATE_LEN, filefd)))
        MD5_Update(&md5, raw, bytes);
    fclose(filefd);

    unsigned char md5_hash[MD5_DIGEST_LENGTH];
    if (!MD5_Final(md5_hash, &md5)) return 0;

    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
        sprintf(&md5_string[i*2], "%02x", (unsigned int)md5_hash[i]);
    md5_string[MD5_DIGEST_LENGTH * 2] = '\0';
    return md5_string;
}

static size_t getShaMD5(const char *path, char **sha256S, char **md5S){
    FILE *fd = fopen(path, "rb");
    if (!fd) return 0;

    MD5_CTX md5;
    SHA256_CTX sha;
    if (!MD5_Init(&md5) || !SHA256_Init(&sha)) return 0;

    size_t bytes, total=0;
    unsigned char raw[MD5_UPDATE_LEN];
    while ((bytes = fread(raw, 1, MD5_UPDATE_LEN, fd))) {
        if (!MD5_Update(&md5, raw, bytes) || !SHA256_Update(&sha, raw, bytes)) {
            fclose(fd);
            return 0;
        }
        total+=bytes;
    }
    fclose(fd);

    unsigned char md5_hash[MD5_DIGEST_LENGTH];
    unsigned char sha256[SHA256_DIGEST_LENGTH];
    if (!MD5_Final(md5_hash, &md5) || !SHA256_Final(sha256, &sha)) return 0;

    char *md5_string = malloc(MD5_DIGEST_LENGTH * 2 + 1);
    MALLOC_ERROR_CHECK(md5_string);
    char *sha256_string = malloc(SHA256_DIGEST_LENGTH * 2 + 1);
    MALLOC_ERROR_CHECK(sha256_string);

    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
        sprintf(&md5_string[i*2], "%02x", (unsigned int)md5_hash[i]);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        sprintf(&sha256_string[i*2], "%02x", (unsigned int)sha256[i]);
    md5_string[MD5_DIGEST_LENGTH * 2] = sha256_string[SHA256_DIGEST_LENGTH*2] = '\0';
    *sha256S = sha256_string;
    *md5S = md5_string;

    return total;
}

static int getFileRaw(const char *path, char **out){
    FILE *filefd = fopen(path, "rb");
    if (filefd == 0){
        fprintf(stderr, "fopen failed. (%d)\n", errno);
        return 0;
    }
    fseek(filefd, 0, SEEK_END);
    int filesize = ftell(filefd);
    if (filesize == -1){
        fclose(filefd);
        fprintf(stderr, "filesize getting error. (%d)\n", errno);
        return 0;
    }
    rewind(filefd);
    char *raw = malloc(filesize);
    if (raw == 0){
        fprintf(stderr, "Malloc() failed. (%d)\n", errno);
        return 0;
    }
    int res = fread(raw, 1, filesize, filefd);
    printf("raw %d\n", filesize);
    fclose(filefd);

    if (res != filesize) {
        free(raw);
        fprintf(stderr, "fread error. (%d)\n", errno);
        return 0;
    }
    *out = raw;
    return filesize;
}

int saveFiles(struct network *net, int fifo){
    Queue *q = initQueue();
    QNode *n = 0;
    QNode tmpn = {0};
    char path[MAX_PATH_LEN] = DOWNLOAD_PATH; 
    int res;

    for (;;) {
        while (read(fifo, &tmpn, sizeof tmpn) > 0) {
            addToQueue(q, &tmpn);
        } 
        if (q->size == 0)
            break;

        //Path: /home/roman/Documents/clang/yandexDiskWebDav/build/test/T_kormen_Ch_leyzerson_R_rivest_K_shtayn_-_Algoritmy_Postroenie_I_Analiz_-_2013.pdf
        //New path: /home/roman/Documents/clang/yandexDiskWebDav/build/test/.ttmp
        //Name: T_kormen_Ch_leyzerson_R_rivest_K_shtayn_-_Algoritmy_Postroenie_I_Analiz_-_2013.pdf
        n = getFromQueue(q);
        if (n->isFile){
            res = webdavGet1(net, n->href, n->md5);
            if (res<=0) {
                printf("Error\n");
                break;
            }
            printf("Created file: %s\n", n->href);
        } else {
            memcpy(path + strlen(DOWNLOAD_PATH), n->href, strlen(n->href) + 1);
            if (mkdir(path, 0777) == -1){
                if (errno == EEXIST)
                    logMessage("Folder exists");
                else 
                    logErrno("Creating folder error");
            } else logMessage("OK");
        }
     }

    destroyQueue(q);
    return 0;
}


static int estConnection(struct network **net) {
        int res;
        if ((res = initNetworkStruct(net)) != E_SUCCESS)
            return res;

        return connect_to(*net,  WHOST);
};

int synchronize(const char *rootPath){
    int fd_fifo;
    int res;
    struct network *net = 0;

    if (mkfifo(FIFO_PATH, 0777) != 0){
        fprintf(stderr, "Error while creating FIFO; (%d): %s\n", errno, strerror(errno));
        return 1;
    };

    char path[MAX_PATH_LEN] = {'\0'};
    strcat(path, DOWNLOAD_PATH);
    strcat(path, rootPath);
    if (mkdir(path, 0777))
        logErrno("Creating folder error");

    switch (fork())
    {
    case -1:
        /* errror */
        break;
    case 0:
        if ((res = estConnection(&net) != E_SUCCESS)) {
            logLibError(res, 0);
            return res;
        } 
        if((fd_fifo=open(FIFO_PATH, O_RDONLY)) == - 1){
            logLibError(E_FIFO_OPEN, 1);
            return E_FIFO_OPEN;
        }

        saveFiles(net, fd_fifo);

        close(fd_fifo);
        freeNetworkStruct(net);

        exit(0);
        break; 
    default:
        if ((res = estConnection(&net) != E_SUCCESS)) {
            logLibError(res, 0);
            return res;
        }     
        if((fd_fifo=open(FIFO_PATH, O_WRONLY)) == -1){
            logLibError(E_FIFO_OPEN, 1);
            //return E_FIFO_OPEN;
        }
        Node *root = malloc(sizeof *root);
        root->next = 0;
        memset(root->href, '\0', sizeof *root->href);
        strcpy(root->href, rootPath);

        if ((res = createFolderNode(root, net, fd_fifo)) < 0)
            logSyncErr(root->href, res);

        free(root);
        close(fd_fifo);
        freeNetworkStruct(net);

        int status;
        int pid = wait(&status);
        printf("Pid: %d\n", pid);
        break;
    }

    //treeTraverse(root);
    if (unlink(FIFO_PATH) != 0) {
        fprintf(stderr, "Error while closing FIFO; (%d): %s\n", errno, strerror(errno));
        return 1;
    }
    return 0;
}

static char* getSha256(const char *path){
    FILE *fd = fopen(path, "rb");
    if (!fd){
        fprintf(stderr, "fopen failed. (%d)\n", errno);
        return 0;
    }

    char sha256[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha;
    if (!SHA256_Init(&sha)) return 0;
        
    size_t bytes;
    char raw[MD5_UPDATE_LEN];
    while ((bytes = fread(raw, 1, MD5_UPDATE_LEN, fd)))
        if (!SHA256_Update(&sha, raw, bytes)) return 0;
    fclose(fd);

    if (!SHA256_Final(sha256, &sha)) return 0;

    char *sha256_string = malloc(SHA256_DIGEST_LENGTH * 2 + 1);
    MALLOC_ERROR_CHECK(sha256_string);

    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        sprintf(&sha256_string[i*2], "%02x", (unsigned int)sha256[i]);

    return sha256_string;
}

int fileUpload(const char *filePath, const char *remotePath) {
    int res;
    struct network *net = 0;
    if ((res = estConnection(&net) != E_SUCCESS)) {
        logLibError(res, 0);
        return res;
    } 

    char *sha = 0, *md5 = 0;
    size_t fsize = getShaMD5(filePath, &sha, &md5);
    if (!fsize) return 0;
    //printf("%s : %s : %d", sha, md5, fsize);

    const char *req = "PUT %s HTTP/1.1\r\n"
                      "Host: %s\r\n"
                      "Accept: */*\r\n"
                      "Authorization: OAuth %s\r\n"
                      "Etag: %s\r\n"
                      "Sha256: %s\r\n"
                      "Expect: 100-continue\r\n"
                      "Content-Type: application/binary\r\n"
                      "Content-Length: %d\r\n\r\n";
    ssize_t headerLen = snprintf(NULL, 0, req, remotePath, WHOST, TOKEN, md5, sha, fsize) + 1; 
    char *header = malloc(headerLen);
    MALLOC_ERROR_CHECK(header);
    ssize_t len = snprintf(header, headerLen, req, remotePath, WHOST, TOKEN, md5, sha, fsize) + 1; 
    free(md5);
    free(sha);
    if (len < 0 || len > headerLen)
        return E_SPRINTF;

    Response *r = requestMM(header, net);
    free(header);
    printf("%d\n", r->status);
    exit(EXIT_SUCCESS);

    switch (r->status) {
        //file exists
        case 201:
            return  123;

        //continue upload 
        case 100:
            printf("");
            FILE *fd = open(filePath, "rb");
            if (!fd) printf("error\n");
            r = requestFM(fd, net);
            if (r->status != 200) {
                //error
            }
            fclose(fd);
            break;

        //error
        default:
            break;
    }
    return 0;
}

/*
void treeTraverse(Node *node){
    static int tab = 1;
    printf("%*sFolder path: %s\n", tab, " ", node->info->href);
    printf("%*s---Name: %s\n", tab, " ", node->info->displayname);
    printf("%*s---Creation date: %s\n", tab, " ", node->info->creationdate);
    printf("%*s---Last Modified: %s\n", tab, " ", node->info->getlastmodified);
    for (int i = 0; i < node->leafs_count; i++){
        printLeaf(node->leafs[i], tab);
    }
    tab *= 2;
    for (int i = 0; i < node->nodes_count; i++){
        treeTraverse(node->nodes[i]);
    }
}
*/
///
/*
void traverseXML(xmlNode *a_node, struct item *head) {
    static int count = 0;
    for (xmlNode *node = a_node; node; node = node->next) {
        if (node->type == XML_ELEMENT_NODE){
            if (strcmp(node->name, "response") == 0) { 
                if (count != 0) {
                    struct item *tmp = malloc(sizeof *tmp); 
                    memset(tmp, 0, sizeof *tmp);
                    head->next = tmp;
                    head = tmp;
                }
                count++;
            } else if (strcmp(node->name, "href") == 0) { 
                strcpy(head->href, xmlNodeGetContent(node));
            } else if (strcmp(node->name, "creationdate") == 0) { 
                strcpy(head->creationdate, xmlNodeGetContent(node));
            } else if (strcmp(node->name, "displayname") == 0) { 
                strcpy(head->displayname, xmlNodeGetContent(node));
            } else if (strcmp(node->name, "getlastmodified") == 0) { 
                strcpy(head->getlastmodified, xmlNodeGetContent(node));
            } else if (strcmp(node->name, "file_url") == 0) { 
                if (head->info == 0) {
                    head->info = malloc(sizeof(head->info));
                }
                strcpy(head->info->file_url, xmlNodeGetContent(node));
            } else if (strcmp(node->name, "getetag") == 0) { 
                if (head->info == 0) {
                    head->info = malloc(sizeof(head->info));
                }
                strcpy(head->info->getetag, xmlNodeGetContent(node));
            } else if (strcmp(node->name, "mulca_file_url") == 0) { 
                if (head->info == 0) {
                    head->info = malloc(sizeof(head->info));
                }
                strcpy(head->info->mulca_file_url, xmlNodeGetContent(node));
            } else if (strcmp(node->name, "mulca_digest_url") == 0) { 
                if (head->info == 0) {
                    head->info = malloc(sizeof(head->info));
                }
                strcpy(head->info->mulca_digest_url, xmlNodeGetContent(node));
            } else if (strcmp(node->name, "getcontenttype") == 0) { 
                if (head->info == 0) {
                    head->info = malloc(sizeof(head->info));
                }
                strcpy(head->info->getcontenttype, xmlNodeGetContent(node));
            } else if (strcmp(node->name, "getcontentlength") == 0) { 
                if (head->info == 0) {
                    head->info = malloc(sizeof(head->info));
                }
                strcpy(head->info->getcontentlength, xmlNodeGetContent(node));
            }
        }
        traverseXML(node->children, head);
    }
}
*/