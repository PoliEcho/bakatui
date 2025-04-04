#include "const.h"
#include "helper_funcs.h"
#include "memory.h"
#include "net.h"
#include <bits/chrono.h>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <curses.h>
#include <cwchar>
#include <ncurses.h>
#include <nlohmann/json.hpp>
#include <panel.h>
#include <string>
#include <sys/types.h>
#include <vector>

using nlohmann::json;

std::vector<allocation> komens_allocated;

void komens_page() {
  current_allocated = &komens_allocated;
  json resp_from_api;
  {
    std::string endpoint = "api/3/komens";
    resp_from_api = bakaapi::get_data_from_endpoint(endpoint, GET);
  }
  setlocale(LC_ALL, "");
  /* Initialize curses */
  initscr();
  start_color();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  /* Initialize all the colors */
  for (uint8_t i = 0; i < 8; i++) {
    init_pair(i, i, COLOR_BLACK);
  }
}