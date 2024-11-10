#include "main.h"
#include "color.h"
#include "helper_funcs.h"
#include "net.h"
#include <csignal>
#include <curl/curl.h>
#include <curses.h>
#include <iostream>
#include <regex>
#include <string>
#include <unistd.h>

std::string baka_api_url;

int main(int argc, char **argv) {
  // signal handlers
  signal(SIGTERM, safe_exit);
  signal(SIGINT, safe_exit);
  signal(SIGQUIT, safe_exit);
  signal(SIGHUP, safe_exit);

  // error signal handlers
  signal(SIGSEGV, safe_exit);

  /*initscr();

  // creating a window;
  // with height = 15 and width = 10
  // also with start x axis 10 and start y axis = 20
  WINDOW *win = newwin(15, 17, 2, 10);
  refresh();

  // making box border with default border styles
  box(win, 0, 0);

  // move and print in window
  mvwprintw(win, 0, 1, "Greeter");
  mvwprintw(win, 1, 1, "Hello");

  // refreshing the window
  wrefresh(win);

  getch();
  endwin(); */
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

  {
    std::string username;
    std::cout << "enter username: ";
    std::cin >> username;
    std::string password;

    password = getpass("enter password: ");
    // DEBUG
    // std::cout << "\nenter password: ";
    // std::cin >> password;

    bakaapi::login(username, password);
  }
  return 0;
}