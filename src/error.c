#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include"error.h"

static const char* getError(error_t err){
    if (err > ERROR_COUNT && err - MIN_ERR_ENUM < 0)
        err = E_NOT_SPEC;
    return errArr[err - MIN_ERR_ENUM];
}

void logLibError(error_t err, int printErrno){
  const char *m = getError(err);
  if (printErrno)
    logErrno(m);
  else
    fprintf(stderr, "Smth go wrong: %s\n", m);
};

void logSSLError(const char *message){
    fprintf(stderr, "Smth go wrong in SSL: %s\n", message);
};

void logHParserError(const char *message){
    fprintf(stderr, "Smth go wrong in HTTP_PARSER: %s\n", message);
};

void logMessage(const char *msg){
        fprintf(stderr, "Message: %s\n", msg);
}

// client.c

void logErrno(const char *mes){
    fprintf(stderr, "Fatal Error: %s(%d) - (%s)\n", mes, errno, strerror(errno));
};

void logSyncErr(const char *file, error_t err){
    fprintf(stderr, "Sync error while getting \"%s\": %s;\n", file, getError(err));
};