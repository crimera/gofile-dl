#include "gofile.h"
#include <curl/easy.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

char *get_saved_token() {
  FILE *file = fopen("token.txt", "r");
  if (file == NULL) {
    return NULL;
  }

  char token[33];
  fscanf(file, "%32s", token);
  char *out = strdup(token);

  fclose(file);

  return out;
}

void save_token(char *token) {
  FILE *file = fopen("token.txt", "w");
  if (file == NULL) {
    perror("Error opening file");
    exit(1);
  }

  fprintf(file, "%s", token);
  fclose(file);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <file_id> [-d <directory>]\n", argv[0]);
    return 1;
  }

  char *directory = NULL;
  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-d") == 0) {
      if (i + 1 < argc) {
        directory = argv[i + 1];
      } else {
        fprintf(stderr, "No directory specified\n");
        return 1;
      }
    }
  }

  CURL *curl = curl_easy_init();
  gofile_set_curlopts(curl);

  char *token = get_saved_token();
  if (token == NULL) {
    if (errno == ENOENT) {
      printf("No token found, generating one\n");
      token = get_account_token(curl);
      save_token(token);
    } else {
      perror("Error reading token");
      return 1;
    }
  }

  printf("token: %s\n", token);

  // working "wm0FjD"
  // Not working "5wQvT1"
  char *file_id = get_content_id(argv[1]);
  printf("file id: %s\n", file_id);

  char *content = fetch_content(curl, token, file_id);
  printf("content: %s\n", content);

  free(file_id);

  Content *files = get_files(content);
  for (int i = 0; i < files->children_count; i++) {
    download(files->files[i].url, token, directory);
  }

  free_files(files);
  free(content);
  free(token);

  curl_easy_cleanup(curl);
  return 0;
}
