#include <curl/curl.h>
#include <string.h>

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  size_t total_size = size * nmemb;
  char *buffer = (char *)userdata;

  // Append the received data to the buffer
  strncat(buffer, (char *)ptr, total_size);

  return total_size;
}

int main() {
  CURLcode ret;
  CURL *hnd;
  struct curl_slist *headers;

  headers = NULL;
  headers = curl_slist_append(
      headers, "Authorization: Bearer JrOAXhQQ8OIBd4LyY4iL2tjweaMcACBt");

  char response[102400] = {0};

  hnd = curl_easy_init();
  curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(hnd, CURLOPT_WRITEDATA, response);
  curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, 102400L);
  curl_easy_setopt(hnd, CURLOPT_URL,
                   "https://api.gofile.io/contents/lCkne3?wt=4fd6sg89d7s6");
  curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
  curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(hnd, CURLOPT_USERAGENT,
                   "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:132.0) "
                   "Gecko/20100101 Firefox/132.0");
  curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
  curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
  curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, 1L);
  curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

  ret = curl_easy_perform(hnd);

  curl_easy_cleanup(hnd);
  hnd = NULL;
  curl_slist_free_all(headers);
  headers = NULL;

  return (int)ret;
}
