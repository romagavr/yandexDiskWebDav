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
#include<assert.h>

#include<libxml/parser.h>
#include<libxml/tree.h>
#include"error.h"
#include"network.h"


static int getSSLerror(SSL *ssl, int ret);
static void messageReset(struct message *m);

static Response* socketRead(struct network *net);
static inline Response* getResponse(struct network *net);

/*
int on_chunk_header(http_parser *parser) {
    return 0;
}
int on_chunk_complete(http_parser *parser) {
    return 0;
}
*/

int on_header_field(http_parser *parser, const char *data, size_t length) {
    struct message *m = (struct message *)parser->data;
    if (m->last_header_element != FIELD)
        m->num_headers++;
    strncat(m->headers[m->num_headers-1][0], data, length);
    m->last_header_element = FIELD;
    //printf("Header field: %.*s\n", (int)length, data);
    return 0;
}

int on_header_value(http_parser *parser, const char *data, size_t length) {
    struct message *m = (struct message *)parser->data;
    strncat(m->headers[m->num_headers-1][1], data, length);
    m->last_header_element = VALUE;
    //printf("Header value: %.*s\n", (int)length, data);
    return 0;
}

int on_message_begin(http_parser *parser) {
  //messageReset((struct message *)parser->data);
  return 0;
}

int on_headers_complete(http_parser *parser) {
  struct message *m = (struct message *)parser->data;
  m->status = parser->status_code;
  //printf("\n***HEADERS COMPLETE***\n\n");
  return 0;
}

int on_message_complete(http_parser *parser) {
    struct message *m = (struct message *)parser->data;
    m->message_complete_cb_called = 1;
    //printf("%s\n", m->body);
    //assert(0);
    //if (m->body != 0 && m->parsed_length != 0)
    //    m->body[m->parsed_length] = '\0';
    //printf("\n***MESSAGE COMPLETE***\n\n");
    return 0;
}

//TODO обработка ошибок в call_back
int on_body(http_parser *parser, const char* data, size_t length) {
    struct message *m = (struct message *)parser->data;
    if (m->isToFile) {
        fwrite(data, sizeof(char), length, m->file);
        m->parsed_length += length;
    } else {
        if (m->body == 0) {
            m->body = malloc(BODY_SIZE);
            MALLOC_ERROR_CHECK(m->body);
        }
        size_t newlen = m->parsed_length + length;
        if (newlen > m->body_size){
            m->body_size = newlen * 2;
            char *new = realloc(m->body, m->body_size);
            if (new == 0){
                return HPE_CB_body;
            }
            m->body = new;
        }
        memcpy(m->body + m->parsed_length, data, length);
        m->parsed_length = newlen;
        //printf("%s\n", m->body);
    }
  return 0;
}

int connect_to(struct network *net, const char *host) {
    logMessage("Establishing TCP connection");

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *peer_address = 0;
    if (getaddrinfo(host, "https", &hints, &peer_address)) {
        logLibError(E_EST_CONN, 1);
        return E_EST_CONN;
    }
  /* 
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, address_buffer,
                sizeof(address_buffer),
                service_buffer, sizeof(service_buffer),
                NI_NUMERICHOST);
    printf("Remote address is: %s %s\n", address_buffer, service_buffer);
*/
    int socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
    if (socket_peer < 0) {
        logLibError(E_EST_CONN, 1);
        return E_EST_CONN;
    }

    if (connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen)) {
        logLibError(E_EST_CONN, 1);
        return E_EST_CONN;
    }
    freeaddrinfo(peer_address);

    OpenSSL_add_all_algorithms();
    SSL_METHOD *method = SSLv23_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (ctx == 0) {
        logLibError(E_SSL_FATAL, 0);
        return E_EST_CONN;
    }
    SSL *ssl = SSL_new(ctx); 
    SSL_set_fd(ssl, socket_peer); 
    if (SSL_connect(ssl) == -1) {
        logLibError(E_SSL_FATAL, 0);
        return E_EST_CONN;
    }

    net->socket_peer = socket_peer;
    net->ctx = ctx;
    net->ssl = ssl;

    logMessage("Connection established");
    return E_SUCCESS;
}

static int getSSLerror(SSL *ssl, int ret){
    int err = SSL_get_error(ssl, ret);
    SSL_load_error_strings();
    char errmes[SSL_ERMES_SIZE] = {0};
    int r_error;
    switch (err)
    {
        //TODO: check another errors
        case SSL_ERROR_ZERO_RETURN:
        {
            // Connection closed
            ERR_error_string_n(err, errmes, SSL_ERMES_SIZE);
            logSSLError(errmes);
            r_error = E_PEER_CLOSED_CONN;
            break;
        }

        case SSL_ERROR_SSL:
        {
            // Fatal error
            ERR_error_string_n(err, errmes, SSL_ERMES_SIZE);
            logSSLError(errmes);
            r_error = E_SSL_FATAL;
            break;
        }

        default:
        {
            logSSLError("Uncaught error");
            r_error = E_SSL_FATAL;
            break;
        }
    }
    ERR_free_strings();
    return r_error;
}

static Response* socketRead(struct network *net){
    size_t bytes, nparsed, total_rec = 0;
    int ret=0;
    struct message *m = (struct message *)net->parser->data;
    char *raw = malloc(RECEIVE_BUFFER_SIZE);
    MALLOC_ERROR_CHECK(raw);
    Response *resp = malloc(sizeof(Response));
    MALLOC_ERROR_CHECK(resp);
    memset(resp, 0, sizeof(Response));

    // TODO: Timeout - если долго нет ответа
    while ((bytes = SSL_read(net->ssl, raw, RECEIVE_BUFFER_SIZE)) > 0){
        nparsed = http_parser_execute(net->parser, net->settings, raw, bytes);
        if (net->parser->http_errno != 0 || nparsed != bytes)
            resp->status = E_HTTP_PARSER_FAILED;
        //printf("%s/n", readbuf);
        /*
        if (m->message_complete_cb_called && (m->status == 207 || m->status == 200 || m->status == 201 || m->status == 100)){
            ret = E_SUCCESS;
            break;
        } else if (m->status != 207 && m->status != 200) {
            switch (m->status)
            {
                case 429:
                    ret = E_TOO_MANY_REQ;
                    break;
                case 400:
                    ret = E_HTTP_STAT_400;
                    break;
                case 403:
                    ret = E_HTTP_STAT_403;
                    break;
                case 404:
                    ret = E_HTTP_STAT_404;
                    break;
                case 409:
                    ret = E_HTTP_STAT_409;
                    break;
                default:
                    break;
            }
            break;
        }
        */
    }

   if (!resp->status) {
        if (!bytes) {
            if (m->isToFile) {
                resp->data = m->body;
                resp->dataLen = m->body_size;
            }
            resp->status = m->status;
 
        } else resp->status = getSSLerror(net->ssl, bytes);;
    }
    free(raw);
    return resp;
}

//int sendFile(const char *filePath, struct network *net)
Response* requestFM(FILE *fd, struct network *net){
    Response *resp;
    size_t b, bSent;
    char raw[100];
    while((b = fread(raw, 1, 100, fd))){
        bSent = SSL_write(net->ssl, raw, b);
    }
    if (bSent >= 0) {
        resp = getResponse(net);
    } else {
        resp = malloc(sizeof(Response));
        MALLOC_ERROR_CHECK(resp);
        memset(resp, 0, sizeof(Response));
        resp->status = getSSLerror(net->ssl, b);
    }
    return resp;
}

//Response* send_to1(const char *mes, struct network *net, FILE *file){
Response* requestMF(const char *mes, struct network *net, FILE *fd){
    struct message *m = (struct message *)net->parser->data;
    m->file = fd;
    m->isToFile = 1;
    return requestMM(mes, net);
}

//int send_to(const char *request, struct network *net, char **resp)
Response* requestMM(const char *mes, struct network *net){
    int b;
    Response *resp;

    if ((b = SSL_write(net->ssl, mes, strlen(mes))) <= 0) {
        resp = malloc(sizeof(Response));
        MALLOC_ERROR_CHECK(resp);
        memset(resp, 0, sizeof(Response));
        resp->status = getSSLerror(net->ssl, b);
    } else 
        resp = getResponse(net);
    return resp;
}

static inline Response* getResponse(struct network *net) {
    Response *resp;
    while(1) {
        resp = socketRead(net);
        if (resp->status == E_TOO_MANY_REQ){
            sleep(1);
            continue;
        }
        break;
    }    
    messageReset((struct message *)net->parser->data);
    return resp;
}

static void messageReset(struct message *m){
    m->status = 0;
    m->body = 0;
    m->body_size = 0;
    m->parsed_length = 0;
    m->last_header_element = 0;
    m->num_headers = 0;
    m->message_complete_cb_called = 0;
    m->isToFile = 0; 
    m->file = 0;

    for (int i=0; i< MAX_HEADERS_COUNT; i++){
        memset(m->headers[i][0], 0, MAX_ELEMENT_HEADER_SIZE);
        memset(m->headers[i][1], 0, MAX_ELEMENT_HEADER_SIZE);
    }
}

// TODO: при ошибке - очистка памяти
int initNetworkStruct(struct network **netw){
    struct network *net = malloc(sizeof(struct network));
    MALLOC_ERROR_CHECK(net);

    memset(net, 0, sizeof(struct network));

    http_parser_settings *settings;
    http_parser *parser;

    settings = malloc(sizeof(http_parser_settings));
    MALLOC_ERROR_CHECK(settings);

    memset(settings, 0, sizeof(http_parser_settings));
    settings->on_header_field = on_header_field;
    settings->on_header_value = on_header_value;
    settings->on_message_begin = on_message_begin;
    settings->on_headers_complete = on_headers_complete;
    settings->on_body = on_body;
    settings->on_message_complete = on_message_complete;
    //settings->on_chunk_header = on_chunk_header;
    //settings->on_chunk_complete = on_chunk_complete;

    parser = malloc(sizeof(http_parser));
    MALLOC_ERROR_CHECK(parser);
    memset(parser, 0, sizeof(http_parser));
    http_parser_init(parser, HTTP_RESPONSE);

    struct message *m = malloc(sizeof(struct message));
    MALLOC_ERROR_CHECK(m);
    memset(m, 0, sizeof(struct message));
    parser->data = m;
    net->parser = parser;
    net->settings = settings;
    *netw = net;
    return E_SUCCESS;
}

void freeNetworkStruct(struct network *net){
    http_parser_settings *settings = net->settings;
    http_parser *parser = net->parser;

    struct message *m = (struct message *)parser->data;
    free(m->body);
    free(m);
    free(parser);
    free(settings);

    SSL_free(net->ssl);
    close(net->socket_peer);
    SSL_CTX_free(net->ctx);

    free(net);
}
