#ifndef ERROR_H
#define ERROR_H

#define MIN_ERR_ENUM -100
#define MALLOC_ERROR_CHECK(p) {\
        if (p == 0) {\
            fprintf(stderr, "** Error: Out of memory - (%d: %s)\n", errno, strerror(errno));\
            exit(EXIT_FAILURE);\
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
    E_HTTP_STAT_403,
    E_HTTP_STAT_409,
    E_FIFO_OPEN,
    E_FIFO_CLOSE,
    E_SYNC_ERROR,
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
    "Http error - (403) Forbidden",
    "Http error - (409) Conflict",
    "Error while opening FIFO",
    "Error while closinging FIFO",
    "Synchronization error"
};


void logLibError(error_t err, int printErrno);
void logSSLError(const char *message);
void logHParserError(const char *message);
void logMessage(const char *msg);

void logErrno(const char *mes);
void logSyncErr(const char *file, error_t err);

#endif // ERROR_H