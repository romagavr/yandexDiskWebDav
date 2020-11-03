#include<sys/inotify.h>
#include<limits.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#define _XOPEN_SOURCE 700
#include<ftw.h>
#include<string.h>
#include<stdint.h>

#include <limits.h>
#include <fcntl.h>
#include <signal.h>

#define BUF_LEN (10 * (sizeof(struct inotify_event) + 256 + 1))

#define WATCH_PATH "/home/roman/Documents/clang/yandexDiskWebDav/build/inotTest/"
#define MAX_OPEN_DESCR 20

int inotifyFd = -1;

static int addToWatch(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf);
static void displayInotifyEven(struct inotify_event *i);

static int addToWatch(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf){
    if (tflag == FTW_D) {
        int wd = inotify_add_watch(inotifyFd, fpath, IN_ALL_EVENTS);
        if (wd == -1) {
            perror("inotify add");
            return 1;
        }
        //printf("Directory %s (%s) was added to watchlist", fpath + ftwbuf->base, fpath);
    }
    return 0;
}

static void displayInotifyEven(struct inotify_event *i){
    int flag = 0; 
    if (i->mask & IN_ATTRIB && (flag = 1))        printf("IN_ATTRIB ");
    if (i->mask & IN_CREATE && (flag = 1))        printf("IN_CREATE ");
    if (i->mask & IN_DELETE && (flag = 1))        printf("IN_DELETE ");
    if (i->mask & IN_DELETE_SELF && (flag = 1))   printf("IN_DELETE_SELF ");
    if (i->mask & IN_IGNORED && (flag = 1))       printf("IN_IGNORED ");
    if (i->mask & IN_MODIFY && (flag = 1))        printf("IN_MODIFY ");
    if (i->mask & IN_MOVE_SELF && (flag = 1))     printf("IN_MOVE_SELF ");
    if (i->mask & IN_MOVED_FROM && (flag = 1))    printf("IN_MOVED_FROM ");
    if (i->mask & IN_MOVED_TO && (flag = 1))      printf("IN_MOVED_TO ");
    if (i->mask & IN_UNMOUNT && (flag = 1))       printf("IN_UNMOUNT ");
    if (flag){
        printf("wd = %2d; ", i->wd);
        if (i->len > 0)
            printf("name = %s ", i->name);
        if (i->cookie > 0)
            printf("cookie = %4d; ", i->cookie);
        printf("\n");
    }

    //printf("wd = %2d; ", i->wd);
    //printf("mask = ");
    //if (i->mask & IN_ACCESS)        printf("IN_ACCESS ");
    //if (i->mask & IN_CLOSE_NOWRITE) printf("IN_CLOSE_NOWRITE ");
    //if (i->mask & IN_CLOSE_WRITE)   printf("IN_CLOSE_WRITE ");
    //if (i->mask & IN_ISDIR)         printf("IN_ISDIR ");
    //if (i->mask & IN_OPEN)          printf("IN_OPEN ");
    //?if (i->mask & IN_Q_OVERFLOW)    printf("IN_Q_OVERFLOW ");
};

static void sigio_handler(int sig)
{
    ssize_t numRead;
    char *p = 0;
    struct inotify_event *event = 0;
    char buf[BUF_LEN] __attribute__ ((aligned(8)));

    numRead = read(inotifyFd, buf, BUF_LEN);
    if (numRead == 0){
        perror("inotify read return 0");
        return;
    }

    if (numRead == -1){
        perror("inotify failed");
        return;
    }

    for (p = buf; p < buf + numRead;){
        event = (struct inotify_event *) p; 
        displayInotifyEven(event);
        p += sizeof(struct inotify_event) + event->len;
    }
}

int main() {
    inotifyFd = inotify_init();
    if (inotifyFd == -1) {
        perror("inotify init");
        return 1;
    }
    int res = nftw(WATCH_PATH, addToWatch, MAX_OPEN_DESCR, 0);
    if (res == -1){
        perror("nftw error");
        return 1;
    }

    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = sigio_handler;
    if (sigaction(SIGIO, &sa, 0) == -1){
        perror("sigaction error");
        return 1;
    }

    /* Set owner process that is to receive "I/O possible" signal */

    if (fcntl(inotifyFd, F_SETOWN, getpid()) == -1) {
        fprintf(stderr, "fcntl(F_SETOWN)");
    }

    /* Enable "I/O possible" signaling and make I/O nonblocking
       for file descriptor */

    int flags = fcntl(inotifyFd, F_GETFL);
    if (fcntl(inotifyFd, F_SETFL, flags | O_ASYNC | O_NONBLOCK) == -1) {
        fprintf(stderr, "fcntl(F_SETFL)");
    }

    while (1) {
        pause();
    }

    return 0;
}

/*
static int display_info(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf)
{
    printf("%-3s %2d ",
            (tflag == FTW_D) ?   "d"   : 
            (tflag == FTW_DNR) ? "dnr" :
            (tflag == FTW_DP) ?  "dp"  : 
            (tflag == FTW_F) ?   "f" :
            (tflag == FTW_NS) ?  "ns"  : 
            (tflag == FTW_SL) ?  "sl" :
            (tflag == FTW_SLN) ? "sln" : "???",
            ftwbuf->level);

    if (tflag == FTW_NS)
        printf("-------");
    else
        printf("%7jd", (intmax_t) sb->st_size);

    printf("   %-40s %d %s\n",
            fpath, ftwbuf->base, fpath + ftwbuf->base);

    return 0;
} */
// gcc -Wall ../inotifySig.c -o inotifySig -D_XOPEN_SOURCE=700