#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>
// header guard
#ifndef _ba_ne_hg_
#define _ba_ne_hg_

// metods
enum metod {
  GET,
  POST,
};

using nlohmann::json;

extern CURL *curl;
namespace bakaapi {
void login(std::string username, std::string password);
void refresh_access_token();
json get_data_from_endpoint(std::string &endpoint, metod metod,
                            std::string additional_data = "");
} // namespace bakaapi
#endif