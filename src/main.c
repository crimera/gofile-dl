#include "gofile.h"

int main(int argc, char *argv[]) {
  CURL *hnd = curl_easy_init();

  Contents *files =
      get_content(hnd, "lCkne3", "JrOAXhQQ8OIBd4LyY4iL2tjweaMcACBt");
  if (files == NULL) {
    fprintf(stderr, "get_content() failed\n");
  }

  for (size_t i = 0; i < files->size; i++) {
    printf("Name: %s\n", files->contents[i].name);
    printf("URL: %s\n", files->contents[i].url);
  }

  curl_easy_cleanup(hnd);
  hnd = NULL;

  return EXIT_SUCCESS;
}
