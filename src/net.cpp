#include "net.h"
#include "color.h"
#include "const.h"
#include "helper_funcs.h"
#include "main.h"
#include <cerrno>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <curl/curl.h>
#include <curses.h>
#include <dirent.h>
#include <format>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>

using nlohmann::json;

// metods
enum {
  GET,
  POST,
};

std::string access_token;

CURL *curl = curl_easy_init();

// Callback function to write data into a std::string
size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                     std::string *userp) {
  size_t totalSize = size * nmemb;
  userp->append((char *)contents, totalSize);
  return totalSize;
}

std::tuple<std::string, int>
send_curl_request(std::string endpoint, uint8_t type, std::string req_data) {
  std::string response;
  std::string url = baka_api_url + endpoint;
  if (type == GET) {
    url.append("?" + req_data);
  }

  if (curl) {
    // DEBUG
    // std::clog << BLUE"[LOG]" << RESET" sending to endpoint: " << url << "\n";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    if (config.ignoressl) {
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(
        headers, "Content-Type: application/x-www-form-urlencoded");
    headers = curl_slist_append(
        headers, std::format("User-Agent: bakatui/{}", VERSION).c_str());

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    switch (type) {
    case GET:
      curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
      break;
    case POST:
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req_data.c_str());
      curl_easy_setopt(curl, CURLOPT_POST, 1L);
      break;
    default:
      std::cerr << RED "[ERROR] " << RESET "invalid metod\n";
      safe_exit(EINVAL);
    }

  } else {
    std::cerr << RED "[ERROR] " << RESET "curl not initialised\n";
    safe_exit(20);
  }
  curl_easy_perform(curl); // Perform the request

  int http_code = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

  return {response, http_code};
}
namespace bakaapi {

void login(std::string username, std::string password) {

  std::string req_data =
      std::format("client_id=ANDR&grant_type=password&username={}&password={}",
                  username, password);

  auto [response, http_code] = send_curl_request("api/login", POST, req_data);
  if (http_code != 200) {
    std::cerr << RED "[ERROR] " << RESET << "login failed " << http_code
              << " is non 200 response\n";
    safe_exit(55);
  }

  SoRAuthFile(true, response);

  {
    std::string savedir_path = std::getenv("HOME");
    savedir_path.append("/.local/share/bakatui");

    std::string urlfile_path = std::string(savedir_path) + "/url";
    std::ofstream urlfile;
    urlfile.open(urlfile_path);
    urlfile << baka_api_url;
    urlfile.close();
  }

  json resp_parsed = json::parse(response);

  access_token = resp_parsed["access_token"];

  // DEBUG
  std::cout << "access token: " << access_token << std::endl;
}

void refresh_access_token() {
  // DEBUG
  std::clog << "refreshing access token please wait...\n";

  json authfile_parsed = json::parse(SoRAuthFile(false, ""));

  std::string refresh_token = authfile_parsed["refresh_token"];

  std::string req_data =
      std::format("Authorization=Bearer&client_id=ANDR&grant_type=refresh_"
                  "token&refresh_token={}",
                  refresh_token);

  // DEBUG
  std::clog << "calling send_curl_request() with folowing req_data\n"
            << req_data << std::endl;
  auto [response, http_code] = send_curl_request("api/login", POST, req_data);
  if (http_code != 200) {
    std::cerr << RED "[ERROR] " << RESET << http_code
              << "is non 200 response\n";
    get_input_and_login();
  }

  SoRAuthFile(true, response);

  json resp_parsed = json::parse(response);

  access_token = resp_parsed["access_token"];
}
void is_access_token_empty() {
  if (access_token.empty()) {
    json authfile_parsed = json::parse(SoRAuthFile(false, ""));
    access_token = authfile_parsed["access_token"];
  }
}

// supports all endpoints that only require access_token
json get_data_from_endpoint(std::string endpoint) {
  is_access_token_empty();
  std::string req_data =
      std::format("Authorization=Bearer&access_token={}", access_token);

  auto [response, http_code] = send_curl_request(endpoint, GET, req_data);

  if (http_code != 200) {
    // DEBUG
    std::clog << "Failed geting data from endpoint: " << endpoint
              << " code: " << http_code << "\nrequest: " << req_data
              << "\nresponse: " << response << std::endl;
    refresh_access_token();
  }

  return json::parse(response);
}
} // namespace bakaapi