#include "komens.h"
#include "helper_funcs.h"
#include "memory.h"
#include "net.h"
#include "types.h"
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <curses.h>
#include <cwchar>
#include <menu.h>
#include <ncurses.h>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

#define MAIN_WIN_PROPORTION 0.714f

#define MAIN_WIN_HIGHT (roundf(LINES * MAIN_WIN_PROPORTION))

#define DEFAULT_OFSET 4

using nlohmann::json;

std::vector<allocation> komens_allocated;

void insert_content(WINDOW *content_win, WINDOW *attachment_win,
                    const json &resp_from_api);

void komens_print_usage_message() {
  attron(COLOR_PAIR(COLOR_BLUE));
  mvprintw(LINES - 2, 0,
           "Use PageUp and PageDown to scoll down or up a page of items");
  mvprintw(LINES - 1, 0, "Arrow Keys to navigate (F1 to Exit)");
  attroff(COLOR_PAIR(COLOR_BLUE));
  refresh();
}

void komens_page(koment_type type) {
  current_allocated = &komens_allocated;

  const json resp_from_api = [&]() -> json {
    const char *types[] = {"/api/3/komens/messages/received",
                           "/api/3/komens/messages/sent",
                           "/api/3/komens/messages/noticeboard"};

    const std::string endpoint = types[type];
    return bakaapi::get_data_from_endpoint(endpoint, POST);
  }();

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

  complete_menu komens_choise_menu;
  komens_allocated.push_back({COMPLETE_MENU_TYPE, &komens_choise_menu, 1});

  size_t num_of_komens = resp_from_api["Messages"].size();
  komens_choise_menu.items = new ITEM *[num_of_komens + 1];
  komens_choise_menu.items_size = num_of_komens + 1;

  char **title_bufs = new char *[num_of_komens];
  komens_allocated.push_back({CHAR_PTR_ARRAY, title_bufs, num_of_komens});
  char **name_bufs = new char *[num_of_komens];
  komens_allocated.push_back({CHAR_PTR_ARRAY, name_bufs, num_of_komens});
  size_t max_item_lenght;
  {
    size_t max_title_lenght = 0;
    size_t max_name_lenght = 0;
    size_t tmp_lenght;
    char tmp_buf[1500];
    for (size_t i = 0; i < num_of_komens; i++) {
      strlcpy(tmp_buf,
              resp_from_api["Messages"][i]["Title"].get<std::string>().c_str(),
              sizeof(tmp_buf));

      tmp_lenght =
          resp_from_api["Messages"][i]["Title"].get<std::string>().length();

      if (tmp_lenght > max_title_lenght) {
        max_title_lenght = tmp_lenght;
      }

      title_bufs[i] = new char[strlen(tmp_buf) + 1];
      strlcpy(title_bufs[i], tmp_buf, strlen(tmp_buf) + 1);

      strlcpy(tmp_buf,
              resp_from_api["Messages"][i]["Sender"]["Name"]
                  .get<std::string>()
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

      komens_choise_menu.items[i] = new_item(title_bufs[i], name_bufs[i]);
    }
    max_item_lenght = 3 + max_title_lenght + 1 + max_name_lenght;
  }
  komens_choise_menu.items[num_of_komens] = nullptr;

  komens_choise_menu.menu = new_menu(komens_choise_menu.items);

  komens_choise_menu.win =
      newwin(MAIN_WIN_HIGHT, max_item_lenght + 1, DEFAULT_OFSET, DEFAULT_OFSET);
  komens_allocated.push_back({WINDOW_TYPE, komens_choise_menu.win, 1});

  set_menu_win(komens_choise_menu.menu, komens_choise_menu.win);
  set_menu_sub(komens_choise_menu.menu,
               derwin(komens_choise_menu.win, MAIN_WIN_HIGHT - 10,
                      max_item_lenght, DEFAULT_OFSET - 1, DEFAULT_OFSET - 3));
  set_menu_format(komens_choise_menu.menu, MAIN_WIN_HIGHT - 5, 1);

  set_menu_mark(komens_choise_menu.menu, " * ");

  box(komens_choise_menu.win, 0, 0);

  wprint_in_middle(komens_choise_menu.win, 1, 0, max_item_lenght, L"Komens",
                   COLOR_PAIR(1));
  mvwaddch(komens_choise_menu.win, 2, 0, ACS_LTEE);
  mvwhline(komens_choise_menu.win, 2, 1, ACS_HLINE, max_item_lenght - 1);
  mvwaddch(komens_choise_menu.win, 2, max_item_lenght, ACS_RTEE);

  post_menu(komens_choise_menu.menu);
  wrefresh(komens_choise_menu.win);

  WINDOW *content_win =
      newwin(MAIN_WIN_HIGHT, COLS - max_item_lenght - DEFAULT_OFSET - 1,
             DEFAULT_OFSET, DEFAULT_OFSET + max_item_lenght + 1);
  komens_allocated.push_back({WINDOW_TYPE, content_win, 1});

  WINDOW *attachment_win = newwin(1, 1, LINES, COLS);
  komens_allocated.push_back({WINDOW_TYPE, attachment_win, 1});

  insert_content(content_win, attachment_win,
                 resp_from_api["Messages"][item_index(
                     current_item(komens_choise_menu.menu))]);

  komens_print_usage_message();

  int c;
  while ((c = getch()) != KEY_F(1)) {
    switch (c) {
    case KEY_DOWN:
    case KEY_NPAGE:
    case 'j':
      menu_driver(komens_choise_menu.menu, REQ_DOWN_ITEM);
      break;

    case KEY_UP:
    case KEY_PPAGE:
    case 'k':
      menu_driver(komens_choise_menu.menu, REQ_UP_ITEM);
      break;
    default:
      if (c >= '1' && c <= '9') {
        size_t index = c - '0' - 1;
        if (index < resp_from_api["Messages"][item_index(
                        current_item(komens_choise_menu.menu))]["Attachments"]
                        .size()) {

          std::string default_path =
              "~/Downloads/" +
              resp_from_api["Messages"][item_index(current_item(
                  komens_choise_menu.menu))]["Attachments"][index]["Name"]
                  .get<std::string>();
          char path[256];

          // Create input prompt at bottom of screen
          move(LINES - 1, 0);
          clrtoeol();
          printw("Save path [%s]: ", default_path.c_str());
          echo();
          curs_set(1);
          getnstr(path, sizeof(path) - 1);
          if (strlen(path) == 0)
            strcpy(path, default_path.c_str());
          noecho();
          curs_set(0);
          move(LINES - 1, 0);
          clrtoeol();
          refresh();

          // Download the attachment
          bakaapi::download_attachment(
              resp_from_api["Messages"][item_index(current_item(
                  komens_choise_menu.menu))]["Attachments"][index]["Id"]
                  .get<std::string>(),
              path);
          komens_print_usage_message();
        }
      }
      break;
    }

    insert_content(content_win, attachment_win,
                   resp_from_api["Messages"][item_index(
                       current_item(komens_choise_menu.menu))]);
    wrefresh(komens_choise_menu.win);
  }
  unpost_menu(komens_choise_menu.menu);
  endwin();
  delete_all(&komens_allocated);
}

void insert_content(WINDOW *content_win, WINDOW *attachment_win,
                    const json &message) {
  wclear(content_win);
  mvwprintw(content_win, 0, 0, "%s",
            html_to_string(message.at("Text")).c_str());
  wrefresh(content_win);
  if (!message.at("Attachments").empty()) {

    size_t max_item_lenght = 0;
    {
      size_t max_name_lenght = 0;
      size_t max_size_lenght = 0;
      size_t tmp_lenght;

      for (size_t j = 0; j < message.at("Attachments").size(); j++) {
        tmp_lenght =
            message.at("Attachments")[j]["Name"].get<std::string>().length();
        if (tmp_lenght > max_name_lenght) {
          max_name_lenght = tmp_lenght;
        }

        tmp_lenght =
            std::to_string(message.at("Attachments")[j]["Size"].get<size_t>())
                .length();
        if (tmp_lenght > max_size_lenght) {
          max_size_lenght = tmp_lenght;
        }
      }

      max_item_lenght = 3 + max_name_lenght + 1 + max_size_lenght;
    }

    mvwin(attachment_win, MAIN_WIN_HIGHT + DEFAULT_OFSET + 1,
          COLS - max_item_lenght - 2);
    wresize(attachment_win, LINES - (MAIN_WIN_HIGHT + DEFAULT_OFSET + 1),
            max_item_lenght + 2);

    wborder(attachment_win, 0, ' ', 0, 0, 0, ACS_HLINE, 0, ACS_HLINE);
    print_in_middle(attachment_win, 0, 0, max_item_lenght + 2, "Attachments",
                    COLOR_PAIR(COLOR_RED));
    for (size_t j = 0; j < message.at("Attachments").size(); j++) {

      mvwprintw(
          attachment_win, j + 1, 2, "%s %s",
          message.at("Attachments")[j]["Name"].get<std::string>().c_str(),
          std::to_string(message.at("Attachments")[j]["Size"].get<size_t>())
              .c_str());

      wattron(attachment_win, COLOR_PAIR(COLOR_MAGENTA));
      mvwprintw(attachment_win, j + 1, 0, "%zu>", j + 1);
      wattroff(attachment_win, COLOR_PAIR(COLOR_MAGENTA));
    }
    { // remove duplicating spaces
      unsigned short attachment_win_top, attachment_win_left,
          attachment_win_height, attachment_win_width;
      getbegyx(attachment_win, attachment_win_top, attachment_win_left);
      getmaxyx(attachment_win, attachment_win_height, attachment_win_width);

      mvvline(attachment_win_top, attachment_win_left - 1, ' ',
              attachment_win_height);
    }
    refresh();

    wrefresh(attachment_win);
  }
}
