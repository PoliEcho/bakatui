#include "komens_menu.h"
#include "helper_funcs.h"
#include "komens.h"
#include "net.h"
#include <cstdlib>
#include <cstring>
#include <curses.h>
#include <menu.h>

void komens_menu() {
  wchar_t *choices[] = {
      L"received", L"send", L"noticeboard", L"Exit", nullptr,
  };
  void (*choicesFuncs[])() = {komens_page, nullptr, nullptr, nullptr, nullptr};

  ITEM **my_items;
  int c;
  MENU *my_menu;
  WINDOW *my_menu_win;
  int n_choices, i;

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
  my_items = (ITEM **)calloc(n_choices, sizeof(ITEM *));
  for (i = 0; i < n_choices; ++i)
    my_items[i] =
        new_item(wchar_to_char(choices[i]), wchar_to_char(choices[i]));

  /* Crate menu */
  my_menu = new_menu((ITEM **)my_items);

  /* Create the window to be associated with the menu */
  my_menu_win = newwin(12, 40, 4, 4);
  keypad(my_menu_win, TRUE);

  /* Set main window and sub window */
  set_menu_win(my_menu, my_menu_win);
  set_menu_sub(my_menu, derwin(my_menu_win, 8, 38, 3, 1));
  set_menu_format(my_menu, 7, 1);

  /* Set menu mark to the string " * " */
  set_menu_mark(my_menu, " * ");

  /* Print a border around the main window and print a title */
  box(my_menu_win, 0, 0);

  wprint_in_middle(my_menu_win, 1, 0, 40, L"Komens Menu", COLOR_PAIR(1));
  mvwaddch(my_menu_win, 2, 0, ACS_LTEE);
  mvwhline(my_menu_win, 2, 1, ACS_HLINE, 38);
  mvwaddch(my_menu_win, 2, 39, ACS_RTEE);

  /* Post the menu */
  post_menu(my_menu);
  wrefresh(my_menu_win);

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
      menu_driver(my_menu, REQ_DOWN_ITEM);
      break;

    case KEY_UP:
    case KEY_PPAGE:
    case 'k':
      menu_driver(my_menu, REQ_UP_ITEM);
      break;
    case 10: // ENTER
      clear();
      if (item_index(current_item(my_menu)) == n_choices - 1) {
        goto close_menu;
      }
      choicesFuncs[item_index(current_item(my_menu))]();
      pos_menu_cursor(my_menu);
      refresh();
      wrefresh(my_menu_win);
      break;
    }
    wrefresh(my_menu_win);
  }
close_menu:

  /* Unpost and free all the memory taken up */
  unpost_menu(my_menu);
  free_menu(my_menu);
  for (i = 0; i < n_choices; ++i)
    free_item(my_items[i]);
  endwin();
}
