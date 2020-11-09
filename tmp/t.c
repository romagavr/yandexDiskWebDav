#include<stdio.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>

#define NAMEDPIPE_NAME "./pipe"
#define BUFSIZE 50

int main() {
    int fd, len;
    
    if (mkfifo(NAMEDPIPE_NAME, 0777)) {
        perror("Parent: mkfifo");
        return 1;
    }
    printf("Parent: FIFO %s is created\n", NAMEDPIPE_NAME);


    switch (fork())
    {
    case -1:
        printf("Forking error\n");
        return 1;
        break;
    
    case 0:
        printf("Child started\n");
        char buf[BUFSIZE];
        char str[BUFSIZE * 15];
        if ((fd = open(NAMEDPIPE_NAME, O_RDONLY)) <= 0) {
            perror("Child: error open for read");
            return 1;
        }
        printf("Child: %s is opend\n", NAMEDPIPE_NAME);
        do {
            memset(buf, '\0', BUFSIZE);
            if ((len = read(fd, buf, BUFSIZE-1)) <= 0) {
                perror("read");
                close(fd);
                printf("Incoming message (%d): \n%s\n", strlen(str), str);
                exit(EXIT_SUCCESS);
                return 0;    
            }
            strncpy(str + strlen(str), buf, len);
        } while (1);
        exit(EXIT_SUCCESS);
        break;

    default:
        //PS Lol - you cant daclare variable after lable
        printf("Parent continued\n");
        char *data = "Smth to send to child through FIFO\n";

        if ((fd = open(NAMEDPIPE_NAME, O_WRONLY)) <= 0) {
            perror("Parent: error open for write");
            return 1;
        }
        for (int i=0; i<10; i++) {
            if ((len = write(fd, data, strlen(data))) <= 0) {
                perror("Parent: read");
                close(fd);
                remove(NAMEDPIPE_NAME);
                return 0;    
            };
        }
        remove(NAMEDPIPE_NAME);
        close(fd);

        int status = 0;
        wait(&status);

        break;
    }

    return 0;
}