#include "marks.h"
#include <cstring>
#include <curses.h>
#include <menu.h>
#include <panel.h>

// Thsi code is based on
// https://github.com/tony/NCURSES-Programming-HOWTO-examples/blob/master/16-panels
/*
The MIT License (MIT)

Copyright (c) 2016 Tony Narlock

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#define NLINES 10
#define NCOLS 40

void init_wins(WINDOW **wins, int n);
void win_show(WINDOW *win, char *label, int label_color);
void print_in_middle(WINDOW *win, int starty, int startx, int width,
                     char *string, chtype color);

void marks_page() {
  WINDOW *my_wins[3];
  PANEL *my_panels[3];
  PANEL *top;
  int ch;

  /* Initialize curses */
  initscr();
  start_color();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  /* Initialize all the colors */
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_BLUE, COLOR_BLACK);
  init_pair(4, COLOR_CYAN, COLOR_BLACK);

  init_wins(my_wins, 3);

  /* Attach a panel to each window */   /* Order is bottom up */
  my_panels[0] = new_panel(my_wins[0]); /* Push 0, order: stdscr-0 */
  my_panels[1] = new_panel(my_wins[1]); /* Push 1, order: stdscr-0-1 */
  my_panels[2] = new_panel(my_wins[2]); /* Push 2, order: stdscr-0-1-2 */

  /* Set up the user pointers to the next panel */
  set_panel_userptr(my_panels[0], my_panels[1]);
  set_panel_userptr(my_panels[1], my_panels[2]);
  set_panel_userptr(my_panels[2], my_panels[0]);

  /* Update the stacking order. 2nd panel will be on top */
  update_panels();

  /* Show it on the screen */
  attron(COLOR_PAIR(4));
  mvprintw(LINES - 2, 0, "Use tab to browse through the windows (F1 to Exit)");
  attroff(COLOR_PAIR(4));
  doupdate();

  top = my_panels[2];
  while ((ch = getch()) != KEY_F(1)) {
    switch (ch) {
    case 9:
      top = (PANEL *)panel_userptr(top);
      top_panel(top);
      break;
    }
    update_panels();
    doupdate();
  }
  endwin();
}

/* Put all the windows */
void init_wins(WINDOW **wins, int n) {
  int x, y, i;
  char label[80];

  y = 2;
  x = 10;
  for (i = 0; i < n; ++i) {
    wins[i] = newwin(NLINES, NCOLS, y, x);
    sprintf(label, "Window Number %d", i + 1);
    win_show(wins[i], label, i + 1);
    x += 40;
  }
}

/* Show the window with a border and a label */
void win_show(WINDOW *win, char *label, int label_color) {
  int startx, starty, height, width;

  getbegyx(win, starty, startx);
  getmaxyx(win, height, width);

  box(win, 0, 0);
  mvwaddch(win, 2, 0, ACS_LTEE);
  mvwhline(win, 2, 1, ACS_HLINE, width - 2);
  mvwaddch(win, 2, width - 1, ACS_RTEE);

  print_in_middle(win, 1, 0, width, label, COLOR_PAIR(label_color));
}