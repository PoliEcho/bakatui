#include "memory.h"
#include <curses.h>
#include <string>
#include <vector>

// header guard
#ifndef _ba_hf_hg_
#define _ba_hf_hg_

extern std::vector<allocation> *current_allocated;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

void safe_exit(int code);
std::string bool_to_string(bool bool_in);
std::string SoRAuthFile(bool save, std::string data);
void get_input_and_login();

// Original functions
void print_in_middle(WINDOW *win, int starty, int startx, int width,
                     const char *string, chtype color);
std::string rm_tr_le_whitespace(const std::string &s);

// Wide character support functions
void wprint_in_middle(WINDOW *win, int starty, int startx, int width,
                      const wchar_t *string, chtype color);
std::wstring wrm_tr_le_whitespace(const std::wstring &s);

// Conversion utilities
char *wchar_to_char(const wchar_t *src);
wchar_t *char_to_wchar(const char *src);

std::wstring string_to_wstring(const std::string &str);
std::string wstring_to_string(const std::wstring &wstr);

std::string html_to_string(std::string html);

#endif