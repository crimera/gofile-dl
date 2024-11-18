#include "gofile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void download(char *url, char *token) {
  char *ariacmd =
      "aria2c.exe -x16 --header=\"Authorization: Bearer %s\" \"%s\"";
  size_t command_size = strlen(ariacmd) + strlen(url) + strlen(token) - 4 + 1;

  char command[command_size];
  snprintf(command, command_size, ariacmd, token, url);

  printf("command: %s\n", command);

  system(command);
}

int main(int argc, char *argv[]) {
  CURLU *url = curl_url();
  curl_url_set(url, CURLUPART_URL, argv[1], 0);
  char *path;
  CURLUcode ret = curl_url_get(url, CURLUPART_PATH, &path, 0);

  if (ret) {
    fprintf(stderr, "curl_url_get() failed\n");
    exit(EXIT_FAILURE);
  }

  char file_id[8] = {0};
  get_content_id(path, file_id);
  curl_url_cleanup(url);

  CURL *hnd = curl_easy_init();
  char *token = "JrOAXhQQ8OIBd4LyY4iL2tjweaMcACBt";

  Contents *files = get_content(hnd, file_id, token);
  if (files == NULL) {
    fprintf(stderr, "get_content() failed\n");
  }

  for (size_t i = 0; i < files->size; i++) {
    printf("Downloading: %s\n", files->contents[i].url);

    download(files->contents[i].url, token);
  }

  free(files);
  curl_easy_cleanup(hnd);

  return EXIT_SUCCESS;
}
