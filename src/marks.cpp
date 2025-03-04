#include "marks.h"
#include "helper_funcs.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <curses.h>
#include <format>
#include <fstream>
#include <iostream>
#include <menu.h>
#include <nlohmann/json.hpp>
#include <panel.h>
#include <string>
#include <algorithm>
#include "net.h"
#include "color.h"

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

#define DEFAULT_X_OFFSET 10
#define DEFAULT_Y_OFFSET 2

#define DEFAULT_PADDING 4

void init_wins(WINDOW **wins, int n, json marks_json);
void win_show(WINDOW *win, char *label, int label_color, int width, int height,
              json marks_json, int SubjectIndex);

void marks_page() {
  // DONT FORGET TO UNCOMMENT
  json resp_from_api = bakaapi::get_grades();
  // std::ifstream f("test-data/marks3.json");
  // json resp_from_api = json::parse(f);

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

  std::clog << COLS << " " << LINES << std::endl;

  /* Initialize all the colors */
  for (uint8_t i = 0; i < 8; i++) {
    init_pair(i, i, COLOR_BLACK);
  }

  init_wins(my_wins, resp_from_api["Subjects"].size(), resp_from_api);

  // store all original window position
  int *original_y = new int[size_my_wins];
  int *original_x = new int[size_my_wins];
  for (size_t i = 0; i < size_my_wins; ++i) {
    getbegyx(my_wins[i], original_y[i], original_x[i]);
  }

  // Attach panels
  for (size_t i = 0; i < size_my_panels; i++) {
    my_panels[i] = new_panel(my_wins[i]);
    set_panel_userptr(my_panels[i], 
      (i + 1 < size_my_panels) ? my_panels[i+1] : my_panels[0]);
  }

  update_panels();
  attron(COLOR_PAIR(4));
  mvprintw(LINES - 2, 0, "Arrows/j/k to scroll | F1 to exit");
  attroff(COLOR_PAIR(4));
  doupdate();

  top = my_panels[size_my_panels - 1];
  long y_offset = 0;

  // Main loop
  while ((ch = getch()) != KEY_F(1)) {
    bool needs_update = false;
    
    switch (ch) {
      case KEY_UP:
      case 'k': // Vim-style up
        y_offset--;
        needs_update = true;
        break;

      case KEY_DOWN:
      case 'j': // Vim-style down
        y_offset++;
        needs_update = true;
        break;
    }

    // Update window positions if scrolled
    if (needs_update) {
      for (size_t i = 0; i < size_my_panels; ++i) {
        int new_y = original_y[i] - y_offset;
        int new_x = original_x[i];
        move_panel(my_panels[i], new_y, new_x);
      }
      
      update_panels();
      doupdate();
    }
  }

  // Cleanup
  endwin();
  clear();
  delete[] my_wins;
  delete[] my_panels;
  delete[] original_y;
  delete[] original_x;
}

/* Put all the windows */
void init_wins(WINDOW **wins, int n, json marks_json) {
  int x, y, i;
  char label[1500];

  y = DEFAULT_Y_OFFSET;
  x = DEFAULT_X_OFFSET;
  uint8_t curent_color = 0;

  int MaxHight = 0;
  // this loop true subjects
  for (i = 0; i < n; ++i) {

    // Calculate label and max_text_length to determine window width
    std::string sub_name = marks_json["Subjects"][i]["Subject"]["Name"];
    // DEBUG
    // std::clog << BLUE"[LOG]" << RESET" procesing subject: " << sub_name << "\n";
    std::string sub_avg_s = marks_json["Subjects"][i]["AverageText"];
    sprintf(label, "%s - avg: %s", sub_name.c_str(), sub_avg_s.c_str());

    size_t max_text_length = strlen(label);
    for (int j = 0; j < marks_json["Subjects"][i]["Marks"].size(); j++) {
      std::string caption = rm_tr_le_whitespace(marks_json["Subjects"][i]["Marks"][j]["Caption"]);
      std::string theme = rm_tr_le_whitespace(marks_json["Subjects"][i]["Marks"][j]["Theme"]);
      caption = rm_tr_le_whitespace(caption);
      theme = rm_tr_le_whitespace(theme);
      max_text_length =
          std::max({max_text_length, caption.length(), theme.length()});
    }

    int width = max_text_length + DEFAULT_PADDING;

    // hanndle windows overflowing off screen
    if (x + width > COLS) {
      x = DEFAULT_X_OFFSET;
      y += MaxHight + 2;
      MaxHight = 0;
    }

    if (marks_json["Subjects"][i]["Marks"].size() * 2 + DEFAULT_PADDING >
        MaxHight) {
      MaxHight =
          marks_json["Subjects"][i]["Marks"].size() * 2 + DEFAULT_PADDING;
    }

    wins[i] = newwin(NLINES, NCOLS, y, x);
    win_show(wins[i], label, curent_color + 1, width,
             marks_json["Subjects"][i]["Marks"].size() * 2 + DEFAULT_PADDING,
             marks_json, i);

    curent_color = (curent_color + 1) % 7;
    x += width + 5;
  }
}

/* Show the window with a border and a label */
void win_show(WINDOW *win, char *label, int label_color, int width, int height,
              json marks_json, int SubjectIndex) {
  int startx, starty;

  wresize(win, height, width);

  getbegyx(win, starty, startx);
  getmaxyx(win, height, width);

  box(win, 0, 0);
  mvwaddch(win, 2, 0, ACS_LTEE);
  mvwhline(win, 2, 1, ACS_HLINE, width - 2);
  mvwaddch(win, 2, width - 1, ACS_RTEE);

  print_in_middle(win, 1, 0, width, label, COLOR_PAIR(label_color));

  char CaptionBuf[1500];
  char ThemeBuf[1500];
  std::string Caption;
  int AdditionalOffset = 0;
  for (size_t i = 0; i < marks_json["Subjects"][SubjectIndex]["Marks"].size();
       i++) {
    Caption = marks_json["Subjects"][SubjectIndex]["Marks"][i]["Caption"];
    Caption = rm_tr_le_whitespace(Caption);
	  Caption.append(std::format(" - {{{}}} [{}]",marks_json["Subjects"][SubjectIndex]["Marks"][i]["MarkText"].get<std::string>(),marks_json["Subjects"][SubjectIndex]["Marks"][i]["Weight"].get<int>()));
    strncpy(CaptionBuf, Caption.c_str(), sizeof(CaptionBuf)-1);
    print_in_middle(win, 3 + i + AdditionalOffset, 0, width, CaptionBuf,COLOR_PAIR(label_color));

    strncpy(ThemeBuf,
            rm_tr_le_whitespace(marks_json["Subjects"][SubjectIndex]["Marks"][i]["Theme"]
                .get<std::string>())
                .c_str(),
            sizeof(ThemeBuf)-1);
    print_in_middle(win, 3 + i + 1 + AdditionalOffset, 0, width, ThemeBuf,
                    COLOR_PAIR(label_color));
    AdditionalOffset++;
  }
}