#include "helper_funcs.h"
#include "komens_menu.h"
#include "marks.h"
#include "memory.h"
#include "net.h"
#include "timetable.h"
#include "types.h"
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <curses.h>
#include <menu.h>

std::vector<allocation> main_menu_allocated;

void main_menu() {
  current_allocated = &main_menu_allocated;
  const wchar_t *choices[] = {
      L"login",    L"Marks",   L"timetable", L"Komens",
      L"Homework", L"Absence", L"Exit",      nullptr,
  };
  void (*choicesFuncs[])() = {nullptr, marks_page, timetable_page, komens_menu,
                              nullptr, nullptr,    nullptr,        nullptr};

  complete_menu main_menu;
  main_menu_allocated.push_back({COMPLETE_MENU_TYPE, &main_menu, 1});

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

  main_menu.items = new ITEM *[ARRAY_SIZE(choices)];
  main_menu.items_size = ARRAY_SIZE(choices);
  for (size_t i = 0; i < ARRAY_SIZE(choices); ++i) {
    main_menu.items[i] =
        new_item(wchar_to_char(choices[i]), wchar_to_char(choices[i]));
  }
  /* Crate menu */
  main_menu.menu = new_menu(main_menu.items);

  /* Create the window to be associated with the menu */
  main_menu.win = newwin(12, 40, 4, 4);
  keypad(main_menu.win, TRUE);

  /* Set main window and sub window */
  set_menu_win(main_menu.menu, main_menu.win);
  set_menu_sub(main_menu.menu, derwin(main_menu.win, 8, 38, 3, 1));
  set_menu_format(main_menu.menu, 7, 1);

  /* Set menu mark to the string " * " */
  set_menu_mark(main_menu.menu, " * ");

  /* Print a border around the main window and print a title */
  box(main_menu.win, 0, 0);

  wprint_in_middle(main_menu.win, 1, 0, 40, L"Main Menu", COLOR_PAIR(1));
  mvwaddch(main_menu.win, 2, 0, ACS_LTEE);
  mvwhline(main_menu.win, 2, 1, ACS_HLINE, 38);
  mvwaddch(main_menu.win, 2, 39, ACS_RTEE);

  /* Post the menu */
  post_menu(main_menu.menu);
  wrefresh(main_menu.win);

  attron(COLOR_PAIR(2));
  mvprintw(LINES - 2, 0,
           "Use PageUp and PageDown to scoll down or up a page of items");
  mvprintw(LINES - 1, 0, "Arrow Keys to navigate (F1 to Exit)");
  attroff(COLOR_PAIR(2));
  refresh();

  int c;
  while ((c = getch()) != KEY_F(1)) {
    switch (c) {
    case KEY_DOWN:
    case KEY_NPAGE:
    case 'j':
      menu_driver(main_menu.menu, REQ_DOWN_ITEM);
      break;

    case KEY_UP:
    case KEY_PPAGE:
    case 'k':
      menu_driver(main_menu.menu, REQ_UP_ITEM);
      break;
    case 10: // ENTER
      clear();
      if (item_index(current_item(main_menu.menu)) == ARRAY_SIZE(choices) - 1) {
        goto close_menu;
      }
      choicesFuncs[item_index(current_item(main_menu.menu))]();
      current_allocated = &main_menu_allocated;
      pos_menu_cursor(main_menu.menu);
      refresh();
      wrefresh(main_menu.win);
      break;
    }
    wrefresh(main_menu.win);
  }
close_menu:

  /* Unpost and free all the memory taken up */
  unpost_menu(main_menu.menu);
  delete_all(&main_menu_allocated);
  endwin();
}
