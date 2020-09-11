#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<error.h>

typedef enum errors {
    E_SUCCESS,
    E_EST_CONN,
    E_PEER_CLOSED_CONN,
    E_SSL_FATAL,
    E_HTTP_PARSER_FAILED,
    ERROR_COUNT,
} error_t;

//TODO: static ???
const char *const errArr[] =
{
    "OK",
    "Error1",
    "Error2",
    "Error3",
    "Error4"
};

static const char* getError(error_t err){
    const char *error = 0;
    if (err < ERROR_COUNT){
        error = errArr[err];
    }
    return error;
}

void logLibError(error_t err){
  const char *mes = getError(err);
  if (err != 0){
        fprintf(stderr, "Smth go wrong: %s\n", mes);
  }
};

void logSSLError(const char *message){
    fprintf(stderr, "Smth go wrong in SSL: %s\n", messsage);
};

void logHParserError(const char *message){
    fprintf(stderr, "Smth go wrong in HTTP_PARSER: %s\n", messsage);
};

void logErrno(const char *msg){
    fprintf(stderr, "Error: %s - (%d: %s)", msg, errno, strerror(errno));
}

void logMessage(const char *msg){
        fprintf(stderr, "Message: %s", msg);
}