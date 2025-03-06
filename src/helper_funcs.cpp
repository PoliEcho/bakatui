#include "helper_funcs.h"
#include "color.h"
#include "net.h"
#include <codecvt>
#include <csignal>
#include <curses.h>
#include <dirent.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>
#include <string>
#include <termios.h>
#include <unistd.h>

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
      std::cerr << RED "[ERROR] " RESET
                << "Failed to create directory: " << savedir_path << "\n";
      safe_exit(EXIT_FAILURE);
    }
  }

  std::string authfile_path = savedir_path + "/auth";

  if (save) {
    std::ofstream authfile(authfile_path);
    if (!authfile.is_open()) {
      std::cerr << RED "[ERROR] " RESET
                << "Failed to open auth file for writing.\n";
      safe_exit(EXIT_FAILURE);
    }
    authfile << data;
    authfile.close();
    return "";
  } else {
    std::ifstream authfile(authfile_path);
    if (!authfile.is_open()) {
      std::cerr << RED "[ERROR] " RESET
                << "Failed to open auth file for reading.\n";
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

  password = getpass("enter password: ");
  // DEBUG
  // std::cout << "\nenter password: ";
  // std::cin >> password;

  bakaapi::login(username, password);
}

// Original function
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

// Wide character version
void wprint_in_middle(WINDOW *win, int starty, int startx, int width,
                      wchar_t *string, chtype color) {
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

  length = wcslen(string);
  temp = (width - length) / 2;
  x = startx + (int)temp;
  wattron(win, color);
  mvwaddwstr(win, y, x, string);
  wattroff(win, color);
  refresh();
}

const std::string WHITESPACE = " \n\r\t\f\v";
const std::wstring WWHITESPACE = L" \n\r\t\f\v";

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

// Wide character versions
std::wstring wltrim(const std::wstring &s) {
  size_t start = s.find_first_not_of(WWHITESPACE);
  return (start == std::wstring::npos) ? L"" : s.substr(start);
}

std::wstring wrtrim(const std::wstring &s) {
  size_t end = s.find_last_not_of(WWHITESPACE);
  return (end == std::wstring::npos) ? L"" : s.substr(0, end + 1);
}

std::wstring wrm_tr_le_whitespace(const std::wstring &s) {
  return wrtrim(wltrim(s));
}

// Conversion utilities
char *wchar_to_char(wchar_t *src) {
  if (!src)
    return nullptr;

  size_t len = wcslen(src) + 1; // +1 for null terminator
  char *dest = new char[len * MB_CUR_MAX];

  std::wcstombs(dest, src, len * MB_CUR_MAX);
  return dest;
}

wchar_t *char_to_wchar(char *src) {
  if (!src)
    return nullptr;

  size_t len = strlen(src) + 1; // +1 for null terminator
  wchar_t *dest = new wchar_t[len];

  std::mbstowcs(dest, src, len);
  return dest;
}

std::wstring string_to_wstring(const std::string &str) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  return converter.from_bytes(str);
}

std::string wstring_to_string(const std::wstring &wstr) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  return converter.to_bytes(wstr);
}