#include"error.h"

static const char* getError(error_t err){
    const char *error = 0;
    if (err < ERROR_COUNT){
        error = errArr[err];
    }
    return error;
}

void logLibError(error_t err, int printErrno){
  const char *mes = getError(err);
  if (err != 0)
    fprintf(stderr, "Smth go wrong: %s\n", mes);
  if (printErrno)
    fprintf(stderr, "Errno: (%d: %s)", errno, strerror(errno));
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