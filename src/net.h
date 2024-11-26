#include <curl/curl.h>
#include <string>
namespace bakaapi {
void login(std::string username, std::string password);
void refresh_access_token();
} // namespace bakaapi