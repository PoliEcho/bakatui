#include "timetable.h"
#include "color.h"
#include "helper_funcs.h"
#include "net.h"
#include <cstdint>
#include <curses.h>
#include <cwchar>
#include <format>
#include <fstream>
#include <iostream>
#include <locale>
#include <ncurses.h>
#include <nlohmann/json.hpp>
#include <string>

using nlohmann::json;
#define BOTTOM_PADDING 3

#define DEFAULT_OFFSET 3

const wchar_t *day_abriviations[] = {nullptr, L"Mo", L"Tu", L"We",
                                     L"Th",   L"Fr", L"Sa", L"Su"};

void draw_grid(const uint8_t num_of_columns, const uint8_t num_of_rows,
               const uint16_t cell_width, const uint16_t cell_height);

uint8_t hour_id_to_index(uint8_t *HourIdLookupTable, uint8_t id) {
  for (uint8_t i = 0; i < 10; i++) {
    if (HourIdLookupTable[i] == id) {
      return i;
    }
  }
  return 0;
}

json *partial_json_by_id(json &resp_from_api, const std::string &what,
                         const std::string &id) {
  for (uint8_t i = 0; i < resp_from_api[what].size(); i++) {
    if (resp_from_api[what][i]["Id"].get<std::string>() == id) {
      return &resp_from_api[what][i];
    }
  }
  return nullptr;
}

std::wstring get_data_for_atom(json &resp_from_api, json *atom,
                               const std::string &from_where,
                               const std::string &id_key,
                               const std::string &what) {
  return string_to_wstring(
      partial_json_by_id(resp_from_api, from_where,
                         atom->at(id_key).get<std::string>())
          ->at(what)
          .get<std::string>());
}

void timetable_page() {
  // DONT FORGET TO UNCOMMENT
  // json resp_from_api =
  // bakaapi::get_data_from_endpoint("api/3/timetable/actual");
  std::ifstream f("test-data/timetable.json");
  json resp_from_api = json::parse(f);

  // this may be unnecessary but i dont have enaugh data to test it
  // it sorts the hours by start time
  uint8_t *HourIdLookupTable = new uint8_t[resp_from_api["Hours"].size()];
  {
    using Id_and_Start_time = std::tuple<uint8_t, std::string>;
    //                                   ID,      start_time

    Id_and_Start_time *temp_hour_sorting_array =
        new Id_and_Start_time[resp_from_api["Hours"].size()];

    for (uint8_t i = 0; i < resp_from_api["Hours"].size(); i++) {
      temp_hour_sorting_array[i] = std::make_tuple(
          resp_from_api["Hours"][i]["Id"].get<uint8_t>(),
          resp_from_api["Hours"][i]["BeginTime"].get<std::string>());
    };

    std::sort(temp_hour_sorting_array,
              temp_hour_sorting_array + resp_from_api["Hours"].size(),
              [](const Id_and_Start_time &a, const Id_and_Start_time &b) {
                const std::string &str_a = std::get<1>(a);
                const std::string &str_b = std::get<1>(b);

                const size_t colon_pos_a = str_a.find(':');
                const size_t colon_pos_b = str_b.find(':');

                if (colon_pos_a == std::string::npos ||
                    colon_pos_b == std::string::npos) {
                  std::cerr << RED "[ERROR]" << RESET
                            << " Colon not found in time string\n";
                  safe_exit(EXIT_FAILURE);
                }

                const std::string hour_a_S = str_a.substr(0, colon_pos_a);
                const std::string hour_b_S = str_b.substr(0, colon_pos_b);

                const std::string minute_a_S = str_a.substr(colon_pos_a + 1);
                const std::string minute_b_S = str_b.substr(colon_pos_b + 1);

                const uint8_t hour_a = std::stoi(hour_a_S);
                const uint8_t hour_b = std::stoi(hour_b_S);

                const uint8_t minute_a = std::stoi(minute_a_S);
                const uint8_t minute_b = std::stoi(minute_b_S);

                return (hour_a < hour_b) ||
                       ((hour_a == hour_b) && (minute_a < minute_b));
              });

    for (uint8_t i = 0; i < resp_from_api["Hours"].size(); i++) {
      HourIdLookupTable[i] = std::get<0>(temp_hour_sorting_array[i]);
    }

    delete[] temp_hour_sorting_array;
  }

  for (uint8_t i = 0; i < resp_from_api["Hours"].size(); i++) {
    std::clog << (int)i << ": " << (int)HourIdLookupTable[i] << "\n";
  }

  // some lambda dark magic
  const uint8_t num_of_columns = [&]() -> uint8_t {
    uint8_t result = 0;
    for (uint8_t i = 0; i < resp_from_api["Days"].size(); i++) {
      uint8_t currentSize = resp_from_api["Days"][i]["Atoms"].size();
      if (currentSize > result) {
        result = currentSize;
      }
    }
    return result;
  }();

  const uint8_t num_of_days = resp_from_api["Days"].size();

  setlocale(LC_ALL, "");
  /* Initialize curses */
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

  const uint16_t cell_width = (COLS - BOTTOM_PADDING) / num_of_columns;
  const uint16_t cell_height = (LINES - BOTTOM_PADDING) / num_of_days;

  WINDOW **day_windows = new WINDOW *[num_of_days];
  WINDOW **lesson_windows = new WINDOW *[num_of_columns];
  std::vector<std::vector<WINDOW *>> cells(
      num_of_days, std::vector<WINDOW *>(num_of_columns));

  for (uint8_t i = 0; i < num_of_days; i++) {
    day_windows[i] = newwin(cell_height, DEFAULT_OFFSET,
                            i * cell_height + DEFAULT_OFFSET, 0);
    // this wont draw left boarder window making it so it looks partially
    // offscreen
    wborder(day_windows[i], ' ', 0, 0, 0, ACS_HLINE, 0, ACS_HLINE, 0);
    const wchar_t *day_abriv =
        day_abriviations[resp_from_api["Days"][i]["DayOfWeek"].get<uint8_t>()];

    wprint_in_middle(day_windows[i], cell_height / 2, 0, wcslen(day_abriv),
                     day_abriv, COLOR_PAIR(0));
    wrefresh(day_windows[i]);
  }

  for (uint8_t i = 0; i < num_of_columns; i++) {

    lesson_windows[i] =
        newwin(DEFAULT_OFFSET, cell_width, 0, i * cell_width + DEFAULT_OFFSET);
    wborder(lesson_windows[i], 0, 0, ' ', 0, ACS_VLINE, ACS_VLINE, 0, 0);
    std::wstring caption;
    std::wstring start_time;
    std::wstring end_time;

    for (uint8_t j = 0; j < resp_from_api["Hours"].size(); j++) {
      if (resp_from_api["Hours"][j]["Id"].get<uint8_t>() ==
          HourIdLookupTable[i]) {
        // DEBUG
        // std::clog << resp_from_api["Hours"][j]["Caption"].get<std::string>();

        std::string caption_ascii =
            resp_from_api["Hours"][j]["Caption"].get<std::string>();
        std::string start_time_ascii =
            resp_from_api["Hours"][j]["BeginTime"].get<std::string>();
        std::string end_time_ascii =
            resp_from_api["Hours"][j]["EndTime"].get<std::string>();

        caption = string_to_wstring(caption_ascii);
        start_time = string_to_wstring(start_time_ascii);
        end_time = string_to_wstring(end_time_ascii);

        goto hour_id_found;
      }
    }
    std::cerr << RED "[ERROR]" << RESET " Hour with id " << HourIdLookupTable[i]
              << " not found\n";
    goto timetable_error_exit;

  hour_id_found:

    wprint_in_middle(lesson_windows[i], 0, cell_width / 2, caption.length(),
                     caption.c_str(), COLOR_PAIR(0));
    wprint_in_middle(lesson_windows[i], 1, 1, start_time.length(),
                     start_time.c_str(), COLOR_PAIR(0));
    print_in_middle(lesson_windows[i], 1, cell_width / 2, 1, "-",
                    COLOR_PAIR(0));
    wprint_in_middle(lesson_windows[i], 1, cell_width - end_time.length() - 1,
                     end_time.length(), end_time.c_str(), COLOR_PAIR(0));
    wrefresh(lesson_windows[i]);
  }

  for (uint8_t i = 0; i < num_of_days; i++) {
    for (uint8_t j = 0; j < num_of_columns; j++) {
      cells[i][j] =
          newwin(cell_height, cell_width, i * cell_height + DEFAULT_OFFSET,
                 j * cell_width + DEFAULT_OFFSET);
      box(cells[i][j], 0, 0);
      json *atom;
      for (uint8_t k = 0; k < resp_from_api["Days"][i]["Atoms"].size(); k++) {
        if (resp_from_api["Days"][i]["Atoms"][k]["HourId"].get<uint8_t>() ==
            HourIdLookupTable[j]) {
          atom = &resp_from_api["Days"][i]["Atoms"][k];
          goto correct_atom_found;
        }
      }
      continue;
    correct_atom_found:
      try {
        if (atom->contains("Change")) {
        }
        std::wstring Subject_Abbrev = get_data_for_atom(
            resp_from_api, atom, "Subjects", "SubjectId", "Abbrev");
        wprint_in_middle(cells[i][j], cell_height / 2,
                         cell_width / 2 - wcslen(Subject_Abbrev.c_str()) / 2,
                         wcslen(Subject_Abbrev.c_str()), Subject_Abbrev.c_str(),
                         COLOR_PAIR(0));

        std::wstring Room_Abbrev =
            get_data_for_atom(resp_from_api, atom, "Rooms", "RoomId", "Abbrev");
        wprint_in_middle(cells[i][j], cell_height - 2,
                         cell_width - wcslen(Room_Abbrev.c_str()) - 1,
                         wcslen(Room_Abbrev.c_str()), Room_Abbrev.c_str(),
                         COLOR_PAIR(0));

        std::wstring Teacher_Abbrev = get_data_for_atom(
            resp_from_api, atom, "Teachers", "TeacherId", "Abbrev");
        wprint_in_middle(cells[i][j], cell_height - 2, 1,
                         wcslen(Teacher_Abbrev.c_str()), Teacher_Abbrev.c_str(),
                         COLOR_PAIR(0));
        wrefresh(cells[i][j]);
      } catch (...) {
        // world's best error handling
        __asm__("nop");
      }
    }
  }
  refresh();
  getch();
timetable_error_exit:
  delete[] HourIdLookupTable;
  delete[] day_windows;
  delete[] lesson_windows;
  endwin();
}