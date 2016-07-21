//Problem        : Mug Color
//Language       : C++
//Compiled Using : g++
//Version        : GCC 4.9.1
//Input for your program will be provided from STDIN
//Print out all output from your program to STDOUT

#include <iostream>
#include <string>
#include <algorithm>
#include <climits> 
#include <string>

using namespace std;

enum Color {
    WHITE, BLACK, BLUE, RED , YELLOW, MAX_COLOR
};

string colorName(const Color color) {
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
};

Color colorNum(const string color) {
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
};

void setColorNum(Color color, char& colorSet) {
  char x = 1;
  for (int c = WHITE; c < color; ++c) {
    x = x << 1;
  }
  colorSet |= x;
};

Color getColorNum(char colorSet) {
  char color = colorSet;
  for (int c = WHITE; c < MAX_COLOR; ++c) {
    if (!(color & 1)) {
      return static_cast<Color>(c);
    }
    color = color >> 1;
  }
	return MAX_COLOR;
};

void setColor(string color, char& colorSet) {
  setColorNum(colorNum(color), colorSet);
};

string getColor(char colorSet) {
  return colorName(getColorNum(colorSet));
};

int main() {
	int n;
	cin >> n;
	
	char colorSet = 0;
	
	string color;
	for (int i = 0; i < n; ++i) {
		cin >> color;
		setColor(color, colorSet);
	}
	
	cout << getColor(colorSet) << endl;

    return 0;
}
