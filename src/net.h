#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <string>

using nlohmann::json;

extern CURL *curl;
namespace bakaapi {
void login(std::string username, std::string password);
void refresh_access_token();
json get_grades();
} // namespace bakaapi