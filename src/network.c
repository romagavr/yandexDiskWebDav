#include"network.h"


static int getSSLerror(SSL *ssl, int ret);
static void messageReset(struct message *m);

static int socketRead(struct network *net);
static int socketWrite(const char *request, size_t size, SSL *ssl);


int on_chunk_header(http_parser *parser) {
    struct message *m = (struct message *)parser->data;
    if (m->chunked != 1)
        m->chunked = 1;
    m->chunk_length = parser->content_length;
    m->content_length += parser->content_length;
    //printf("Chunk length: %d\n", parser->content_length);
    return 0;
}

int on_chunk_complete(http_parser *parser) {
    
    return 0;
}

int on_header_field(http_parser *parser, const char *data, size_t length) {
    struct message *m = (struct message *)parser->data;
    if (m->last_header_element != FIELD)
        m->num_headers++;
    strncat(m->headers[m->num_headers-1][0], data, length);
    m->last_header_element = FIELD;
    ///printf("Header field: %.*s\n", (int)length, data);
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
  struct message *m = (struct message *)parser->data;
  messageReset(m);
  m->message_begin_cb_called = 1;
  //printf("\n***MESSAGE BEGIN***\n\n");
  return 0;
}

int on_headers_complete(http_parser *parser) {
  struct message *m = (struct message *)parser->data;
  m->status = parser->status_code;
  m->headers_complete_cb_called = 1;
  //printf("\n***HEADERS COMPLETE***\n\n");
  return 0;
}

int on_message_complete(http_parser *parser) {
  struct message *m = (struct message *)parser->data;
  m->message_complete_cb_called = 1;
  //printf("\n***MESSAGE COMPLETE***\n\n");
  return 0;
}

//TODO обработка ошибок в call_back
int on_body(http_parser *parser, const char* data, size_t length) {
  struct message *m = (struct message *)parser->data;
  int t_len = m->parsed_length + length;
  if (t_len > m->body_size){
      char *new = realloc(m->body, t_len * 2);
      if (new == 0){
         return HPE_CB_body;
      }
      m->body = new;
      m->body_size = t_len * 2;
  }
  memcpy(m->body + m->parsed_length, data, length);
  m->parsed_length = t_len;
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
/*            printf("\n%.*s\n", bytes_rec, read+total_rec);
            ssize_t nparsed = http_parser_execute(parser, settings, read + total_rec, bytes_rec);
             
            printf("\nStatus: %d\n", parser->status_code);
            printf("\nStatus: %d\n", nparsed);
            total_rec += bytes_rec;
            struct message *m = (struct message *)parser->data;

            printf("\n%d\n", m->status_code);
            for (int i=0; i < m->num_headers; i++) {
                printf("\nKey: %s; Value: %s\n", m->headers[i][0], m->headers[i][1]);
            }
            printf("\nRaw: %hhx", m->body); */

static int socketRead(struct network *net){
    int bytes_rec = 0;
    int ret;
    struct message *m = (struct message *)net->parser->data;

    // TODO: Timeout - если долго нет ответа
    while (1){
        memset(net->read, 0, RECEIVE_BUFFER_SIZE);
        bytes_rec = SSL_read(net->ssl, net->read, RECEIVE_BUFFER_SIZE);
        if (bytes_rec > 0) {
            ssize_t nparsed = http_parser_execute(net->parser, net->settings, net->read, bytes_rec);
            //printf("Bytes rec: %d, Nparsed: %d, Status: %d\n", bytes_rec, nparsed, m->status);
            if (net->parser->http_errno != 0){
                printf("Errno\n");
                // TODO: Не работает
                //logHParserError(HTTP_PARSER_ERRNO((http_parser *)net->parser));
                ret = E_HTTP_PARSER_FAILED;
                break;
            }
            if (m->status == 429){
                ret = E_TOO_MANY_REQ;
                break;
            }
            if (m->status == 400) {
                ret = E_HTTP_STAT_400;
                break;
            }
            if (m->status == 404) {
                ret = E_HTTP_STAT_404;
                break;
            }
            if (m->message_complete_cb_called) {
                ret = E_SUCCESS;
                break;
            }
        } else {
            ret = getSSLerror(net->ssl, bytes_rec);
            break;
        }
    }  
    return ret;
}

static int socketWrite(const char *request, size_t size, SSL *ssl){
    int bytes_sent = 0;

    bytes_sent = SSL_write(ssl, request, size);
    if (bytes_sent <= 0) {
        return getSSLerror(ssl, bytes_sent);
    }  
    return E_SUCCESS;
}

int send_to(const char *request, size_t size, struct network *net){
    int ret = socketWrite(request, size, net->ssl);
    if (ret != E_SUCCESS){
        return E_SEND;
    }
    while(1) {
        ret = socketRead(net);
        if (ret == E_TOO_MANY_REQ){
            sleep(1);
            continue;
        }
        break;
    }    
    return ret;
}

static void messageReset(struct message *m){
    m->status = 0;
    memset(m->body, 0, m->body_size);
    m->content_length = 0;
    m->parsed_length = 0;
    m->last_header_element = 0;
    m->num_headers = 0;

    m->message_begin_cb_called = 0;
    m->message_complete_cb_called = 0;
    m->headers_complete_cb_called = 0;

    m->chunked = 0;
    m->chunk_length = 0;
    
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
    net->read = malloc(RECEIVE_BUFFER_SIZE);
    MALLOC_ERROR_CHECK(net->read);

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
    settings->on_chunk_header = on_chunk_header;
    settings->on_chunk_complete = on_chunk_complete;

    parser = malloc(sizeof(http_parser));
    MALLOC_ERROR_CHECK(parser);
    memset(parser, 0, sizeof(http_parser));
    http_parser_init(parser, HTTP_RESPONSE);

    struct message *m = malloc(sizeof(struct message));
    MALLOC_ERROR_CHECK(m);
    m->body_size = BODY_SIZE;
    m->body = malloc(m->body_size);
    MALLOC_ERROR_CHECK(m->body);
    messageReset(m);

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

    free(net->read);
    
    SSL_free(net->ssl);
    close(net->socket_peer);
    SSL_CTX_free(net->ctx);

    free(net);
}