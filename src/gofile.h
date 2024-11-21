#ifndef gofile__h
#define gofile__h

#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONTENT_URL "https://api.gofile.io/contents/%s?wt=4fd6sg89d7s6"
#define GOFILE_USER_AGENT                                                      \
  "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:132.0) "                       \
  "Gecko/20100101 Firefox/132.0"
#define BUFFER_SIZE 102400

char response[BUFFER_SIZE];
CURLcode ret;
char *error;

typedef struct {
  char *name;
  char *url;
} Content;

typedef struct {
  Content *contents;
  size_t size;
} Contents;

typedef enum { NotAuthorized } GofileError;

typedef struct {
  GofileError *error;
  void *data;
} Result;

void free_contents(Contents *contents) {
  for (size_t i = 0; i < contents->size; i++) {
    free(contents->contents[i].name);
    free(contents->contents[i].url);
  }
  free(contents->contents);
  free(contents);
}

void free_result(Result *result) {
  if (result->error != NULL) {
    free(error);
  }
  free(result);
  result = NULL;
}

int DEBUG_MODE = 0;

void get_content_id(const char *url, char *file_id) {
  const char *index = strrchr(url, '/');
  strcpy(file_id, index + 1);
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  size_t total_size = size * nmemb;
  char *buffer = (char *)userdata;

  if (BUFFER_SIZE < total_size + strlen(buffer) + 1) {
    fprintf(stderr, "Buffer overflow detected\n");
    exit(EXIT_FAILURE);
  }

  if (DEBUG_MODE) {
    printf("total_size: %ld\n", total_size);
    printf("buffer: %s\n", (char *)ptr);
  }

  // Append the received data to the buffer
  strncat(buffer, (char *)ptr, total_size);

  return total_size;
}

Result *get_content(CURL *hnd, char *file_id, char token[32]) {
  struct curl_slist *headers;

  char token_header[64] = "Authorization: Bearer ";
  strncat(token_header, token, 32);

  headers = NULL;
  headers = curl_slist_append(headers, token_header);

  char response[BUFFER_SIZE] = {0};

  char content_url[64] = {0};
  snprintf(content_url, sizeof(content_url), CONTENT_URL, file_id);

  curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(hnd, CURLOPT_WRITEDATA, response);
  curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, BUFFER_SIZE);
  curl_easy_setopt(hnd, CURLOPT_URL, content_url);
  curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(hnd, CURLOPT_USERAGENT, GOFILE_USER_AGENT);
  curl_easy_setopt(hnd, CURLOPT_HTTPGET, 1L);

  ret = curl_easy_perform(hnd);
  printf("ret: %d\n", ret);
  if (ret != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(ret));
    return NULL;
  } else {
    long status_code;
    curl_easy_getinfo(hnd, CURLINFO_RESPONSE_CODE, &status_code);
    if (status_code == 401) {
      fprintf(stderr, "Not authorized\n");
      Result *result = malloc(sizeof(Result));
      result->error = malloc(sizeof(GofileError));
      *(result->error) = NotAuthorized;
      result->data = NULL;
      return result;
    }
  }

  cJSON *json = cJSON_Parse(response);
  if (json == NULL) {
    fprintf(stderr, "get_content() Could not parse json: %s\n",
            cJSON_GetErrorPtr());
    exit(EXIT_FAILURE);
  }
  json = cJSON_GetObjectItem(json, "data");
  json = cJSON_GetObjectItem(json, "children");

  int children_size = cJSON_GetArraySize(json);
  Contents *children = malloc(sizeof(Contents));
  children->contents = malloc(sizeof(Content) * children_size);
  children->size = children_size;

  for (int i = 0; i < children_size; i++) {
    cJSON *item = cJSON_GetArrayItem(json, i);
    cJSON *url = cJSON_GetObjectItem(item, "link");
    cJSON *name = cJSON_GetObjectItem(item, "name");

    if (url == NULL) {
      fprintf(stderr, "get_content() url is NULL\n");
      exit(EXIT_FAILURE);
    }

    if (name == NULL) {
      fprintf(stderr, "get_content() name is NULL\n");
      exit(EXIT_FAILURE);
    }

    children->contents[i].url = strdup(url->valuestring);
    children->contents[i].name = strdup(name->valuestring);
  }

  cJSON_Delete(json);
  json = NULL;
  curl_slist_free_all(headers);
  headers = NULL;

  Result *result = malloc(sizeof(Result));
  result->error = NULL;
  result->data = children;
  return result;
}

void make_guest_account(CURL *hnd, size_t result_token_size,
                        char *result_token) {
  curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(hnd, CURLOPT_WRITEDATA, response);
  curl_easy_setopt(hnd, CURLOPT_URL, "https://api.gofile.io/accounts");
  curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, "{}");

  memset(response, 0, sizeof(response));
  ret = curl_easy_perform(hnd);
  if (ret != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(ret));
    exit(EXIT_FAILURE);
  }

  error = strdup(curl_easy_strerror(ret));

  cJSON *json = cJSON_Parse(response);
  if (json == NULL) {
    fprintf(stderr, "make_guest_account() Could not parse json: %s\n",
            cJSON_GetErrorPtr());
    exit(EXIT_FAILURE);
  }

  cJSON *data = cJSON_GetObjectItem(json, "data");
  cJSON *token = cJSON_GetObjectItem(data, "token");

  if (strlen(token->valuestring) > result_token_size) {
    fprintf(stderr, "make_guest_account() token is too long\n");
    exit(EXIT_FAILURE);
  }

  strcpy(result_token, token->valuestring);

  cJSON_Delete(json);
}

#endif
