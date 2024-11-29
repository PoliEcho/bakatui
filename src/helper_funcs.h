#include <curses.h>
#include <string>
void safe_exit(int code);
std::string bool_to_string(bool bool_in);
std::string SoRAuthFile(bool save, std::string data);
void get_input_and_login();
void print_in_middle(WINDOW *win, int starty, int startx, int width,
                     char *string, chtype color);