// header guard
#ifndef _ba_me_hg_
#define _ba_me_hg_

#include <cstddef>
#include <menu.h>
#include <vector>
enum AllocationType {
  WINDOW_ARRAY,
  PANEL_ARRAY,
  ITEM_ARRAY,
  CHAR_PTR_ARRAY,
  GENERIC_ARRAY,
  WINDOW_TYPE,
  PANEL_TYPE,
  MENU_TYPE,
  COMPLETE_MENU_TYPE,
  GENERIC_TYPE
};

struct allocation {
  AllocationType type;
  void *ptr;
  std::size_t size;
};

void delete_all(std::vector<allocation> *allocated);
#endif
