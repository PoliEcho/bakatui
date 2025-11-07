#include "komens_menu.h"
#include "helper_funcs.h"
#include "komens.h"
#include "memory.h"
#include "net.h"
#include "types.h"
#include <cstdlib>
#include <cstring>
#include <curses.h>
#include <menu.h>

std::vector<allocation> komens_menu_allocated;

void komens_menu() {
  current_allocated = &komens_menu_allocated;
  wchar_t *choices[] = {
      L"received", L"sent", L"noticeboard", L"Exit", nullptr,
  };

  complete_menu komens_menu;
  komens_menu_allocated.push_back({COMPLETE_MENU_TYPE, &komens_menu, 1});

  int c;
  int n_choices;

  /* Initialize curses */
  setlocale(LC_ALL, "");
  initscr();
  start_color();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_CYAN, COLOR_BLACK);

  /* Create items */
  n_choices = ARRAY_SIZE(choices);
  komens_menu.items = new ITEM *[ARRAY_SIZE(choices)];
  komens_menu.items_size = ARRAY_SIZE(choices);

  for (int i = 0; i < n_choices; ++i)

    komens_menu.items[i] =
        new_item(wchar_to_char(choices[i]), wchar_to_char(choices[i]));

  /* Crate menu */
  komens_menu.menu = new_menu(komens_menu.items);

  /* Create the window to be associated with the menu */
  komens_menu.win = newwin(12, 40, 4, 4);
  keypad(komens_menu.win, TRUE);

  /* Set main window and sub window */
  set_menu_win(komens_menu.menu, komens_menu.win);
  set_menu_sub(komens_menu.menu, derwin(komens_menu.win, 8, 38, 3, 1));
  set_menu_format(komens_menu.menu, 7, 1);

  /* Set menu mark to the string " * " */
  set_menu_mark(komens_menu.menu, " * ");

  /* Print a border around the main window and print a title */
  box(komens_menu.win, 0, 0);

  wprint_in_middle(komens_menu.win, 1, 0, 40, L"Komens Menu", COLOR_PAIR(1));
  mvwaddch(komens_menu.win, 2, 0, ACS_LTEE);
  mvwhline(komens_menu.win, 2, 1, ACS_HLINE, 38);
  mvwaddch(komens_menu.win, 2, 39, ACS_RTEE);

  /* Post the menu */
  post_menu(komens_menu.menu);
  wrefresh(komens_menu.win);

  attron(COLOR_PAIR(2));
  mvprintw(LINES - 2, 0,
           "Use PageUp and PageDown to scoll down or up a page of items");
  mvprintw(LINES - 1, 0, "Arrow Keys to navigate (F1 to Exit)");
  attroff(COLOR_PAIR(2));
  refresh();

  while ((c = getch()) != KEY_F(1)) {
    switch (c) {
    case KEY_DOWN:
    case KEY_NPAGE:
    case 'j':
      menu_driver(komens_menu.menu, REQ_DOWN_ITEM);
      break;

    case KEY_UP:
    case KEY_PPAGE:
    case 'k':
      menu_driver(komens_menu.menu, REQ_UP_ITEM);
      break;
    case 10: // ENTER
      clear();
      if (item_index(current_item(komens_menu.menu)) == n_choices - 1) {
        goto close_menu;
      }
      komens_page(
          static_cast<koment_type>(item_index(current_item(komens_menu.menu))));
      current_allocated = &komens_menu_allocated;
      clear();
      pos_menu_cursor(komens_menu.menu);
      wrefresh(komens_menu.win);
      refresh();
      redrawwin(komens_menu.win);
      break;
    }
    wrefresh(komens_menu.win);
  }
close_menu:

  /* Unpost and free all the memory taken up */
  unpost_menu(komens_menu.menu);
  delete_all(&komens_menu_allocated);
  endwin();
}
