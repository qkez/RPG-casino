#pragma once
#include <SFML/Graphics.hpp>
#include "GameLogic.h"
#include <string> 

enum class UIButton { NONE, SPIN, HOLD, HACK, BLOOD_SPIN, ROBBERY, SAVE_EXIT };

class UI {
public:
    UI();
    void draw(sf::RenderWindow& window, const GameLogic& game);
    UIButton getClickedButton(int mouseX, int mouseY, GameState currentState);

private:
    sf::Font fontMain;      // Для заголовков и кнопок (font_gitch)
    sf::Font fontPixel;     // Для логов и цифр (font_pixel)

    sf::RectangleShape spinButton;
    sf::RectangleShape holdButton;
    sf::RectangleShape hackButton;
    sf::RectangleShape bloodSpinButton;
    sf::RectangleShape robberyButton;
    sf::RectangleShape saveButton;

    void drawBar(sf::RenderWindow& window, int x, int y, int value, int max, sf::Color color);
    void drawText(sf::RenderWindow& window, const std::string& text, int x, int y, int size, sf::Color color, bool isGlitch = false, bool useMainFont = true);

    
    std::string getSymbolString(Symbol sym);
};