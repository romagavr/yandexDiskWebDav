#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

#define MIN_ERR_ENUM -100

#define MALLOC_ERROR_CHECK(p) {\
        if (p == 0) {\
            return E_MALLOC_FAILED;\
        }\
}

typedef enum errors {
    E_SUCCESS = MIN_ERR_ENUM,
    E_EST_CONN,
    E_PEER_CLOSED_CONN,
    E_SSL_FATAL,
    E_HTTP_PARSER_FAILED,
    E_SEND,
    E_MALLOC_FAILED,
    E_TOO_MANY_REQ,
    E_NOT_SPEC,
    E_SPRINTF,
    E_HTTP_STAT_400,
    E_HTTP_STAT_404,
    E_FIFO_OPEN,
    E_FIFO_CLOSE,
    ERROR_COUNT,
} error_t;

//TODO: static ???
static const char *const errArr[] =
{
    "OK",
    "Error while establishing connection",
    "Peer closed connection",
    "Fatal error while SSL processing",
    "Error while parsing response",
    "Error while sending request",
    "Error while allocating memory (malloc)",
    "Warning - too many requests (status = 429)",
    "Unspecified error",
    "Sprintf error",
    "Http error - (400) Bad request",
    "Http error - (404) Not found",
    "Error while opening FIFO"
    "Error while closinging FIFO"
};


void logLibError(error_t err, int printErrno);
void logSSLError(const char *message);
void logHParserError(const char *message);
void logMessage(const char *msg);

void logErrno(const char *mes);