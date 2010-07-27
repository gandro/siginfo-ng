#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "siginfo-ng.h"

#define HTTP_EOL    "\r\n"

#define HTTP_END_HEADERS        "\r\n\r\n"
#define HTTP_END_HEADERS_ALT    "\n\n"

static int siginfo_connnect(siginfo_Settings *settings) {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sockfd, error;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    error = getaddrinfo(settings->server, settings->port, &hints, &result);
    if(error != 0) {
        log_print(log_Warning, "getaddrinfo: %s\n", gai_strerror(error));
        return -1;
    }

    for(rp = result; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(sockfd == -1) {
            continue;
        }

        if(connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            break;
        }

        close(sockfd);
    }

    if(rp == NULL) {
        return -1;
    }

    freeaddrinfo(result);

    return sockfd;
}

static int siginfo_disconnnect(int socket) {
    return close(socket);
}

static int siginfo_send_data(int socket, const void *data) {
    int success = -1;
    size_t total, sent, left;

    left = total = strlen(data);
    sent = 0;

    while(sent < total) {
        success = send(socket, data+sent, left, 0);
        if(success == -1) {
            log_print(log_Warning, "siginfo_send_data: %s", strerror(errno));
            break;
        }

        sent += success;
        left -= success;
    }

    return (success == -1) ? -1 : 0;
}

#define to_hex(dec) ("0123456789abcdef"[(dec) & 15])

static char *siginfo_encode_data(const char *data) {
    /* from http://www.geekhideout.com/urlcode.shtml */
    const char *data_p = data;
    char *enc, *enc_p;
    enc = enc_p = malloc(strlen(data) * 3 + 1);

    if(!enc) {
        log_print(log_Fatal, "siginfo_encode_data: malloc() failed!\n");
        exit(EXIT_FAILURE);
    }

    while (*data_p) {
        if((isalnum(*data_p) || *data_p == '-' ||
            *data_p == '_' || *data_p == '.' || *data_p == '~')
        ) {
            *enc_p++ = *data_p;
        } else if(*data_p == ' ') {
            *enc_p++ = '+';
        } else {
            *enc_p++ = '%';
            *enc_p++ = to_hex(*data_p >> 4);
            *enc_p++ = to_hex(*data_p & 15);
        }
        data_p++;
    }
    *enc_p = '\0';

    return enc;
}

static int siginfo_send_formated(int socket, const char *fmt, ...) {
    va_list args;
    size_t length;
    char formated[128];

    va_start(args, fmt);
    length = vsnprintf(formated, sizeof(formated), fmt, args);
    va_end(args);

    if(siginfo_send_data(socket, formated) == -1) {
        return -1;
    }

    return length;
}

static int siginfo_send_headers(int socket, siginfo_Settings *settings, int datalen) {
    siginfo_send_data(socket, "POST /insert.php HTTP/1.1" HTTP_EOL);
    siginfo_send_formated(socket, "Host: %s" HTTP_EOL, settings->server);
    siginfo_send_data(socket, "Connection: close" HTTP_EOL);
    siginfo_send_data(socket, "User-Agent: "CLIENT_NAME" v"CLIENT_VERSION HTTP_EOL);
    siginfo_send_data(socket, "Content-Type: application/x-www-form-urlencoded" HTTP_EOL);
    siginfo_send_formated(socket, "Content-Length: %d" HTTP_EOL, datalen);

    return siginfo_send_data(socket, HTTP_EOL);
}

static void siginfo_post_userdata(int socket, siginfo_Settings *settings) {
    size_t i, length = 0;
    char *username, *password, *computer, *uptime, *version;
    char *row[SIGINFO_ROWS];

    /* sizeof considers '\0' instead of separation character '&' */

    username = siginfo_encode_data(settings->username);
    length += sizeof("user=") + strlen(username);

    password = siginfo_encode_data(settings->password);
    length += sizeof("pass=") + strlen(password);

    computer = siginfo_encode_data(settings->computer);
    length += sizeof("comp=") + strlen(computer);

    uptime = siginfo_encode_data(settings->uptime);
    length += sizeof("uptime=") + strlen(uptime);

    version = siginfo_encode_data(settings->version);
    length += sizeof("version=") + strlen(version) - 1;

    for(i=0; i<SIGINFO_ROWS; i++) {
        row[i] = siginfo_encode_data(settings->layout.row[i]);
        length += sizeof("zeile0=") + strlen(row[i]);
    }

    if(siginfo_send_headers(socket, settings, length) == -1) {
        return;
    }

    siginfo_send_formated(socket,
        "user=%s&pass=%s&comp=%s&uptime=%s&version=%s",
        username, password, computer, uptime, version
    );

    for(i=0; i<SIGINFO_ROWS; i++) {
        siginfo_send_formated(socket, "&zeile%d=%s", i+1, row[i]);
    }

    free(username); free(password); free(computer);
    free(uptime); free(version);

    for(i=0; i<SIGINFO_ROWS; i++) {
        free(row[i]);
    }
}


static int siginfo_parse_response(int socket) {
    char response[2048], *http_content = NULL;
    int read, http_ver, http_status, siginfo_status;

    memset(response, 0, sizeof(response));

    read = recv(socket, response, sizeof(response)-1, MSG_WAITALL);
    if(read == -1) {
        log_print(log_Error, "Failed to get response from server: %s\n", strerror(errno));
        return 302;
    } else if(read == 0) {
        log_print(log_Error, "Failed to get response from server: %s\n", "Connection closed");
        return 302;
    }

    read = sscanf(response, "HTTP/1.%d %d", &http_ver, &http_status);
    if(read == EOF) {
        log_print(log_Error, "Invalid server response header: %s\n", response);
        return 303;
    }

    if(http_status != 200) {
        log_print(log_Error, "Unsupported HTTP status code: %d\n", http_status);
        return 303;
    }

    http_content = strstr(response, HTTP_END_HEADERS);
    if(http_content == NULL) {
        http_content = strstr(response, HTTP_END_HEADERS_ALT);
        if(http_content == NULL) {
            log_print(log_Error, "HTTP content section not found");
            return 303;
        } else {
            http_content += strlen(HTTP_END_HEADERS_ALT);
        }
    } else {
        http_content += strlen(HTTP_END_HEADERS);
    }

    read = sscanf(http_content, "%d", &siginfo_status);
    if(read == EOF) {
        log_print(log_Error, "Invalid server response: %s\n", http_content);
        return 303;
    }

    if(siginfo_status == 3) {
        /* hack: assuming 'Transfer-Encoding: chunked' */
        read = sscanf(strstr(http_content, "3") + 1, "%d", &siginfo_status);
        if(read == EOF) {
            log_print(log_Error, "Invalid server response: %s\n", http_content);
            return 303;
        }
    }

    return siginfo_status;
}


const char *siginfo_status_message(int status) {
    switch(status) {
        case 100:
            return "Profile updated successfully.";

        case 201:
            return "Invalid username.";
        case 202:
            return "Invalid password.";
        case 203:
            return "Invalid computer.";
        case 204:
            return "No username specified.";
        case 205:
            return "No password specified.";
        case 206:
            return "No computer specified.";
        case 210:
            return "Data transmission failed.";
        case 220:
            return "Uptime difference to big.";

        case 301:
            return "Connection to server failed.";
        case 302:
            return "Receiving from server failed.";
        case 303:
            return "Invalid response from server.";

        case 401:
            return "Server couldn't connect to database.";
        case 402:
            return "Server reported database error.";
        case 403:
            return "Server reported processing error.";

        default:
            return "Unknown or invalid status code.";
    }
}

int siginfo_publish_data(siginfo_Settings *settings) {
    int socket, status;

    socket = siginfo_connnect(settings);
    if(socket == -1) {
        return 301;
    }
    siginfo_post_userdata(socket, settings);
    status = siginfo_parse_response(socket);
    siginfo_disconnnect(socket);

    return status;
}
