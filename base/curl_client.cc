#include "curl_client.h"

#include <iostream>
#include <curl/curl.h>

#include "base/logging.h"

namespace base {

using namespace std;

static size_t WriteCallback(void *ptr, size_t size, size_t nmemb, void *data) {
	string * xml = (string *) data;
	xml->append((char *) ptr, size * nmemb);
	return size * nmemb;
}

CurlClient::CurlClient() {
  curl_global_init(CURL_GLOBAL_ALL);
}

int CurlClient::Get(const string & url, string * response) {
  static string empty;
  return DoRequest(url, HTTP_GET, empty, response);
}

int CurlClient::Post(const string & url, const string & post_body, string * response) {
  return DoRequest(url, HTTP_POST, post_body, response);
}

int CurlClient::DoRequest(const string & url, HttpMethod method, const string & post_body, string * response) {
  CURL *  curl = curl_easy_init();
  if (!curl) {
    return -1;
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);

  if (method == HTTP_POST) {
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (curl_off_t)post_body.length());
  } else {
    curl_easy_setopt(curl, CURLOPT_POST, 0);
  }

  {
    // TODO : 仔细 check curl 的 API
    // struct curl_slist * chunk = curl_slist_append(chunk, "Expect:"); // data lenght more than 1024
    // curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
    // TODO : use curl_slist_free_all() after the *perform() call to free this list again
  }

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    string msg = curl_easy_strerror(res);
    LOG_WARN("curl error : " << res << "-"<< msg);
    return -2;
  }

  long response_code = 0;
  res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

  if (res != CURLE_OK) {
    string msg = curl_easy_strerror(res);
    LOG_WARN("curl error : " << res << "-"<< msg);
    return -3;
  }
  LOG_DEBUG("curl responst code : " << response_code);
  if (response_code >= 500)
    return -4;

  curl_easy_cleanup(curl);
  return 0;
}

int CurlClient::PostWithResponse(const std::string & url, const std::string & post_body, std::string * response, const std::string * header) {
  return DoRequestWithResponse(url, HTTP_POST, post_body, response, header);
}

int CurlClient::GetWithResponse(const std::string & url, std::string * response, const std::string * header) {
  static string empty;
  return DoRequestWithResponse(url, HTTP_GET, empty, response, header);
}

int CurlClient::DoRequestWithResponse(const std::string & url, HttpMethod method, const std::string & post_body, std::string * response, const std::string * header) {
  CURL *  curl = curl_easy_init();
  if (!curl) {
    return -1;
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);

  if (method == HTTP_POST) {
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (curl_off_t)post_body.length());
  } else {
    curl_easy_setopt(curl, CURLOPT_POST, 0);
  }

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

  // 增加http header
  struct curl_slist *list = NULL;
  if (header != NULL) {
    list = curl_slist_append(list, header->c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
  }

  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    string msg = curl_easy_strerror(res);
    LOG_WARN("curl error : " << res << "-"<< msg);
    return -2;
  }

  long response_code = 0;
  res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

  if (res != CURLE_OK) {
    string msg = curl_easy_strerror(res);
    LOG_WARN("curl error : " << res << "-"<< msg);
    return -3;
  }
  LOG_DEBUG("curl responst code : " << response_code);
  if (response_code >= 500)
    return -4;

  curl_easy_cleanup(curl);
  if (list != NULL)
    curl_slist_free_all(list);
  return response_code; 
}

} // namespace base
