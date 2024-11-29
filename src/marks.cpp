#include "marks.h"
#include "helper_funcs.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <curses.h>
#include <fstream>
#include <menu.h>
#include <nlohmann/json.hpp>
#include <panel.h>

using nlohmann::json;

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

void init_wins(WINDOW **wins, int n, json marks_json);
void win_show(WINDOW *win, char *label, int label_color);

void marks_page() {

  // DONT FORGET TO UNCOMMENT
  // json resp_from_api = bakaapi::get_grades();
  std::ifstream f("test-data/marks2.json");
  json resp_from_api = json::parse(f);

  WINDOW **my_wins;
  size_t size_my_wins = resp_from_api["Subjects"].size();
  my_wins = new (std::nothrow) WINDOW *[size_my_wins];

  PANEL **my_panels;
  size_t size_my_panels = resp_from_api["Subjects"].size();
  my_panels = new (std::nothrow) PANEL *[size_my_panels];

  PANEL *top;
  int ch;

  setlocale(LC_ALL, "");
  /* Initialize curses */
  initscr();
  start_color();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);

  /* Initialize all the colors */
  for (size_t i = 0; i < 8; i++) {
    init_pair(i, i, COLOR_BLACK);
  }

  init_wins(my_wins, resp_from_api["Subjects"].size(), resp_from_api);

  for (size_t i = 0; i < size_my_panels; i++) {
    /* Attach a panel to each window Order is bottom up */
    my_panels[i] = new_panel(my_wins[i]);

    /* Set up the user pointers to the next panel */
    if ((i + 1) < size_my_panels) {
      set_panel_userptr(my_panels[i], my_panels[(i + 1)]);
    } else {
      set_panel_userptr(my_panels[i], my_panels[0]);
    }
  }

  // Update the stacking order.
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
void init_wins(WINDOW **wins, int n, json marks_json) {
  int x, y, i;
  char label[1500];

  y = 2;
  x = 10;
  uint8_t curent_color = 0;
  for (i = 0; i < n; ++i) {
    wins[i] = newwin(NLINES, NCOLS, y, x);
    {
      std::string sub_name = marks_json["Subjects"][i]["Subject"]["Name"];
      std::string sub_avg_s = marks_json["Subjects"][i]["AverageText"];

      sprintf(label, "%s - avg: %s", sub_name.c_str(), sub_avg_s.c_str());
    }
    curent_color++;
    if (curent_color >= 7) {
      curent_color = 1;
    }
    win_show(wins[i], label, curent_color);
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