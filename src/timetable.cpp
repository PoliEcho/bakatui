#include "timetable.h"
#include "net.h"
#include <cstdint>
#include <format>
#include <fstream>
#include <iostream>
#include <ncurses.h>
#include <nlohmann/json.hpp>

using nlohmann::json;

void timetable_page() {
  // DONT FORGET TO UNCOMMENT
  // json resp_from_api =
  // bakaapi::get_data_from_endpoint("api/3/timetable/actual");
  std::ifstream f("test-data/timetable.json");
  json resp_from_api = json::parse(f);

  // calculate table size
  // some lambda dark magic
  const uint8_t num_of_columns = [&]() -> uint8_t {
    uint8_t result = 0;
    for (uint8_t i = 0; i < resp_from_api["Days"].size(); i++) {
      uint8_t currentSize = resp_from_api["Days"][i]["Atoms"].size();
      if (currentSize > result) {
        result = currentSize;
      }
    }
    return result;
  }();

  const uint8_t num_of_rows = resp_from_api["Days"].size();

  setlocale(LC_ALL, "");
  /* Initialize curses */
  initscr();
  start_color();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  printw("LINES: %d COLS: %d", LINES, COLS);
}