/*
 * ----------------------------------------------------------------------------
 * "THE COFFEE-WARE LICENSE" (Revision 12/2007):
 * Sebastian Wicki <gandro@gmx.net> wrote this file. As long as you retain
 * this notice you can do whatever you want with this stuff. If we meet some
 * day, and you think this stuff is worth it, you can buy me a cup of coffee
 * in return. 
 * ----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>

#include "http.h"
#include "siginfo-ng.h"

const char *http_headers[] = {
    "POST /insert.php HTTP/1.1\r\n",
    "Host: "HTTP_SERVER"\r\n",
    "Connection: close\r\n",
    "User-Agent: "CLIENT" v"VERSION"\r\n",
    "Content-Type: application/x-www-form-urlencoded\r\n",
    NULL
};

struct {
    int fd;
    struct sockaddr_in addr;
    struct hostent *host;
} http_socket;

char *url_encode(const char *src) {
    const char hex[] = "0123456789ABCDEF";
    char *start, *dest;
    int dest_len = strlen(src)*3+1;
    dest = start = malloc(dest_len);
    memset(dest, 0, dest_len);

    if(dest == NULL) { 
        return NULL;
    }

    do {
        if(
            src[0] == '\\' && src[1] != '\0' &&
             isxdigit(src[1]) && isxdigit(src[2])
        ) {
            *dest++ = '%';
            *dest++ = toupper(*++src);
            *dest++ = toupper(*++src);
        } else if(!isalnum(*src) && !strchr("!'()*-._~", *src)) {
            unsigned char c = *src;
			*dest++ = '%';
			*dest++ = hex[c >> 4];
			*dest++ = hex[c & 0xf];
        } else {
            *(dest++) = *src;
        }
    } while(*++src);
    *(dest++) = '\0';
    return start;
}

int http_connect() {
    http_socket.fd = socket(AF_INET, SOCK_STREAM, 0);

    if(http_socket.fd < 0) {
        fprintf(logfile, "Error: Couldn't create socket!\n");
        return HTTP_FAILURE;
    }

    http_socket.host = gethostbyname(HTTP_SERVER);
    if(http_socket.host == NULL) {
        fprintf(logfile, "Error: Couldn't resolve \""HTTP_SERVER"\"!\n");
        return HTTP_FAILURE;
    }

    memcpy((char *) &http_socket.addr.sin_addr,
            http_socket.host->h_addr,
            http_socket.host->h_length);

    http_socket.addr.sin_family = AF_INET;
    http_socket.addr.sin_port = htons(HTTP_PORT);

    if(
        connect(http_socket.fd,
        (struct sockaddr*) &http_socket.addr,
        sizeof(http_socket.addr)) < 0
    ) {
        fprintf(logfile, "Error: Couldn't connect to \""HTTP_SERVER"\"!\n");
        return HTTP_FAILURE;
    }

    return HTTP_SUCCESS;
}

int http_send_raw(const char *data) {
    if(send(http_socket.fd, data, strlen(data), 0) != strlen(data)) {
        fprintf(logfile, "Error: Couldn't send data to \""HTTP_SERVER"\"!\n");
        return HTTP_FAILURE;
    }

    return HTTP_SUCCESS;
}

int http_send_headers() {
    int i;

    for(i=0; http_headers[i] != NULL; i++) {
        if(http_send_raw(http_headers[i]) != HTTP_SUCCESS) {
            return HTTP_FAILURE;
        }
    }

    return HTTP_SUCCESS;
}

int http_send_post(const char *post_data) {
    char length_header[24];
    memset(length_header, 0, sizeof(length_header));

    snprintf(length_header, 24, "Content-Length: %i\r\n", 
                            (int) strlen(post_data));

    http_send_raw(length_header);
    http_send_raw("\r\n");

    return http_send_raw(post_data);
}

int http_get_siginfo_status() {
    char buffer[512], *status;
    memset(buffer, 0, sizeof(buffer));

    if(
        recv(http_socket.fd, buffer, 512, MSG_PEEK) < 0 ||
        strncmp(buffer, "HTTP/1.1 200", 12) != 0
    ) {
        return 301;
    }

    /* skip headers */
    status = strstr(buffer, "\r\n\r\n")+4;

    return atoi(status);
}

void http_close() {
    close(http_socket.fd);
}
