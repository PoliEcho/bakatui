#include <csignal>
#include <curses.h>
#include <dirent.h>
#include <filesystem>
#include <iostream>
#include <stdio.h>
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

  exit(code);
}

std::string bool_to_string(bool bool_in) {
  return bool_in ? "true" : "false";
}

std::string SoRAuthFile(bool save, std::string data){

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

  if(save){

  } else {

  }
}


