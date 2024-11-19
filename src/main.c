#include "gofile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void download(char *url, char *token)
{
  char *ariacmd =
      "aria2c.exe -x16 --continue --header=\"Authorization: Bearer %s\" \"%s\"";
  size_t command_size = strlen(ariacmd) + strlen(url) + strlen(token) - 4 + 1;

  char command[command_size];
  snprintf(command, command_size, ariacmd, token, url);

  printf("command: %s\n", command);

  system(command);
}

void make_guest_account_and_save(CURL *hnd, size_t token_size, char *token, FILE *file) {
      make_guest_account(hnd, token_size, token);
      file = fopen("token.txt", "w");
      fwrite(token, token_size, 1, file);
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    fprintf(stderr, "Usage: %s <url>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  CURLU *url = curl_url();
  curl_url_set(url, CURLUPART_URL, argv[1], 0);
  char *path;
  CURLUcode ret = curl_url_get(url, CURLUPART_PATH, &path, 0);

  if (ret)
  {
    fprintf(stderr, "curl_url_get() failed\n");
    exit(EXIT_FAILURE);
  }

  char file_id[8] = {0};
  get_content_id(path, file_id);
  curl_url_cleanup(url);

  CURL *hnd = curl_easy_init();
  curl_easy_setopt(hnd, CURLOPT_USERAGENT, GOFILE_USER_AGENT);
  char token[33];

  FILE *file = fopen("token.txt", "rw");
  if (file == NULL)
  {
    if (errno == ENOENT)
    {
      fprintf(stderr, "token.txt not found\n");
      make_guest_account_and_save(hnd, sizeof token, token, file);
    }
    else
    {
      fprintf(stderr, "Failed to open token.txt: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  fscanf(file, "%s", token);
  fclose(file);

  printf("token: %s\n", token);

  Result *files = get_content(hnd, file_id, token);
  if (files->error != NULL)
  {
    if (*(files->error) == NotAuthorized)
    {
      make_guest_account_and_save(hnd, sizeof token, token, file);
    } else {
      fprintf(stderr, "Failed getting contents\n");
    }
  }

  Contents *contents = files->data;
  for (size_t i = 0; i < contents->size; i++)
  {
    printf("Downloading: %s\n", contents->contents[i].url);

    download(contents->contents[i].url, token);
  }

  free(files);
  curl_easy_cleanup(hnd);

  return EXIT_SUCCESS;
}
