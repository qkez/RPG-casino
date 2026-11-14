#pragma once
feature/graphics-ui
#include <SFML/Graphics.hpp>
#include "GameLogic.h"

class UI {
public:
    UI();
    // Функция отрисовки: берет окно и данные игры, и рисует их
    void draw(sf::RenderWindow& window, const GameLogic& game);

private:
    // Здесь будут храниться наши фигуры (кнопка, квадрат)
    sf::RectangleShape spinButton;
    sf::RectangleShape reelBox;
};
main
