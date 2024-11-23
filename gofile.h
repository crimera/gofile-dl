#ifndef gofile__h
#define gofile__h

#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONTENT_URL "https://api.gofile.io/contents/%s?wt=4fd6sg89d7s6"
#define GOFILE_USER_AGENT                                                      \
  "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:132.0) "                       \
  "Gecko/20100101 Firefox/132.0"

#define GOFILE_FILESTATUS_NOTFOUND "error-notFound"

typedef struct {
  char *data;
  size_t size;
} Response;

typedef struct {
  char *name;
  char *url;
} File;

typedef struct {
  File *files;
  size_t children_count;
} Content;

Response *new_response();
void free_response(Response *resp);

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);
void gofile_set_curlopts(CURL *hnd);
char *get_content_id(const char *url);

char *fetch_guest_account(CURL *hnd);
char *get_account_token(CURL *hnd);

char *fetch_content(CURL *hnd, char *token, char *file_id);
Content *get_files(char *content);
void free_files(Content *content);

#endif
