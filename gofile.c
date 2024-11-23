#include "gofile.h"
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Response *new_response() {
  Response *response = (Response *)malloc(sizeof(Response));

  if (response != NULL) {
    response->data = NULL;
    response->size = 0;
  }

  return response;
}

void free_response(Response *resp) {
  if (resp != NULL) {
    free(resp->data);
    free(resp);
  }
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  size_t total_size = size * nmemb;
  Response *response = (Response *)userdata;

  char *temp = (char *)realloc(response->data, response->size + total_size + 1);
  if (temp == NULL) {
    printf("Failed to allocate\n");
  } else {
    response->data = temp;
  }

  memcpy(&(response->data[response->size]), ptr, total_size);
  response->size += total_size;
  response->data[response->size] = '\0';

  return total_size;
}

char *get_content_id(const char *url) {
  const char *index = strrchr(url, '/');
  return strdup(index + 1);
}

void gofile_set_curlopts(CURL *hnd) {
  curl_easy_setopt(hnd, CURLOPT_USERAGENT, GOFILE_USER_AGENT);
  curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_callback);
}

char *fetch_guest_account(CURL *hnd) {
  CURLcode res;
  Response *response = new_response();

  curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(hnd, CURLOPT_WRITEDATA, response);
  curl_easy_setopt(hnd, CURLOPT_URL, "https://api.gofile.io/accounts");
  curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, "{}");

  res = curl_easy_perform(hnd);
  if (res != CURLE_OK) {
    printf("Failed to perform curl request");
    return NULL;
  }

  char *out = strdup(response->data);

  free_response(response);

  return out;
}

char *get_account_token(CURL *hnd) {

  char *result = fetch_guest_account(hnd);

  cJSON *json = cJSON_Parse(result);
  if (json == NULL) {
    fprintf(stderr, "failed to parse json: %s", cJSON_GetErrorPtr());
  }

  cJSON *data = cJSON_GetObjectItem(json, "data");
  cJSON *token = cJSON_GetObjectItem(data, "token");

  char *out = strdup(token->valuestring);

  cJSON_Delete(json);
  free(result);

  return out;
}

char *fetch_content(CURL *hnd, char *token, char *file_id) {
  CURLcode res;
  Response *response = new_response();

  struct curl_slist *headers;

  char token_header[22 + 33] = "Authorization: Bearer ";
  strncat(token_header, token, 32);

  headers = NULL;
  headers = curl_slist_append(headers, token_header);

  char content_url[64] = {0};
  snprintf(content_url, sizeof(content_url), CONTENT_URL, file_id);

  curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(hnd, CURLOPT_WRITEDATA, response);
  curl_easy_setopt(hnd, CURLOPT_URL, content_url);
  curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(hnd, CURLOPT_HTTPGET, 1L);

  res = curl_easy_perform(hnd);
  if (res != CURLE_OK) {
    fprintf(stderr, "fetch_content() curl failed: %s", curl_easy_strerror(res));
    free_response(response);
    return NULL;
  }

  char *out = strdup(response->data);
  curl_slist_free_all(headers);
  free_response(response);

  return out;
}

Content *get_files(char *content) {
  // TODO: check for parsing errors
  cJSON *json = cJSON_Parse(content);

  cJSON *status = cJSON_GetObjectItem(json, "status");
  if (strcmp(status->valuestring, GOFILE_FILESTATUS_NOTFOUND) == 0) {
    printf("get_files(): File not found\n");
  }

  cJSON *data = cJSON_GetObjectItem(json, "data");

  cJSON *children = cJSON_GetObjectItem(data, "children");
  size_t count = cJSON_GetArraySize(children);

  Content *contents = (Content *)malloc(sizeof(Content));
  contents->files = (File *)malloc(sizeof(File) * count);
  contents->children_count = count;

  for (int i = 0; i < count; i++) {
    cJSON *item = cJSON_GetArrayItem(children, i);
    cJSON *url = cJSON_GetObjectItem(item, "link");
    cJSON *name = cJSON_GetObjectItem(item, "name");

    contents->files[i].url = strdup(url->valuestring);
    contents->files[i].name = strdup(name->valuestring);
  }

  cJSON_Delete(json);

  return contents;
}

void free_files(Content *content) {
  if (content == NULL) {
    return;
  }

  for (size_t i = 0; i < content->children_count; ++i) {
    free(content->files[i].name);
    free(content->files[i].url);
  }

  free(content->files);
  content->files = NULL;
  content->children_count = 0;

  free(content);
}
