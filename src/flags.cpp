#include "const.h"
#include <iostream>
#include "helper_funcs.h"
#include <cstdio>

void PrintHelp() {
    std::cout << "Usage: " << NAME << " [OPTIONS]" << "\n"
    << "-h        Show this help menu\n"
    << "-V        Show version\n"
    << "-v        verbose mode\n"
    << "-L        Force new login\n"
    << "-S        Ignore SSL cert validity\n";
    safe_exit(0);
  }
  
void PrintVersion() {
    std::cout << NAME" " << VERSION"\n" << "License GPLv3: GNU GPL version 3 <https://gnu.org/licenses/gpl.html>.\n";
    safe_exit(0);
  }

void DeleteLogin(std::string savedir_path) {
    std::remove((savedir_path + "/authfile").c_str());
    std::remove((savedir_path + "/urlfile").c_str());
}