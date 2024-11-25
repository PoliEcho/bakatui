#include "net.h"
#include "color.h"
#include "helper_funcs.h"
#include "main.h"
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <curl/curl.h>
#include <dirent.h>
#include <errno.h>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

using nlohmann::json;

CURL *curl = curl_easy_init();

// Callback function to write data into a std::string
size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                     std::string *userp) {
  size_t totalSize = size * nmemb;
  userp->append((char *)contents, totalSize);
  return totalSize;
}

std::tuple<std::string, int> send_curl_request(std::string endpoint,
                                               std::string type,
                                               std::string req_data) {
  std::string response;
  std::string url = baka_api_url + endpoint;

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, req_data.c_str());

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(
        headers, "Content-Type: application/x-www-form-urlencoded");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    if (type == "POST") {
      curl_easy_setopt(curl, CURLOPT_POST, 1L);
    } else {
      std::cerr << RED "[ERROR] " << RESET "invalid method\n";
      safe_exit(-5);
    }

  } else {
    std::cerr << RED "[ERROR] " << RESET "curl not initialised\n";
    safe_exit(20);
  }
  curl_easy_perform(curl); // Perform the request

  int http_code = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

  curl_easy_cleanup(curl); // Cleanup

  return {response, http_code};
}
namespace bakaapi {

std::string access_token;

void login(std::string username, std::string password) {

  std::string req_data =
      std::format("client_id=ANDR&grant_type=password&username={}&password={}",
                  username, password);

  auto [response, http_code] = send_curl_request("api/login", "POST", req_data);
  if (http_code != 200) {
    std::cerr << RED "[ERROR] " << RESET << http_code
              << "is non 200 response\n";
    safe_exit(55);
  }

  std::string savedir_path = std::getenv("HOME");
  savedir_path.append("/.local/share/bakatui");

  DIR *savedir = opendir(savedir_path.c_str());
  if (savedir) {
    /* Directory exists. */
    closedir(savedir);
  } else if (ENOENT == errno) {
    /* Directory does not exist. */
    std::filesystem::create_directories(savedir_path);
  } else {
    /* opendir() failed for some other reason. */
    std::cerr << "cannot access ~/.local/share/bakatui\n";
    safe_exit(100);
  }
  {

    std::string authfile_path = std::string(savedir_path) + "/auth";
    std::ofstream authfile;
    authfile.open(authfile_path);
    authfile << response;
    authfile.close();
  }

  {
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
  std::string savedir_path = std::getenv("HOME");
  savedir_path.append("/.local/share/bakatui");
  std::string authfile_path = std::string(savedir_path) + "/auth";

  std::ifstream authfile(authfile_path);
  json authfile_parsed = json::parse(authfile);

  std::string refresh_token = authfile_parsed["refresh_token"];

  std::string req_data =
      std::format("Authorization=Bearer&client_id=ANDR&grant_type=refresh_"
                  "token&refresh_token={}",
                  refresh_token);

  auto [response, http_code] = send_curl_request("api/login", "POST", req_data);
  if (http_code != 200) {
    std::cerr << RED "[ERROR] " << RESET << http_code
              << "is non 200 response\n";
    safe_exit(55);
  }

  {
    std::ofstream authfile_out;
    authfile_out.open(authfile_path);
    authfile_out << response;
    authfile_out.close();
  }

  json resp_parsed = json::parse(response);

  access_token = resp_parsed["access_token"];
}

json get_grades() {
  if(access_token.empty()) {
    
  }
      std::string req_data =
      std::format("Authorization=Bearer&access_token={}",
                  access_token);

    auto [response, http_code] = send_curl_request("api/3/marks", "POST", req_data);

    

    
}
} // namespace bakaapi