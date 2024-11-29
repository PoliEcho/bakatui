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
                 "repeats please report it as bug\n";
    break;

  default:
    break;
  }

  curl_easy_cleanup(curl);

  exit(code);
}

std::string bool_to_string(bool bool_in) { return bool_in ? "true" : "false"; }

std::string SoRAuthFile(bool save, std::string data) {

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

  std::string authfile_path = std::string(savedir_path) + "/auth";

  if (save) {
    std::ofstream authfile(authfile_path);
    authfile << data;
    authfile.close();
    return "";
  } else {
    std::ifstream authfile(authfile_path);
    authfile >> data;
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