#include<stdlib.h>
#include<stdio.h>
#include<memory.h>

#include<openssl/ssl.h>
#include<openssl/err.h>
#include<openssl/md5.h>

#include<string.h>
#include<fcntl.h>
#include<unistd.h>

static char* getMD5sum(const char *str, long len){
    unsigned char md5_hash[MD5_DIGEST_LENGTH];
    char *md5_string = malloc(MD5_DIGEST_LENGTH * 2 + 1);
    if (md5_string == 0){
        fprintf(stderr, "Malloc failed. (%d)\n", errno);
        return 0;
    }
    MD5_CTX md5;
    if (MD5_Init(&md5) == 0){
        fprintf(stderr, "MD5_Init failed. (%d)\n", errno);
        return 0;
    };

    if (MD5_Init(&md5) == 0){
        fprintf(stderr, "MD5_Init failed. (%d)\n", errno);
        return 0;
    };
    if (MD5_Update(&md5, str, len) == 0){
        fprintf(stderr, "MD5_Update failed. (%d)\n", errno);
        return 0;
    };
                    printf("three\n");
    if (MD5_Final(md5_hash, &md5) == 0){
        fprintf(stderr, "MD5_Final failed. (%d)\n", errno);
        return 0;
    };
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
        sprintf(&md5_string[i*2], "%02x", (unsigned int)md5_hash[i]);
    md5_string[MD5_DIGEST_LENGTH * 2] = '\0';
    return md5_string;
}

static long getFileRaw(const char *path, char **out){
    FILE *filefd = fopen(path, "rb");
    if (filefd == 0){
        fprintf(stderr, "fopen failed. (%d)\n", errno);
        return 0;
    }
    fseek(filefd, 0, SEEK_END);
    long filesize = ftell(filefd);
    printf("%ld\n", filesize);
    if (filesize == -1){
        fclose(filefd);
        fprintf(stderr, "filesize getting error. (%d)\n", errno);
        return 0;
    }
    rewind(filefd);
    char *raw = malloc(filesize);
    if (raw == 0){
        fprintf(stderr, "iMalloc() failed. (%d)\n", errno);
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

int main(void){
    char *raw = 0;
    long len = getFileRaw("/home/roman/Downloads/ritual.mkv", &raw);    
    if (len == 0){
        printf("error: len = 0\n");
        return 1;
    }
    char sum = getMD5sum(raw, len);
    if (sum == 0){
        printf("error: sum = 0\n");
        return 1;
    }
    printf("MD5 sum = %s", sum);
    return 0;
}