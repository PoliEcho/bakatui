// header guard
#ifndef _ba_ty_hg_
#define _ba_ty_hg_

#include <istream>
#include <menu.h>
struct Config {
  bool verbose = false;
  bool ignoressl = false;
};

class LimitedInt {
private:
  int value;
  const int min_value;
  const int max_value;

  void setValue(int new_value) {
    int range = max_value - min_value + 1;
    if (new_value < min_value || new_value > max_value) {
      new_value = ((new_value - min_value) % range + range) % range + min_value;
    }
    value = new_value;
  }

public:
  // Constructors
  LimitedInt(int initial_value = 0, int min = std::numeric_limits<int>::min(),
             int max = std::numeric_limits<int>::max())
      : min_value(min), max_value(max) {
    if (min >= max)
      throw std::invalid_argument("Min must be less than max");
    setValue(initial_value);
  }

  // Conversion operators
  operator int() const { return value; } // Implicit conversion to int

  // Assignment operators
  LimitedInt &operator=(int rhs) {
    setValue(rhs);
    return *this;
  }

  // Compound assignment with BoundedInt
  LimitedInt &operator+=(const LimitedInt &rhs) {
    setValue(value + rhs.value);
    return *this;
  }

  // Compound assignment with int
  LimitedInt &operator+=(int rhs) {
    setValue(value + rhs);
    return *this;
  }

  // Similarly define -=, *=, /=, %=, &=, |=, ^=, <<=, >>= for both types

  // Increment/decrement
  LimitedInt &operator++() { // Prefix ++
    setValue(value + 1);
    return *this;
  }

  LimitedInt operator++(int) { // Postfix ++
    LimitedInt temp = *this;
    setValue(value + 1);
    return temp;
  }

  LimitedInt &operator--() { // Prefix --
    setValue(value - 1);
    return *this;
  }

  LimitedInt operator--(int) { // Postfix --
    LimitedInt temp = *this;
    setValue(value - 1);
    return temp;
  }

  // Binary arithmetic operators
  friend LimitedInt operator+(const LimitedInt &lhs, const LimitedInt &rhs) {
    LimitedInt result = lhs;
    result += rhs;
    return result;
  }

  friend LimitedInt operator+(const LimitedInt &lhs, int rhs) {
    LimitedInt result = lhs;
    result += rhs;
    return result;
  }

  friend LimitedInt operator+(int lhs, const LimitedInt &rhs) {
    LimitedInt result(lhs, rhs.min_value, rhs.max_value);
    result += rhs;
    return result;
  }

  // Similarly define -, *, /, %, &, |, ^, <<, >> for all combinations

  // Unary operators
  LimitedInt operator-() const {
    return LimitedInt(-value, min_value, max_value);
  }

  LimitedInt operator+() const { return *this; }

  // Comparison operators
  friend bool operator==(const LimitedInt &lhs, const LimitedInt &rhs) {
    return lhs.value == rhs.value;
  }

  friend bool operator!=(const LimitedInt &lhs, const LimitedInt &rhs) {
    return lhs.value != rhs.value;
  }

  friend bool operator<(const LimitedInt &lhs, const LimitedInt &rhs) {
    return lhs.value < rhs.value;
  }

  // Similarly define >, <=, >= for both BoundedInt and int comparisons

  // Stream operators
  friend std::ostream &operator<<(std::ostream &os, const LimitedInt &bi) {
    os << bi.value;
    return os;
  }

  friend std::istream &operator>>(std::istream &is, LimitedInt &bi) {
    int temp;
    is >> temp;
    bi.setValue(temp);
    return is;
  }
};

struct SelectorType {
  LimitedInt x;
  LimitedInt y;

  SelectorType(int xArg, int yArg, int min_limit_x, int max_limit_x,
               int min_limit_y, int max_limit_y)
      : x(xArg, min_limit_x, max_limit_x), y(yArg, min_limit_y, max_limit_y) {}
};

struct complete_menu {
  WINDOW *win;
  ITEM **items;
  MENU *menu;
};

#endif