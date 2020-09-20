#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

#define MALLOC_ERROR_CHECK(p) { \
        if (p == 0) { \
            logLibError(E_MALLOC_FAILED, 1); \
            return E_MALLOC_FAILED; \
        } \ 
}

typedef enum errors {
    E_SUCCESS,
    E_EST_CONN,
    E_PEER_CLOSED_CONN,
    E_SSL_FATAL,
    E_HTTP_PARSER_FAILED,
    E_SEND,
    E_MALLOC_FAILED,
    E_TOO_MANY_REQ,
    ERROR_COUNT,
} error_t;

//TODO: static ???
static const char *const errArr[] =
{
    "OK",
    "Error while establishing connection",
    "Peer closed connection",
    "Fatal error while SSL processing",
    "Erroe while parsing response",
    "Error while sending request",
    "Error while allocating memory (malloc)",
    "Warning - too many requests (status = 429)"
};

static const char* getError(error_t err);

void logLibError(error_t err, int printErrno);
void logSSLError(const char *message);
void logHParserError(const char *message);
void logMessage(const char *msg);