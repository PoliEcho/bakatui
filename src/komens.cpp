#include "komens.h"
#include "helper_funcs.h"
#include "memory.h"
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <curses.h>
#include <cwchar>
#include <fstream>
#include <iostream>
#include <menu.h>
#include <nlohmann/json.hpp>
#include <vector>

using nlohmann::json;

std::vector<allocation> komens_allocated;

void komens_page() {
  current_allocated = &komens_allocated;
  json resp_from_api;
  {
    std::ifstream f("test-data/komens.json");
    resp_from_api = json::parse(f);
    f.close();
  }

  /* Initialize curses */
  setlocale(LC_ALL, "");
  initscr();
  start_color();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  /* Initialize all the colors */
  for (uint8_t i = 0; i < 8; i++) {
    init_pair(i, i, COLOR_BLACK);
  }
  size_t num_of_komens = resp_from_api["Messages"].size();
  ITEM **komens_items = new ITEM *[num_of_komens + 1];
  komens_allocated.push_back({ITEM_ARRAY, komens_items, num_of_komens});

  char **title_bufs = new char *[num_of_komens];
  char **name_bufs = new char *[num_of_komens];
  char tmp_buf[1500];
  for (size_t i = 0; i < num_of_komens; i++) {
    wcstombs(tmp_buf,
             string_to_wstring(
                 resp_from_api["Messages"][i]["Title"].get<std::string>())
                 .c_str(),
             sizeof(tmp_buf));
    title_bufs[i] = new char[strlen(tmp_buf) + 1];
    strlcpy(title_bufs[i], tmp_buf, strlen(tmp_buf));
    wcstombs(
        tmp_buf,
        string_to_wstring(
            resp_from_api["Messages"][i]["Sender"]["Name"].get<std::string>())
            .c_str(),
        sizeof(tmp_buf));

    name_bufs[i] = new char[strlen(tmp_buf) + 1];
    strlcpy(name_bufs[i], tmp_buf, strlen(tmp_buf));

    komens_items[i] = new_item(title_bufs[i], name_bufs[i]);
  }
  komens_items[num_of_komens] = nullptr;

  MENU *komens_choise_menu = new_menu(komens_items);
  komens_allocated.push_back({MENU_TYPE, komens_choise_menu, 0});

  WINDOW *komens_choise_menu_win = newwin(40, 80, 4, 4);
  komens_allocated.push_back({WINDOW_TYPE, komens_choise_menu_win, 0});

  set_menu_win(komens_choise_menu, komens_choise_menu_win);
  set_menu_sub(komens_choise_menu,
               derwin(komens_choise_menu_win, 30, 78, 3, 1));
  set_menu_format(komens_choise_menu, 29, 1);

  set_menu_mark(komens_choise_menu, " * ");

  box(komens_choise_menu_win, 0, 0);

  wprint_in_middle(komens_choise_menu_win, 1, 0, 40, L"Komens", COLOR_PAIR(1));
  mvwaddch(komens_choise_menu_win, 2, 0, ACS_LTEE);
  mvwhline(komens_choise_menu_win, 2, 1, ACS_HLINE, 38);
  mvwaddch(komens_choise_menu_win, 2, 39, ACS_RTEE);

  post_menu(komens_choise_menu);
  wrefresh(komens_choise_menu_win);

  attron(COLOR_PAIR(COLOR_BLUE));
  mvprintw(LINES - 2, 0,
           "Use PageUp and PageDown to scoll down or up a page of items");
  mvprintw(LINES - 1, 0, "Arrow Keys to navigate (F1 to Exit)");
  attroff(COLOR_PAIR(COLOR_BLUE));
  refresh();

  int c;
  while ((c = getch()) != KEY_F(1)) {
    switch (c) {
    case KEY_DOWN:
    case KEY_NPAGE:
    case 'j':
      menu_driver(komens_choise_menu, REQ_DOWN_ITEM);
      break;

    case KEY_UP:
    case KEY_PPAGE:
    case 'k':
      menu_driver(komens_choise_menu, REQ_UP_ITEM);
      break;
    }
    wrefresh(komens_choise_menu_win);
  }
  delete_all(&komens_allocated);
}