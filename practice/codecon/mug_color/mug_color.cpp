#include <string>

#include <iostream>

#include "mug_color.h"

using namespace std;

void Mug::setColor(string color) {
  setColorNum(colorNum(color));
}

string Mug::getColor() const {
  return colorName(getColorNum());
}

void Mug::setColorNum(Mug::Color color) {
  char x = 1;
  for (int c = WHITE; c < color; ++c) {
    x = x << 1;
  }
  m_colorSet |= x;
}

Mug::Color Mug::getColorNum() const {
  char color = m_colorSet;
  for (int c = WHITE; c < MAX_COLOR; ++c) {
    if (!(color & 1)) {
      return static_cast<Mug::Color>(c);
    }
    color = color >> 1;
  }
	return MAX_COLOR;
}

string Mug::colorName(const Mug::Color color) {
  switch (color) {
    case WHITE:
      return "White";
    case BLACK:
      return "Black";
    case BLUE:
      return "Blue";
    case RED:
      return "Red";
    case YELLOW:
      return "Yellow";
    default:
      return "";
  }
}

Mug::Color Mug::colorNum(const string color) {
  if (color == "White")
    return WHITE;
  else if (color == "Black")
    return BLACK;
  else if (color == "Blue")
    return BLUE;
  else if (color == "RED")
    return RED;
  else if (color == "Yellow")
    return YELLOW;
  else
    return MAX_COLOR;
}
