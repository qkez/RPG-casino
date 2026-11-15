#pragma once
#include <SFML/Graphics.hpp>
#include "GameLogic.h"

class UI {
public:
    UI();
    void draw(sf::RenderWindow& window, const GameLogic& game);

    // НОВОЕ: Функция, которая проверяет, попал ли клик по кнопке
    bool isSpinButtonClicked(int mouseX, int mouseY);

private:
    sf::RectangleShape spinButton;
    sf::RectangleShape reelBox;
};