#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<unistd.h>
#include<errno.h>

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#include<openssl/ssl.h>
#include<openssl/err.h>
#include<openssl/md5.h>

#include<libxml/parser.h>
#include<libxml/tree.h>
#include<assert.h>

#include"error.h"

#include"../lib/http-parser/http_parser.h"

#define HOST "oauth.yandex.ru"
#define WHOST "webdav.yandex.ru"

//{"access_token": "AgAAAAAJfAwtAAaSXZEN657D4ETDiWSPzkL4oDE", "expires_in": 31536000, "refresh_token": "1:c0790JFluYo6AsrR:ZLZuX_KaVR_2EDeWof3G1zKDMne3DGeO-u8ywEe8VVwgd0JJEpr1:nOUDZvjgDg-U6hv3WgnUYQ", "token_type": "bearer"}

#define MAX_ELEMENT_HEADER_SIZE 500 
#define MAX_HEADERS_COUNT 15
#define RECEIVE_BUFFER_SIZE 16384
#define BODY_SIZE 3000

#define SSL_ERMES_SIZE 300


struct network {
    http_parser_settings *settings;
    http_parser *parser;

    SSL *ssl;
    SSL_CTX *ctx; 
    int socket_peer;
};

struct message {
  char *body;
  FILE *file;

  int body_size; // sizeof allocated memory for *body
  int parsed_length; //sizeof parsed data in *body
  int isToFile; // is need to write to file
  
  int status;
  int message_complete_cb_called;

  int num_headers;
  enum { NONE=0, FIELD, VALUE } last_header_element;
  char headers[MAX_HEADERS_COUNT][2][MAX_ELEMENT_HEADER_SIZE];

};

int on_chunk_header(http_parser *parser);
int on_chunk_complete(http_parser *parser);
int on_header_field(http_parser *parser, const char *data, size_t length); 
int on_header_value(http_parser *parser, const char *data, size_t length);
int on_message_begin(http_parser *parser);
int on_headers_complete(http_parser *parser);
int on_message_complete(http_parser *parser);
int on_body(http_parser *parser, const char* data, size_t length);
        
int connect_to(struct network *net, const char *host);

int send_to(const char *request, struct network *net, char **resp);
int send_to1(const char *request, struct network *net, FILE *file);

int initNetworkStruct(struct network **netw);
void freeNetworkStruct(struct network *net);
