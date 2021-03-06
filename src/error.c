#include"error.h"

static const char* getError(error_t err);

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
    fprintf(stderr, "** (%s) - Errno: (%d: %s)\n", mes, errno, strerror(errno));
};