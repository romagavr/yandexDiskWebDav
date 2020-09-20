#include"client.h"


//TODO: traverse remote filesystem
//TODO: if lastModify > last_traversed_remote
//          add file/collection to download queue
//TODO: set inoify on folder and inner entities
int item_construct(struct file_system *fs){

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
static xmlNode* getFolderXml(const char *folder, struct network *net) {

    char *sendline = malloc(MAXLINE+1);
    snprintf(sendline, MAXLINE,
		"PROPFIND %s HTTP/1.1\r\n"
		"Host: %s\r\n"
        "Accept: */*\r\n"
        "Depth: 1\r\n"
        "Authorization: OAuth %s\r\n\r\n", folder, WHOST, TOKEN);

    int res = send_to(sendline, strlen(sendline), net);
    if (res != E_SUCCESS) {
        return 0;
    }
    struct message *m = (struct message *)net->parser->data;
    if (m->status == 429){
        for (int i=0; i< m->num_headers; i++){
            printf("%s: %s\n", m->headers[i][0], m->headers[i][1]);
        }
        printf("Body: %s\n", m->body);
    }
    //printf("Body: %s\n", m->body);
    printf("Body size: %d\n", m->body_size);
    if (m->status == 404 || m->status == 400)
        return 0;
    if (m->content_length = 0)
        return 0;

    LIBXML_TEST_VERSION
    xmlNode *root_element = 0;
    xmlDoc *doc = 0;
    doc = xmlParseDoc(m->body);
    root_element = xmlDocGetRootElement(doc);

    return root_element;
}

static Leaf* createNewLeaf(void){
    Leaf *leaf = malloc(sizeof *leaf);
    memset(leaf, 0, sizeof *leaf);
    leaf->info = malloc(sizeof *leaf->info);
    memset(leaf->info, 0, sizeof *leaf->info);
    leaf->fileinfo = malloc(sizeof *leaf->fileinfo);
    memset(leaf->fileinfo, 0, sizeof *leaf->fileinfo);
    return leaf;
}

static void parseXML(xmlNode *a_node, Node *node, Leaf *leaf) {
    int resp = 0;
    int countResp = -1;
    static char isFile = 0;
    if (node == 0){
        //TODO: exit
    }
    if (leaf == 0){
        leaf = createNewLeaf();
    }

    for (xmlNode *xmlnd = a_node; xmlnd; xmlnd = xmlnd->next) {
        if (xmlnd->type == XML_ELEMENT_NODE){
            if (strcmp(xmlnd->name, "response") == 0) { 
                countResp++;
                if (countResp == 0) continue;
                resp = 1;
            } else if (strcmp(xmlnd->name, "href") == 0) { 
                strcpy(leaf->info->href, xmlNodeGetContent(xmlnd));
            } else if (strcmp(xmlnd->name, "creationdate") == 0) { 
                strcpy(leaf->info->creationdate, xmlNodeGetContent(xmlnd));
            } else if (strcmp(xmlnd->name, "displayname") == 0) { 
                strcpy(leaf->info->displayname, xmlNodeGetContent(xmlnd));
            } else if (strcmp(xmlnd->name, "getlastmodified") == 0) { 
                strcpy(leaf->info->getlastmodified, xmlNodeGetContent(xmlnd));
            } else if (strcmp(xmlnd->name, "file_url") == 0) { 
                if (!isFile) isFile = 1;
                strcpy(leaf->fileinfo->file_url, xmlNodeGetContent(xmlnd));
            } else if (strcmp(xmlnd->name, "getetag") == 0) { 
                if (!isFile) isFile = 1;
                strcpy(leaf->fileinfo->getetag, xmlNodeGetContent(xmlnd));
            } else if (strcmp(xmlnd->name, "mulca_file_url") == 0) { 
                if (!isFile) isFile = 1;
                strcpy(leaf->fileinfo->mulca_file_url, xmlNodeGetContent(xmlnd));
            } else if (strcmp(xmlnd->name, "mulca_digest_url") == 0) { 
                if (!isFile) isFile = 1;
                strcpy(leaf->fileinfo->mulca_digest_url, xmlNodeGetContent(xmlnd));
            } else if (strcmp(xmlnd->name, "getcontenttype") == 0) { 
                if (!isFile) isFile = 1;
                strcpy(leaf->fileinfo->getcontenttype, xmlNodeGetContent(xmlnd));
            } else if (strcmp(xmlnd->name, "getcontentlength") == 0) { 
                if (!isFile) isFile = 1;
                strcpy(leaf->fileinfo->getcontentlength, xmlNodeGetContent(xmlnd));
            }
        }
        parseXML(xmlnd->children, node, leaf);
        if (resp == 1) {
            if (isFile){
                node->leafs[node->leafs_count] = leaf;
                node->leafs_count++;

                leaf = createNewLeaf();
                isFile = 0;
            } else {
                Node *n = malloc(sizeof *n);
                memset(n, 0, sizeof *n);
                n->info = leaf->info;
                node->nodes[node->nodes_count] = n;
                node->nodes_count++;

                leaf->info = malloc(sizeof *leaf->info);
            }
        }
    }
    //TODO: this is huina
    if (resp == 1){
        free(leaf->fileinfo);
        free(leaf->info);
        free(leaf);
    }
}

static void createFolderNode(Node *node, struct network *net) {
    //TODO: errorChecking
    //logMessage(node->info->href);
    xmlNode* root = getFolderXml(node->info->href, net);
    parseXML(root, node, 0); 
    xmlFreeNode(root);
    for(size_t i = 0; i < node->nodes_count; i++) {
        createFolderNode(node->nodes[i], net);
    }
}

Node* getRemoteFSTree(const char *rootPath, struct network *net){
    Node *root = malloc(sizeof *root);
    memset(root, 0, sizeof *root);
    root->info = malloc(sizeof *root->info);
    memset(root->info, 0, sizeof *root->info);
    //TODO: memcpy or strcpy ?? and in traverseXML too
    memcpy(root->info->href, rootPath, strlen(rootPath) + 1);

    createFolderNode(root, net);

    treeTraverse(root);

    return root;
}

void printLeaf(Leaf *leaf, int tab){
    printf("%*sFile path: %s\n", tab, " ", leaf->info->href);
    printf("%*s---Name: %s\n", tab, " ", leaf->info->displayname);
    printf("%*s---Creation date: %s\n", tab, " ", leaf->info->creationdate);
    printf("%*s---Last Modified: %s\n", tab, " ", leaf->info->getlastmodified);
    printf("%*s---File Url: %s\n", tab, " ", leaf->fileinfo->file_url);
    printf("%*s---Etag: %s\n", tab, " ", leaf->fileinfo->getetag);
    printf("%*s---Mulca file url: %s\n", tab, " ", leaf->fileinfo->mulca_file_url);
    printf("%*s---Content Type: %s\n", tab, " ", leaf->fileinfo->getcontenttype);
    printf("%*s---Content Length: %s\n", tab, " ", leaf->fileinfo->getcontentlength);
    printf("%*s---Mulca digest url: %s\n", tab, " ", leaf->fileinfo->mulca_digest_url);
}

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

void printItems(struct item *head){
    while(head->next != 0){
        if (head->info == 0)
            printf("Folder: %s\n", head->href);
        else
            printf("File: %s\n", head->displayname);
        head = head->next;
    }
}
*/
int getSpaceInfo(struct network *net) {
    char *body = "<?xml version=\"1.0\" ?>"
                 "<D:propfind xmlns:D=\"DAV:\">"
                 "<D:allprop />"
                    //"<D:prop>"
                    //    "<D:quota-available-bytes/>"
                    //    "<D:quota-used-bytes/>"
                    //"</D:prop>"
                 "</D:propfind>";
    
    char *sendline = malloc(MAXLINE+1);
    snprintf(sendline, MAXLINE,
		"PROPFIND / HTTP/1.1\r\n"
		"Host: %s\r\n"
        "Accept: */*\r\n"
        "Depth: 0\r\n"
        "Content-Type: text/xml\r\n"
        "Content-Length: %d\r\n"
        "Authorization: OAuth %s\r\n\r\n%s",  WHOST, strlen(body), TOKEN, body);

    int res = send_to(sendline, strlen(sendline), net);
    if (res != E_SUCCESS) {
        return 0;
    }
    struct message *m = (struct message *)net->parser->data;
    printf("Body: %s\n", m->body);
    if (m->status == 404 || m->status == 400)
        return 0;
}

/*ssize_t getFolderStruct(const char *folder, struct network *net) {
    //getSpaceInfo(net);
    //exit(1);
    char *sendline = malloc(MAXLINE+1);
    snprintf(sendline, MAXLINE,
		"PROPFIND %s HTTP/1.1\r\n"
		"Host: %s\r\n"
        "Accept: *\/*\r\n" //slash UBRAT
        "Depth: 1\r\n"
        "Authorization: OAuth %s\r\n\r\n", folder, WHOST, TOKEN);

    int res = send_to(sendline, strlen(sendline), net);
    if (res != E_SUCCESS) {
        return 0;
    }
    struct message *m = (struct message *)net->parser->data;
    printf("Body: %s\n", m->body);
    if (m->status == 404 || m->status == 400)
        return 0;
    if (m->content_length > 0){
        LIBXML_TEST_VERSION
        xmlNode *root_element = 0;
        xmlDoc *doc = 0;
        doc = xmlParseDoc(m->body);
        root_element = xmlDocGetRootElement(doc);
        //TODO: Надо бы тут сделать парсинг удаленных дирeкторий B-tree??

        struct item *head = malloc(sizeof *head); 
        memset(head, 0, sizeof *head);
        traverseXML(root_element, head);
        printItems(head);
        //print_element_names(root_element);
    }

    return 1;
}*/
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