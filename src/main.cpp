#include <SFML/Graphics.hpp>
#include "GameLogic.h"
#include "UI.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Cyberpunk RPG Casino");

    GameLogic game;
    UI ui;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            // Обработка кликов мыши
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {

                // Используем СТАРЫЙ метод проверки клика, который есть в UI.cpp
                if (ui.isSpinButtonClicked(event.mouseButton.x, event.mouseButton.y)) {
                    game.spin();
                }
            }
        }

        window.clear(sf::Color(10, 10, 20)); // Темный фон
        ui.draw(window, game); // Рисуем всё
        window.display();
    }
    return 0;
}