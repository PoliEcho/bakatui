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
#include <curl/header.h>
#include <curses.h>
#include <dirent.h>
#include <fcntl.h>
#include <format>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <stdlib.h>
#include <string>
#include <unistd.h>

#define ROUND_UP(x, y) ((((x) + (y) - 1)) & ~((y) - 1))

using nlohmann::json;

std::string access_token;

CURL *curl = curl_easy_init();

// Callback function to write data into a std::string
size_t WriteCallback_to_string(void *contents, size_t size, size_t nmemb,
                               void *userp) {
  size_t totalSize = size * nmemb;
  static_cast<std::string *>(userp)->append((char *)contents, totalSize);
  return totalSize;
}

size_t WriteCallback_to_file(void *contents, size_t size, size_t nmemb,
                             void *userp) {
  const size_t sector_size = 4096;
  size_t total = size * nmemb;
  void *aligned_buf = aligned_alloc(sector_size, ROUND_UP(total, sector_size));

  // Zero out the buffer before copying
  memset(aligned_buf, 0, ROUND_UP(total, sector_size));
  memcpy(aligned_buf, contents, total);

  // Cast userp to fstream*
  std::fstream *fileStream = static_cast<std::fstream *>(userp);

  // Write to the file using fstream
  fileStream->write(static_cast<char *>(aligned_buf), total);

  // Check if write was successful
  if (fileStream->fail()) {
    free(aligned_buf);
    return 0;
  }

  free(aligned_buf);
  return total;
}

std::tuple<std::string, int> send_curl_request(
    std::string endpoint, metod type, std::string req_data,
    size_t (*WriteCallback_function)(void *, size_t, size_t,
                                     void *) = WriteCallback_to_string,
    std::fstream *fileStream = nullptr) {
  std::string response;
  std::string url = baka_api_url + endpoint;
  if (type == GET) {
    url.append("?" + req_data);
  }

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback_function);
    if (fileStream) {
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, fileStream);
    } else {
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    }
    if (config.ignoressl) {
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    }

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip, deflate");

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

    CURLcode curl_return_code = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    if (curl_return_code != CURLE_OK) {
      std::cerr << RED "[ERROR] " << RESET << "curl_easy_perform() failed: "
                << curl_easy_strerror(curl_return_code) << "\n";
      safe_exit(21);
    }
    if (fileStream) {
      fileStream->close();
    }

    int http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    return {response, http_code};

  } else {
    std::cerr << RED "[ERROR] " << RESET "curl not initialised\n";
    safe_exit(20);

    // prevent compiler warning
    return {"", -1};
  }
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
}

void refresh_access_token() {
  if (config.verbose) {
    std::clog << "refreshing access token please wait...\n";
  }

  json authfile_parsed = json::parse(SoRAuthFile(false, ""));

  std::string refresh_token = authfile_parsed["refresh_token"];

  std::string req_data =
      std::format("Authorization=Bearer&client_id=ANDR&grant_type=refresh_"
                  "token&refresh_token={}",
                  refresh_token);

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
json get_data_from_endpoint(const std::string &endpoint, metod metod,
                            std::string additional_data) {
  is_access_token_empty();
access_token_refreshed:
  std::string req_data =
      std::format("Authorization=Bearer&access_token={}", access_token);
  if (!additional_data.empty()) {
    req_data.append(std::format("&{}", additional_data));
  }

  auto [response, http_code] = send_curl_request(endpoint, metod, req_data);

  if (http_code != 200) {
    refresh_access_token();
    goto access_token_refreshed;
  }

  return json::parse(response);
}

int download_attachment(std::string id, std::string path) {
  if (config.verbose) {
    std::clog << "downloading attachment please wait...\n";
  }
  if (path[0] == '~') {
    path = std::string(std::getenv("HOME")) + path.substr(1);
  }

  std::string savedir_path = std::filesystem::path(path).parent_path().string();
  if (!std::filesystem::exists(savedir_path)) {
    if (!std::filesystem::create_directories(savedir_path)) {
      std::cerr << RED "[ERROR] " RESET
                << "Failed to create directory: " << savedir_path << "\n";
      safe_exit(EXIT_FAILURE);
    }
  }

  std::fstream fileStream(path, std::ios::out | std::ios::binary);
  if (!fileStream.is_open()) {
    std::cerr << RED "[ERROR] " RESET << "Failed to open file for writing\n";
    safe_exit(EXIT_FAILURE);
  }

  is_access_token_empty();
access_token_refreshed:
  std::string req_data =
      std::format("Authorization=Bearer&access_token={}", access_token);

  auto [response, http_code] =
      send_curl_request(std::format("/api/3/komens/attachment/{}", id), GET,
                        req_data, WriteCallback_to_file, &fileStream);
  if (http_code != 200) {
    if (config.verbose) {
      std::clog << BLUE "[LOG] " RESET << "download failed " << http_code
                << " is non 200 response\n";
    }
    refresh_access_token();
    goto access_token_refreshed;
  }
  return 0;
}

} // namespace bakaapi