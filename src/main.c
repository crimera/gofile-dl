#include "gofile.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: allow for specifying custom save path
void download(char *url, char *token, char *path) {
  char *outpath = "";

  if (path != NULL) {
    asprintf(&outpath, "-d %s", path);
  }

  char *ariacmd =
      "aria2c -x16 %s --continue --header=\"Authorization: Bearer %s\" \"%s\"";

  char *command;
  if (asprintf(&command, ariacmd, outpath, token, url) == -1) {
    printf("Failed to allocate memory for command\n");
  }

  printf("command: %s\n", command);

  system(command);
  free(command);
  if (strcmp(outpath, "") != 0) {
    free(outpath);
  }
}

// TODO: token.txt should be put in a tmp file
void make_guest_account_and_save(CURL *hnd, size_t token_size, char *token) {
  make_guest_account(hnd, token_size, token);
  FILE *file = fopen("token.txt", "w");
  fwrite(token, strlen(token), 1, file);
  fclose(file);
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <url>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

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
  curl_easy_setopt(hnd, CURLOPT_USERAGENT, GOFILE_USER_AGENT);
  char token[33];

  FILE *file = fopen("token.txt", "r");
  if (file == NULL) {
    if (errno == ENOENT) {
      fprintf(stderr, "token.txt not found\n");
      make_guest_account_and_save(hnd, sizeof token, token);

      file = fopen("token.txt", "r");
      if (file == NULL) {
        fprintf(stderr, "Failed creating token.txt file: %s \n",
                strerror(errno));
        exit(EXIT_FAILURE);
      }
    } else {
      fprintf(stderr, "Failed to open token.txt: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  fscanf(file, "%s", token);
  fclose(file);

  printf("token: %s\n", token);

  Result *files = get_content(hnd, file_id, token);
  if (files->error != NULL) {
    if (*(files->error) == NotAuthorized) {
      make_guest_account_and_save(hnd, sizeof token, token);
    } else {
      fprintf(stderr, "Failed getting contents\n");
    }

    free_result(files);
  }

  Contents *contents = files->data;
  for (size_t i = 0; i < contents->size; i++) {
    printf("Downloading: %s\n", contents->contents[i].url);

    download(contents->contents[i].url, token, argv[2]);
  }

  free(files);
  curl_easy_cleanup(hnd);

  return EXIT_SUCCESS;
}
