#include <csignal>
#include <curses.h>
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
  if (bool_in) {
    return "true";
  } else {
    return "false";
  }
}