#include "memory.h"
#include "helper_funcs.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <curses.h>
#include <nlohmann/json.hpp>
#include "net.h"
#include <nlohmann/json_fwd.hpp>
#include <ncurses.h>
#include <string>

using nlohmann::json;

std::vector<allocation> absence_allocated;
#define DATE_LEN 10//DD.MM.YYYY
#define SUM_FIELD_NUM 7
#define NUM_PRESSISON 3
#define FLOAT_PRESSISON 6
#define BASE_ABSENCE_WIN_SIZE 1+NUM_PRESSISON+1+NUM_PRESSISON+1+FLOAT_PRESSISON+1+1
#define ABSENCE_WIN_HIGHT 5
constexpr char sum_field_names[SUM_FIELD_NUM] = {'/', 'X','N','P','O', '-', 'D'};
constexpr chtype sum_field_colors[SUM_FIELD_NUM] = {
    COLOR_PAIR(COLOR_RED),
    COLOR_PAIR(COLOR_GREEN),
    COLOR_PAIR(COLOR_RED),
    COLOR_PAIR(COLOR_YELLOW),
    COLOR_PAIR(COLOR_MAGENTA),
    COLOR_PAIR(COLOR_GREEN),
    COLOR_PAIR(COLOR_CYAN)
};

constexpr char* sum_field_strings[SUM_FIELD_NUM] = {"Unsolved","Ok","Missed","Late","Soon","School","DistanceTeaching"};
    
    
constexpr char date_str[] = "Date";

void absence_page() {
    current_allocated = &absence_allocated;
    const json resp_from_api = [&]() -> json {
    const std::string endpoint = "api/3/absence/student";
    return bakaapi::get_data_from_endpoint(endpoint, GET);
  }();
  
    /* Initialize curses */
  setlocale(LC_ALL, "");
  initscr();
  start_color();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);

  /* Initialize all the colors */
  for (uint8_t i = 0; i < 8; i++) {
    init_pair(i, i, COLOR_BLACK);
  }

  const uint16_t sum_win_height = (resp_from_api["Absences"].size()*2)+1;
  const uint16_t sum_win_width = 1+DATE_LEN+1+(SUM_FIELD_NUM*4);
  const float absence_threshold = resp_from_api["PercentageThreshold"].get<float>()*100.0f;
  {
  WINDOW*sum_win = newwin(sum_win_height,sum_win_width,0,0);
  
    absence_allocated.push_back({WINDOW_TYPE, sum_win, 1});
    box(sum_win, 0, 0);
    print_in_middle(sum_win, 1, 1, DATE_LEN, date_str, COLOR_PAIR(COLOR_RED));
    for (uint16_t i = 0; i < SUM_FIELD_NUM; i++) {
        mvwaddch(sum_win, 0, 1+DATE_LEN+(i*4), ACS_TTEE);
        mvwaddch(sum_win, 1, 1+DATE_LEN+(i*4), ACS_VLINE);
        mvwaddch(sum_win, sum_win_height-1, 1+DATE_LEN+(i*4), ACS_BTEE);
        wattron(sum_win, sum_field_colors[i]);
        mvwaddch(sum_win, 1, 1+DATE_LEN+2+(i*4),sum_field_names[i]);   
        wattroff(sum_win, sum_field_colors[i]);
    }
     
    for (size_t i = 0; resp_from_api["Absences"].size()-1 > i; i++) {
        mvwaddch(sum_win, 2+(i*2), 0, ACS_LTEE);
        mvwhline(sum_win, 2+(i*2), 1, ACS_HLINE, DATE_LEN);
        mvwaddch(sum_win, 2+(i*2), 1+DATE_LEN, ACS_PLUS);
        for(size_t j=0;j<SUM_FIELD_NUM;j++) {
            mvwhline(sum_win, 2+(i*2),1+DATE_LEN+1+(j*4), ACS_HLINE,3);
            mvwaddch(sum_win, 2+(i*2),1+DATE_LEN+4+(j*4), ACS_PLUS);
            mvwaddch(sum_win, 3+(i*2),1+DATE_LEN+(j*4), ACS_VLINE);
            mvwprintw(sum_win,3+(i*2),1+DATE_LEN+1+(j*4),"%3d",resp_from_api["Absences"][i][sum_field_strings[j]].get<int>());
        }
        mvwaddch(sum_win, 2+(i*2), sum_win_width-1, ACS_RTEE);
        const std::string date_str = resp_from_api["Absences"][i]["Date"].get<std::string>();
        const char* date_cstr = date_str.c_str();
        mvwprintw(sum_win, 2+(i*2)+1, 1, "%.2s.%.2s.%.4s", date_cstr+8,date_cstr+5,date_cstr);
    }
    wrefresh(sum_win);
}
    
    WINDOW** subject_wins = new WINDOW* [resp_from_api["AbsencesPerSubject"].size()];
    absence_allocated.push_back({WINDOW_ARRAY, subject_wins, resp_from_api["AbsencesPerSubject"].size()});

    uint16_t window_y_offset = 0;
    uint16_t window_x_offset = sum_win_width+1;
    for (size_t i = 0;resp_from_api["AbsencesPerSubject"].size() > i; i++) {
      const uint16_t subject_name_lenght = string_to_wstring(resp_from_api["AbsencesPerSubject"][i]["SubjectName"].get<std::string>()).length();
      const uint16_t subject_win_width = 2 + subject_name_lenght > BASE_ABSENCE_WIN_SIZE ? subject_name_lenght+2 : BASE_ABSENCE_WIN_SIZE;
      if (window_x_offset + subject_win_width > COLS) {
          window_x_offset = sum_win_width+1;
          window_y_offset += ABSENCE_WIN_HIGHT;
      }
      subject_wins[i] = newwin(ABSENCE_WIN_HIGHT,subject_win_width, window_y_offset, window_x_offset);
      box(subject_wins[i],0,0);
      window_x_offset +=subject_win_width;

      const uint16_t absence_per_subject_base = resp_from_api["AbsencesPerSubject"][i]["Base"].get<uint16_t>();
      const uint16_t absence_per_subject_lessons_count = resp_from_api["AbsencesPerSubject"][i]["LessonsCount"].get<uint16_t>();
      const float absence_per_subject_percentage = ((resp_from_api["AbsencesPerSubject"][i]["Base"].get<float>()/resp_from_api["AbsencesPerSubject"][i]["LessonsCount"].get<float>())*100.0);

      
      mvwaddch(subject_wins[i],2,0,ACS_LTEE);
      mvwhline(subject_wins[i],2,1,ACS_HLINE,subject_win_width-2);
      mvwaddch(subject_wins[i],2,subject_win_width-1,ACS_RTEE);
      mvwaddch(subject_wins[i], 2, 1+NUM_PRESSISON, ACS_TTEE);
      mvwaddch(subject_wins[i], 3, 1+NUM_PRESSISON, ACS_VLINE);
      mvwaddch(subject_wins[i], 4, 1+NUM_PRESSISON, ACS_BTEE);
      mvwaddch(subject_wins[i], 2, 1+(NUM_PRESSISON*2)+1, ACS_TTEE);
      mvwaddch(subject_wins[i], 3, 1+(NUM_PRESSISON*2)+1, ACS_VLINE);
      mvwaddch(subject_wins[i], 4, 1+(NUM_PRESSISON*2)+1, ACS_BTEE);
      if (subject_win_width != BASE_ABSENCE_WIN_SIZE) {
          mvwaddch(subject_wins[i], 2, subject_win_width-FLOAT_PRESSISON-3, ACS_TTEE);
          mvwaddch(subject_wins[i], 3, subject_win_width-FLOAT_PRESSISON-3, ACS_VLINE);
          mvwaddch(subject_wins[i], 4, subject_win_width-FLOAT_PRESSISON-3, ACS_BTEE);
      }
      mvwprintw(subject_wins[i], 3, 1, "%3d",absence_per_subject_base);
      mvwprintw(subject_wins[i], 3, 1+NUM_PRESSISON+1, "%3d",absence_per_subject_lessons_count);

      chtype text_color;
      if (absence_threshold <= absence_per_subject_percentage) {
          text_color = COLOR_PAIR(COLOR_RED);
      } else if (absence_threshold*0.6f <= absence_per_subject_percentage) {
          text_color = COLOR_PAIR(COLOR_YELLOW);
      } else {
          text_color = COLOR_PAIR(COLOR_GREEN);
      }
      
      wprint_in_middle(subject_wins[i],1, 1, subject_win_width-1, string_to_wstring(resp_from_api["AbsencesPerSubject"][i]["SubjectName"].get<std::string>()).c_str() , text_color);
      wattron(subject_wins[i], text_color);
      mvwprintw(subject_wins[i], 3, subject_win_width-FLOAT_PRESSISON-1-1, "%6.2f%%",absence_per_subject_percentage);
      wattroff(subject_wins[i], text_color);

      wrefresh(subject_wins[i]);
    }
    attron(COLOR_PAIR(4));
    mvprintw(LINES-1, 0, "F1 to exit | Absence threshold: %.2f%% ", absence_threshold);
    attroff(COLOR_PAIR(4));
    for (uint8_t i = 0; i < SUM_FIELD_NUM; i++) {
        attron(COLOR_PAIR(4));
        addch('|');
        attroff(COLOR_PAIR(4));
        wattron(stdscr, sum_field_colors[i]);
        printw( " %c:%s ", sum_field_names[i], sum_field_strings[i]);
        wattroff(stdscr, sum_field_colors[i]);
    }

    
    refresh();
    while (getch() != KEY_F(1));
    
    
  delete_all(&absence_allocated);
  clear();
}