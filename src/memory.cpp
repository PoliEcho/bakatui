#include "memory.h"
#include "color.h"
#include <iostream>
#include <ncurses.h>
#include <panel.h>

void delete_all(std::vector<allocation> *allocated) {
  if (allocated == nullptr) {
    return;
  }
  for (long long i = allocated->size() - 1; i >= 0; i--) {
    switch (allocated->at(i).type) {
    case WINDOW_ARRAY: {
      WINDOW **windows = static_cast<WINDOW **>(allocated->at(i).ptr);
      for (std::size_t j = 0; j < allocated->at(i).size; j++) {
        delwin(windows[j]);
      }
      delete[] windows;
      break;
    }
    case PANEL_ARRAY: {
      PANEL **panels = static_cast<PANEL **>(allocated->at(i).ptr);
      for (std::size_t j = 0; j < allocated->at(i).size; j++) {
        del_panel(panels[j]);
      }
      delete[] panels;
      break;
    }
    case ITEM_ARRAY: {
      ITEM **items = static_cast<ITEM **>(allocated->at(i).ptr);
      for (std::size_t j = 0; j < allocated->at(i).size; j++) {
        free_item(items[j]);
      }
      delete[] items;
      break;
    }
    case GENERIC_ARRAY:
      delete[] static_cast<char *>(allocated->at(i).ptr);
      break;
    case WINDOW_TYPE:
      delwin(static_cast<WINDOW *>(allocated->at(i).ptr));
      break;
    case PANEL_TYPE:
      del_panel(static_cast<PANEL *>(allocated->at(i).ptr));
      break;
    case MENU_TYPE:
      free_menu(static_cast<MENU *>(allocated->at(i).ptr));
      break;
    case GENERIC_TYPE:
      delete static_cast<char *>(allocated->at(i).ptr);
      break;
    default:
      std::cerr << RED "[!!CRITICAL!!]" << RESET " Unknown allocation type"
                << "\n";
      break;
    }
    allocated->pop_back();
  }
}
