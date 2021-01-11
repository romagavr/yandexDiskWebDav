#include<sys/inotify.h>
#include<limits.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#include<ftw.h>
#include<string.h>
#include<stdint.h>

#include<poll.h>
#include<errno.h>

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

#define WATCH_PATH "/home/roman/Documents/clang/yandexDiskWebDav/build/inotTest/"
#define MAX_OPEN_DESCR 20

int inotifyFd = -1;

static int addToWatch(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf);
static void displayInotifyEven(struct inotify_event *i);

char **watchDir;
int wdSize = 10;

//struct for file modifying(for ".tmp editors")/moving files
// may be send this?

struct file_state {
    char *name;

    int create;
    int modify;
    int movedFrom;
    int movedTo;

    long exp_time;
};
struct file_state *files;
int files_size;

//Only directories added
static int addToWatch(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf){
    if (tflag == FTW_D) {
        int wd = inotify_add_watch(inotifyFd, fpath, IN_ALL_EVENTS);
        if (wd == -1) {
            perror("inotify add");
            return 1;
        }
        if (wd >= wdSize) {
            char *n = realloc(watchDir, wd * 2);
            if (n == 0) return 1;
            watchDir = &n; 
        }
        watchDir[wd] = malloc(100);
        strcpy(watchDir[wd], fpath);
        //*(&(*watchDir) + wd)= malloc(100);
        //strcpy(*(&(*watchDir) + wd), fpath);
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
    //Create .tmp; write to .tmp; on save rename .tmp
    // watch saved
    if (flag) {
        if (i->mask & IN_MOVED_TO) {
            strcpy(files[files_size].name, watchDir[i->wd]);
            printf("\n*****\n---- File %s/%s\n---- changed\n*****\n", watchDir[i->wd], i->name);
        }
    }
    if (flag){
        printf("wd = %2d; path %s", i->wd, watchDir[i->wd]);
        if (i->len > 0)
            printf(" name = %s ", i->name);
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

#define POLL_CAPASITY 1
#define POLL_TIMEOUT -1

int main() {
    files = malloc(sizeof *files * 10);
    watchDir = malloc(sizeof(char *) * wdSize);
    if (watchDir == 0) return 1;

    inotifyFd = inotify_init1(IN_NONBLOCK);
    if (inotifyFd == -1) {
        perror("inotify init");
        return 1;
    }
    int res = nftw(WATCH_PATH, addToWatch, MAX_OPEN_DESCR, 0);
    if (res == -1){
        perror("nftw error");
        return 1;
    }
/*
    struct pollfd *pfds = malloc(sizeof *pfds * POLL_CAPASITY);
    if (pfds == 0) return 1;
    pfds[0].fd = inotifyFd;
    pfds[0].events = POLLIN;
    int poll_count = 0; 
    int size_in_ev = sizeof(struct inotify_event);
*/
    ssize_t numRead;
    char buf[BUF_LEN];
    char *p = 0;
    struct inotify_event *event = 0;
    for (;;){
       /* if ((poll_count = poll(pfds, POLL_CAPASITY, POLL_TIMEOUT)) <= 0) {
			if (poll_count == 0) {
				fprintf(stdout, "Message: poll timeout expired.\n");
			} else {
                printf("Erorr");
				exit(EXIT_FAILURE);
			}
		}

        if (((*pfds).revents & POLLIN)) {
            for (p = buf; p < buf + numRead;){
                event = (struct inotify_event *) p; 
                displayInotifyEven(event);
                p += size_in_ev + event->len;
            }
        }
    }*/
        numRead = read(inotifyFd, buf, BUF_LEN);
        if (numRead == 0){
            perror("inotify read return 0");
            return 1;
        }

        if (numRead == -1 && errno != EAGAIN){
            perror("inotify failed");
            return 1;
        }

        for (p = buf; p < buf + numRead;){
           event = (struct inotify_event *) p; 
           displayInotifyEven(event);
           p += sizeof(struct inotify_event) + event->len;
        }
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