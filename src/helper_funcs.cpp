#include "net.h"
#include <csignal>
#include <curses.h>
#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include "color.h"

void safe_exit(int code) {
  switch (code) {
  case SIGTERM:
    std::cerr << "\nreceived SIGTERM exiting...\n";
    break;
  case SIGINT:
    std::cerr << "\nreceived SIGINT exiting...\n";
    break;
  case SIGQUIT:
    std::cerr << "\nreceived SIGQUIT exiting...\n";
    break;
  case SIGHUP:
    std::cerr << "\nreceived SIGHUP exiting...\n";
    break;

  case SIGSEGV:
    std::cerr << "\nreceived SIGSEGV(segmentaiton fault) exiting...\nIf this "
                 "repeats please report it as a bug\n";
    break;

  default:
    break;
  }

  curl_easy_cleanup(curl);
  endwin();

  exit(code);
}

std::string bool_to_string(bool bool_in) { return bool_in ? "true" : "false"; }

std::string SoRAuthFile(bool save, std::string data) {

    std::string home = std::getenv("HOME");
    if (home.empty()) {
        std::cerr << RED "[ERROR] " RESET << "HOME environment variable not set.\n";
        safe_exit(EXIT_FAILURE);
    }

    std::string savedir_path = home;
    savedir_path.append("/.local/share/bakatui");


    if (!std::filesystem::exists(savedir_path)) {
        if (!std::filesystem::create_directories(savedir_path)) {
            std::cerr << RED "[ERROR] " RESET << "Failed to create directory: " << savedir_path << "\n";
            safe_exit(EXIT_FAILURE);
        }
    }

    std::string authfile_path = savedir_path + "/auth";

    if (save) {
        std::ofstream authfile(authfile_path);
        if (!authfile.is_open()) {
            std::cerr << RED "[ERROR] " RESET << "Failed to open auth file for writing.\n";
            safe_exit(EXIT_FAILURE);
        }
        authfile << data;
        authfile.close();
        return "";
    } else {
        std::ifstream authfile(authfile_path);
        if (!authfile.is_open()) {
            std::cerr << RED "[ERROR] " RESET << "Failed to open auth file for reading.\n";
            safe_exit(EXIT_FAILURE);
        }
        data.assign((std::istreambuf_iterator<char>(authfile)), 
                    std::istreambuf_iterator<char>());
        authfile.close();
        return data;
    }
}

void get_input_and_login() {
  std::string username;
  std::cout << "enter username: ";
  std::cin >> username;
  std::string password;

  // password = getpass("enter password: ");
  // DEBUG
  std::cout << "\nenter password: ";
  std::cin >> password;

  bakaapi::login(username, password);
}

void print_in_middle(WINDOW *win, int starty, int startx, int width,
                     char *string, chtype color) {
  int length, x, y;
  float temp;

  if (win == NULL)
    win = stdscr;
  getyx(win, y, x);
  if (startx != 0)
    x = startx;
  if (starty != 0)
    y = starty;
  if (width == 0)
    width = 80;

  length = strlen(string);
  temp = (width - length) / 2;
  x = startx + (int)temp;
  wattron(win, color);
  mvwprintw(win, y, x, "%s", string);
  wattroff(win, color);
  refresh();
}


const std::string WHITESPACE = " \n\r\t\f\v";
 
std::string ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}
 
std::string rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}
 
std::string rm_tr_le_whitespace(const std::string &s) {
    return rtrim(ltrim(s));
}