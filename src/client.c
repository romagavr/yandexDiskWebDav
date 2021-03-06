#include"client.h"

static int webdavGet(struct network *net, const char *filepath, char **file);
static int webdavPropfind(struct network *net, const char *filepath, char **file);

static int estConnection(struct network **net);

static void parseXML(xmlNode *a_node, Node *node, QNode *qnode, int fifo);
static void createFolderNode(Node *node, struct network *net, int fifo);


static int webdavGet(struct network *net, const char *filepath, char **file) {
    const char *req = "GET %s HTTP/1.1\r\n"
		              "Host: %s\r\n"
                      "Accept: */*\r\n"
                      "Authorization: OAuth %s\r\n\r\n";
    ssize_t headerLen = snprintf(NULL, 0, req, filepath, WHOST, TOKEN) + 1; 
    char *header = 0;
    int isMall = 0;
    if (HEADER_LEN < headerLen) {
        header = malloc(headerLen);
        MALLOC_ERROR_CHECK(header);
        isMall = 1;
    } else {
        header = alloca(headerLen);
    }

    int len = snprintf(header, headerLen, req, filepath, WHOST, TOKEN);
    if (len < 0 || len >= headerLen)
        return E_SPRINTF;

    int res = send_to(header, len, net);
    if (isMall)
        free(header);
    
    if (res != E_SUCCESS)
        return res;

    struct message *m = (struct message *)net->parser->data;
    *file = m->body;

    return m->parsed_length;
}

static int webdavPropfind(struct network *net, const char *filepath, char **file) {
    const char *req = "PROPFIND %s HTTP/1.1\r\n"
                      "Host: %s\r\n"
                      "Accept: */*\r\n"
                      "Depth: 1\r\n"
                      "Authorization: OAuth %s\r\n\r\n";
    ssize_t headerLen = snprintf(NULL, 0, req, filepath, WHOST, TOKEN) + 1; 
    char *header = 0;
    int isMall = 0;
    if (HEADER_LEN < headerLen) {
        header = malloc(headerLen);
        MALLOC_ERROR_CHECK(header);
        isMall = 1;
    } else {
        header = alloca(headerLen);
    }

    int len = snprintf(header, headerLen, req, filepath, WHOST, TOKEN);
    if (len < 0 || len >= headerLen)
        return E_SPRINTF;

    int res = send_to(header, len, net);
    if (isMall)
        free(header);
    
    if (res != E_SUCCESS)
        return res;

    struct message *m = (struct message *)net->parser->data;
    *file = m->body;

    return m->parsed_length;
}

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
        "Accept: */*\r\n"
        "Depth: 0\r\n"
        "Content-Type: text/xml\r\n"
        "Content-Length: %ld\r\n"
        "Authorization: OAuth %s\r\n\r\n%s",  WHOST, strlen(body), TOKEN, body);

    int res = send_to(sendline, strlen(sendline), net);
    if (res != E_SUCCESS) {
        return 0;
    }
    struct message *m = (struct message *)net->parser->data;
    printf("Body: %s\n", m->body);
    if (m->status == 404 || m->status == 400)
        return 0;
    return 0;
}
/*

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
    (queue->data[queue->rear]).isFile = node->isFile;
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
    if (qnode == 0){
        qnode = malloc(sizeof *qnode);
    }

    int resp = (strcmp((const char *)a_node->name, "response") == 0) ? 1 : 0;

    for (xmlNode *xmlnd = a_node; xmlnd; xmlnd = xmlnd->next) {
        if (resp && xmlnd->children){
            memset(qnode, '\0', sizeof *qnode);
            parseXML(xmlnd->children, node, qnode, fifo);

            write(fifo, qnode, sizeof *qnode);

            if (xmlnd != a_node && !qnode->isFile) {
                Node *n = malloc(sizeof *n);
                memset(n, 0, sizeof *n);
                strcpy(n->href, qnode->href);

                node->nodes[node->nodes_count] = n;
                node->nodes_count++;
            }
        } else {
            if (strcmp((const char *)xmlnd->name, "href") == 0) { 
                strcpy((char *)qnode->href, (const char *)xmlNodeGetContent(xmlnd));
            } else if (strcmp((const char *)xmlnd->name, "resourcetype") == 0 && !xmlnd->children) { 
                qnode->isFile = 1;
            }

            if (xmlnd->children)
                parseXML(xmlnd->children, node, qnode, fifo);
        }
    }

    if (resp) free(qnode);
}

static void createFolderNode(Node *node, struct network *net, int fifo) {
    if (node == 0) {
        node = malloc(sizeof *node);
        memset(node, 0, sizeof *node);
        strcpy(node->href, "/");
    }

    char *body = 0;
    int res = webdavPropfind(net, node->href, &body);
    if (res < 0) {
        logLibError(res, 0);
        return;
    }

    // TODO: Free *doc
    //
    LIBXML_TEST_VERSION
    xmlDoc *doc = xmlParseDoc((const unsigned char*)body);
    xmlNode *root = xmlDocGetRootElement(doc);
    parseXML(root, node, 0, fifo); 
    xmlFreeNode(root);

    for(size_t i = 0; i < node->nodes_count; i++) {
        createFolderNode(node->nodes[i], net, fifo);
        // TODO: delete node[i]
    }
}

int saveFiles(struct network *net, int fifo){
    Queue *q = initQueue();
    QNode *n = 0;
    QNode tmpn = {0};
    char *file = 0;
    char path[MAX_PATH_LEN] = DOWNLOAD_PATH; 
    int fsize;
    int crFd;
    int len = 0;

    for (;;) {
        while (read(fifo, &tmpn, sizeof tmpn) > 0) {
            addToQueue(q, &tmpn);
        } 
        if (q->size == 0)
            break;

        n = getFromQueue(q);
        memset(path + strlen(DOWNLOAD_PATH), '\0', MAX_PATH_LEN - strlen(DOWNLOAD_PATH));
        strcat(path, n->href);
        if (n->isFile){
            if ((fsize = webdavGet(net, n->href, &file)) < 0) {
                logLibError(fsize, 0);
                continue; 
            }
            if ((crFd = open(path, O_CREAT | O_WRONLY, 0777)) == -1){
                logErrno("Open file to write");
                continue;
            }
            len = 0;
            while (len != fsize) {
                if ((len = write(crFd, file, fsize)) == -1) {
                    logErrno("Writing to file error");
                }
            }
            printf("Created file: %s (%d bytes)\n", path, fsize);
            close(crFd);
        } else {
            if (mkdir(path, 0777) == -1)
                logErrno("Creating folder error");
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
        memset(root, '\0', sizeof *root);
        strcpy(root->href, rootPath);

        createFolderNode(root, net, fd_fifo);

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
/*
int fileUpload(const char *file, long int file_size, const char *remPath, struct network *net) {
    SSL *ssl = net->ssl;

    unsigned char md5_hash[MD5_DIGEST_LENGTH];
    char md5_string[MD5_DIGEST_LENGTH * 2 + 1];
    MD5_CTX md5;
    if (MD5_Init(&md5) == 0){
        fprintf(stderr, "MD5_Init failed. (%d)\n", errno);
        return -1;
    };
    if (MD5_Update(&md5, file, strlen(file)) == 0){
        fprintf(stderr, "MD5_Update failed. (%d)\n", errno);
        return -1;
    };
    if (MD5_Final(md5_hash, &md5) == 0){
        fprintf(stderr, "MD5_Final failed. (%d)\n", errno);
        return -1;
    };
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
        sprintf(&md5_string[i*2], "%02x", (unsigned int)md5_hash[i]);

    char sha256[SHA256_DIGEST_LENGTH];
    char sha256_string[SHA256_DIGEST_LENGTH * 2 + 1];
    SHA256_CTX sha;
    if (SHA256_Init(&sha) == 0){
        fprintf(stderr, "SHA256_Init failed. (%d)\n", errno);
        return -1;
    };
    if (SHA256_Update(&sha, file, strlen(file)) == 0){
        fprintf(stderr, "SHA256_Update failed. (%d)\n", errno);
        return -1;
    };
    if (SHA256_Final(sha256, &sha) == 0){
        fprintf(stderr, "SHA256_Final failed. (%d)\n", errno);
        return -1;
    };
    // TODO: как это получается
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        sprintf(&sha256_string[i*2], "%02x", (unsigned int)sha256[i]);

    const char *req = "PUT %s HTTP/1.1\r\n"
                      "Host: %s\r\n"
                      "Accept: *\/*\r\n" // убрать \
                      "Authorization: OAuth %s\r\n"
                      "Etag: %s\r\n"
                      "Sha256: %s\r\n"
                      "Expect: 100-continue\r\n"
                      "Content-Type: application/binary\r\n"
                      "Content-Length: %d\r\n\r\n";

    ssize_t headerLen = snprintf(NULL, 0, req, remPath, WHOST, TOKEN, md5_string, sha256_string, file_size);
    headerLen++; //For '\0'
    char *header = 0;
    if (HEADER_LEN < headerLen) {
        header = malloc(headerLen);
        if (header == 0){
            fprintf(stderr, "Malloc() failed. (%d)\n", errno);
            return -1;
        }
    } else {
        header = alloca(headerLen);
    }
    
    // TODO Обработка ошибок
    snprintf(header, headerLen, req, remPath, WHOST, TOKEN, md5_string, sha256_string, file_size);
    size_t packetLen = headerLen + file_size - 1;
    char *packet = 0;
    if (MAXLINE < packetLen) {
        packet = malloc(packetLen);
        if (packet == 0){
            fprintf(stderr, "Malloc() failed. (%d)\n", errno);
            return -1;
        }
    } else {
        packet = alloca(packetLen);
    }
    memcpy(packet, header, headerLen);
    memcpy(packet + headerLen, file, file_size);

    // TODO обработка ошибок чтения/записи в сокет
    // TODO парсинг ответа
    //  https://github.com/nodejs/http-parser
    char *read = malloc(MAXLINE+1);
    if (read == 0)
        return -1;

    ssize_t bytes_reseived = socketWrite(packet, packetLen, net);

    ////// 
    int bytes_sent, bytes_received; 

    bytes_sent = SSL_write(ssl, packet, packetLen);
    bytes_received = SSL_read(ssl, read, MAXLINE);
    printf("Received (%d bytes): %.*s", bytes_received, bytes_received, read);
    if (bytes_received < 1) 
	    printf("Connection closed by peer.\n");
    ////////

    return 0;
}


int uploadFile(const char *localPath, const char *remotePath, struct network *net){

    getFolderStruct(remotePath, net);
    //TODO: check remote path
    FILE *fd = fopen(localPath, "rb");
    if (fd == 0){
        fprintf(stderr, "fopen failed. (%d)\n", errno);
        return -1;
    }

    fseek(fd, 0, SEEK_END);
    long int file_size = ftell(fd);
    if (file_size == -1){
        fclose(fd);
        fprintf(stderr, "filesize getting error. (%d)\n", errno);
        return -1;
    }
    rewind(fd);
    unsigned char *file = malloc(file_size);
    if (file == 0){
        fclose(fd);
        fprintf(stderr, "Malloc() failed. (%d)\n", errno);
        return -1;
    }

    //TODO: Check bounds of size_t and long int
    size_t res = fread(file, 1, file_size, fd);
    fclose(fd);
    if (res != file_size) {
        free(file);
        fprintf(stderr, "fread error. (%d)\n", errno);
        return -1;
    }

    //TODO: Check is it folder of file
    int pos = 0;
    for (int i=0; i<strlen(localPath); i++){
        if (localPath[i] == '/')
            pos = i+1;
    }
    char dst[10];
    size_t dstPathLen = strlen(remotePath);
    memcpy(dst, remotePath, dstPathLen);
    if (dst[dstPathLen - 1] != '/'){
        dst[dstPathLen] = '/';
        dstPathLen++;
    }
    memcpy(dst + dstPathLen, localPath + pos, strlen(localPath) - pos);

/////
    printf("\n%s\n", dst);
    for (int i=0; i <= strlen(dst); i++){
        if (dst[i] == '\0')
            printf("%c - null\n", dst[i]); 
        printf("%c ", dst[i]); 
    }
    exit(EXIT_FAILURE);
/////

    //TODO: Manual set name of remote file
    int result = fileUpload(file, res, dst, net);
    free(file);
    if (result == -1) {
        fprintf(stderr, "File upload error.\n");
        return -1;
    }
    return 0;
}
*/