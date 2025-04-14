#include "memory.h"
#include "color.h"
#include "types.h"
#include <curses.h>
#include <iostream>
#include <menu.h>
#include <ncurses.h>
#include <panel.h>

template <typename T> struct NcursesDeleter {
  static void delete_element(T obj);
};

template <> void NcursesDeleter<WINDOW *>::delete_element(WINDOW *win) {
  delwin(win);
}
template <> void NcursesDeleter<PANEL *>::delete_element(PANEL *pan) {
  del_panel(pan);
}
template <> void NcursesDeleter<ITEM *>::delete_element(ITEM *item) {
  free_item(item);
}

template <typename T> void delete_ncurses_arrays(void *ptr, std::size_t size) {
  T *array = static_cast<T *>(ptr);
  for (std::size_t j = 0; j < size; ++j) {
    NcursesDeleter<T>::delete_element(array[j]);
  }
  delete[] array;
}

void delete_all(std::vector<allocation> *allocated) {
  if (allocated == nullptr) {
    return;
  }
  for (long long i = allocated->size() - 1; i >= 0; i--) {
    switch (allocated->at(i).type) {
    case WINDOW_ARRAY: {
      delete_ncurses_arrays<WINDOW *>(allocated->at(i).ptr,
                                      allocated->at(i).size);
      break;
    }
    case PANEL_ARRAY: {
      delete_ncurses_arrays<PANEL *>(allocated->at(i).ptr,
                                     allocated->at(i).size);
      break;
    }
    case ITEM_ARRAY: {
      delete_ncurses_arrays<ITEM *>(allocated->at(i).ptr,
                                    allocated->at(i).size);
      break;
    }
    case CHAR_PTR_ARRAY: {
      char **array = static_cast<char **>(allocated->at(i).ptr);
      for (std::size_t j = 0; j < allocated->at(i).size; ++j) {
        delete[] array[j];
      }
      delete[] array;
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
    case COMPLETE_MENU_TYPE: {
      free_menu(static_cast<complete_menu *>(allocated->at(i).ptr)->menu);
      delwin(static_cast<complete_menu *>(allocated->at(i).ptr)->win);
      delete_ncurses_arrays<ITEM *>(
          static_cast<complete_menu *>(allocated->at(i).ptr)->items,
          static_cast<complete_menu *>(allocated->at(i).ptr)->items_size);

      break;
    }
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
