#include "komens.h"
#include "helper_funcs.h"
#include "memory.h"
#include "net.h"
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <curses.h>
#include <cwchar>
#include <menu.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#define WIN_HIGHT 40
#define DEFAULT_OFSET 4

using nlohmann::json;

std::vector<allocation> komens_allocated;

void insert_content(WINDOW *content_window, size_t i, json &resp_from_api);

void komens_page(koment_type type) {
  current_allocated = &komens_allocated;
  json resp_from_api;
  {
    /*std::ifstream f("test-data/komens.json");
    resp_from_api = json::parse(f);
    f.close();*/
    const char *types[] = {"/api/3/komens/messages/received",
                           "/api/3/komens/message",
                           "/api/3/komens/messages/noticeboard"};

    const std::string endpoint = types[type];
    resp_from_api = bakaapi::get_data_from_endpoint(endpoint, POST);
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
  size_t max_item_lenght;
  {
    size_t max_title_lenght = 0;
    size_t max_name_lenght = 0;
    size_t tmp_lenght;
    char tmp_buf[1500];
    for (size_t i = 0; i < num_of_komens; i++) {
      wcstombs(tmp_buf,
               string_to_wstring(
                   resp_from_api["Messages"][i]["Title"].get<std::string>())
                   .c_str(),
               sizeof(tmp_buf));

      tmp_lenght =
          resp_from_api["Messages"][i]["Title"].get<std::string>().length();

      if (tmp_lenght > max_title_lenght) {
        max_title_lenght = tmp_lenght;
      }

      title_bufs[i] = new char[strlen(tmp_buf) + 1];
      strlcpy(title_bufs[i], tmp_buf, strlen(tmp_buf) + 1);
      wcstombs(
          tmp_buf,
          string_to_wstring(
              resp_from_api["Messages"][i]["Sender"]["Name"].get<std::string>())
              .c_str(),
          sizeof(tmp_buf));

      tmp_lenght = resp_from_api["Messages"][i]["Sender"]["Name"]
                       .get<std::string>()
                       .length();

      if (tmp_lenght > max_name_lenght) {
        max_name_lenght = tmp_lenght;
      }

      name_bufs[i] = new char[strlen(tmp_buf) + 1];
      strlcpy(name_bufs[i], tmp_buf, strlen(tmp_buf) + 1);

      komens_items[i] = new_item(title_bufs[i], name_bufs[i]);
    }
    max_item_lenght = 3 + max_title_lenght + 1 + max_name_lenght;
  }
  komens_items[num_of_komens] = nullptr;

  MENU *komens_choise_menu = new_menu(komens_items);
  komens_allocated.push_back({MENU_TYPE, komens_choise_menu, 1});

  WINDOW *komens_choise_menu_win =
      newwin(WIN_HIGHT, max_item_lenght + 1, DEFAULT_OFSET, DEFAULT_OFSET);
  komens_allocated.push_back({WINDOW_TYPE, komens_choise_menu_win, 1});

  set_menu_win(komens_choise_menu, komens_choise_menu_win);
  set_menu_sub(komens_choise_menu,
               derwin(komens_choise_menu_win, WIN_HIGHT - 10, max_item_lenght,
                      DEFAULT_OFSET - 1, DEFAULT_OFSET - 3));
  set_menu_format(komens_choise_menu, WIN_HIGHT - 5, 1);

  set_menu_mark(komens_choise_menu, " * ");

  box(komens_choise_menu_win, 0, 0);

  wprint_in_middle(komens_choise_menu_win, 1, 0, max_item_lenght, L"Komens",
                   COLOR_PAIR(1));
  mvwaddch(komens_choise_menu_win, 2, 0, ACS_LTEE);
  mvwhline(komens_choise_menu_win, 2, 1, ACS_HLINE, max_item_lenght - 1);
  mvwaddch(komens_choise_menu_win, 2, max_item_lenght, ACS_RTEE);

  post_menu(komens_choise_menu);
  wrefresh(komens_choise_menu_win);

  WINDOW *content_window =
      newwin(WIN_HIGHT, COLS - max_item_lenght - DEFAULT_OFSET - 1,
             DEFAULT_OFSET, DEFAULT_OFSET + max_item_lenght + 1);
  komens_allocated.push_back({WINDOW_TYPE, content_window, 1});
  box(content_window, 0, 0);
  insert_content(content_window, item_index(current_item(komens_choise_menu)),
                 resp_from_api);
  wrefresh(content_window);

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
    insert_content(content_window, item_index(current_item(komens_choise_menu)),
                   resp_from_api);
    wrefresh(content_window);
    wrefresh(komens_choise_menu_win);
  }
  delete_all(&komens_allocated);
}

void insert_content(WINDOW *content_window, size_t i, json &resp_from_api) {
  wclear(content_window);
  mvwprintw(content_window, 0, 0, "%s",
            html_to_string(resp_from_api.at("Messages")[i]["Text"]).c_str());
}