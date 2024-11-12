#include <curl/curl.h>
#include <string>
namespace bakaapi {
void login(std::string username, std::string password);
void refresh_access_token();
bool is_logged_in();
} // namespace bakaapi