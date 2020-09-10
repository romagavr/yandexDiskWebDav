#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<error.h>

typedef enum errors {
    E_SUCCESS,
    E_EST_CONN,
    E_ERROR2,
    E_ERROR3,
    ERROR_COUNT,
} error_t;

//TODO: static ???
const char *const errArr[] =
{
    "OK",
    "Error1",
    "Error2",
    "Error3"
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

void logErrno(const char *msg){
    fprintf(stderr, "Error: %s - (%d: %s)", msg, errno, strerror(errno));
}

void logMessage(const char *msg){
        fprintf(stderr, "Message: %s", msg);
}