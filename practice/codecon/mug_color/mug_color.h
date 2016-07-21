#ifndef __MUG_COLOR_H__
#define __MUG_COLOR_H__

#include <string>

class Mug {
  public:
  enum Color {
    WHITE, BLACK, BLUE, RED , YELLOW, MAX_COLOR
  };
  
  Mug() : m_colorSet(0) {};
  
  void setColor(const std::string color);
  std::string getColor() const;
  
  private:
  char m_colorSet;

  void setColorNum(Color color);
  Color getColorNum() const;
  
  static std::string colorName(const Color color);
  static Color colorNum(const std::string color);
};

#endif
