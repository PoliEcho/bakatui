#include "main.h"
#include "color.h"
#include "helper_funcs.h"
#include "main_menu.h"
#include "marks.h"
#include "net.h"
#include <csignal>
#include <cstdlib>
#include <curl/curl.h>
#include <curses.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <unistd.h>
#include "const.h"

std::string baka_api_url;

void PrintHelp() {
  std::cout << "Usage: " << NAME << " [OPTIONS]" << "\n"
  << "-h        Show this help menu\n"
  << "-v        Show version\n"
  << "-L        Force new login\n";
  safe_exit(0);
}

int main(int argc, char **argv) {
  // signal handlers
  signal(SIGTERM, safe_exit);
  signal(SIGINT, safe_exit);
  signal(SIGQUIT, safe_exit);
  signal(SIGHUP, safe_exit);

  // error signal handlers
  signal(SIGSEGV, safe_exit);

  {
    int opt;
    while ((opt = getopt(argc, argv, "vf:")) != -1) {  // "f:" expects a value
        switch (opt) {
            case 'h': PrintHelp(); break;
            case 'L': break;
            case 'v': break;
            default:  /* handle error */;
        }
    }

    std::string savedir_path = std::getenv("HOME");
    savedir_path.append("/.local/share/bakatui");
    std::string urlfile_path = std::string(savedir_path) + "/url";
    std::ifstream urlfile;
    urlfile.open(urlfile_path);
    urlfile >> baka_api_url;
    urlfile.close();
  }

  if (baka_api_url.empty()) {

    std::cout << "enter school bakalari url:\n";
    while (true) {
      std::cout << "(or q to quit )";
      std::cin >> baka_api_url;

      const std::regex url_regex_pattern(
          R"((http|https)://(www\.)?[a-zA-Z0-9@:%._\+~#?&//=]{2,256}\.[a-z]{2,6}(/\S*)?)");

      if (std::regex_match(baka_api_url, url_regex_pattern)) {
        break;
      } else if (baka_api_url == "q") {
        std::cerr << GREEN "[NOTICE] " << RESET "user quit\n";
        return 255;
      }
      std::cerr << "enter valid url using following pattern "
                   "[(http|https)://school.bakalari.url]\n";
    }
    if (baka_api_url.back() != '/') {
      baka_api_url.append("/");
    }

    get_input_and_login();
  }
  main_menu();
  
  return 0;
}