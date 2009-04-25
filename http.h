/*
 * ----------------------------------------------------------------------------
 * "THE COFFEE-WARE LICENSE" (Revision 12/2007):
 * Sebastian Wicki <gandro@gmx.net> wrote this file. As long as you retain
 * this notice you can do whatever you want with this stuff. If we meet some
 * day, and you think this stuff is worth it, you can buy me a cup of coffee
 * in return. 
 * ----------------------------------------------------------------------------
 */

#ifndef HTTP_H
#define HTTP_H

#define HTTP_SERVER "siginfo.de"
#define HTTP_PORT   80

enum {
    HTTP_SUCCESS = 0,
    HTTP_FAILURE = -1,
};

extern const char *http_headers[];

char *url_encode(const char *src);

int http_connect();
int http_send_raw(const char *data);
int http_send_headers();
int http_send_post(const char *post_data);

int http_get_siginfo_status();

void http_close();

#endif /* HTTP_H */
