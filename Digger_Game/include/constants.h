#pragma once
//constants file
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <vector>
#include <thread>

const int LIVES = 3;

const int STATS_ARRAY = 5;


const std::string WINDOW_TITLE = "Digger Editor";
const std::string FONT_PATH = "C:/Windows/Fonts/ahronbd.ttf";
const std::string FILE_STATISTIC = "stats.txt";

//data

const std::string FILE_PATH = "Board.txt";
//const char FILE_PATH_CHAR_ARRAY[] = "board.txt";

//size
const int MAX_ROWS = 6;
const int MAX_COLUMMS = 15;
const int WINDOW_WIDTH = sf::VideoMode::getDesktopMode().width + 18;
const int WINDOW_HEIGHT = sf::VideoMode::getDesktopMode().height;
const float OBJECT_WIDTH = 80;
const float OBJECT_HEIGHT = 80;
const int DATA_HEIGHT = 60;
//const float RECTANGLE_SIZE = 130;
//const float BUTTON_SIZE = 110;
//const float BUTTON_SPACE = 30;

//ui
const sf::Color RECTANGLE_COLOR_LIGHT = sf::Color(246, 223, 227);
const sf::Color RECTANGLE_COLOR_DARK = sf::Color(255, 153, 204);
const sf::Color TXT_COLOR_DARK = sf::Color(67, 44, 135);
const sf::Color RECTANGLE_COLOR_NOT_ALLOWD = sf::Color(255, 102, 102);
const sf::Color LINE_COLOR = sf::Color(129, 13, 152);
const sf::Color MENU_BACKGROUND = sf::Color(229, 204, 255);
const float LINE_THICKNESS = 5;


// chars
const char WALL = '#';
const char MONSTER = '!';
const char DIAMOND = 'D';
const char GIFT = '+';
const char STONE = '@';
const char DIGGER = '/';
const char SPACE = ' ';