#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
#include<unistd.h>

int downloadFile(struct network *net, const char *href, char *path, char ifExist){
    char pathForWrite[strlen(path) + 10];
    strcpy(pathForWrite, path);
    if (ifExist) {
        char *pos = strchr(pathForWrite, '/');
        if (pos == 0) 
            return -1;
        *(pos + 1) = '\0';
        strcat(pathForWrite, ".tmpfile");
    }

    const char *req = "GET %s HTTP/1.1\r\n"
		              "Host: %s\r\n"
                      "Accept: */*\r\n"
                      "Authorization: OAuth %s\r\n\r\n";
    ssize_t headerLen = snprintf(NULL, 0, req, href, WHOST, TOKEN) + 1; 
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
        if ((fsize = webdavGet(net, n->href, &file)) < 0) {
            logLibError(fsize, 0);
            continue; 
        }
        if ((fdd = open(path, O_CREAT | O_WRONLY, 0777)) == -1){
            logErrno("Open file to write");
            continue;
        }
        len = 0;
        while (len != fsize) {
            if ((len = write(fdd, file, fsize)) == -1) {
                logErrno("Writing to file error");
            }
        }
        printf("Created file: %s (%d bytes)\n", path, fsize);
        close(fdd);
}

typedef struct QNode {
    char md5[MD5_DIGEST_LENGTH * 2 + 1];
    int isFile;
    int hrefLen;
    char href[];
} QNode

int main(){
    return 0;
}