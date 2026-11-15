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

                // Получаем ID нажатой кнопки
                UIButton btn = ui.getClickedButton(event.mouseButton.x, event.mouseButton.y, game.currentState);

                // Вызываем нужные функции логики в зависимости от нажатой кнопки
                if (btn == UIButton::SPIN) game.spin();
                else if (btn == UIButton::BLOOD_SPIN) game.doBloodSpin();
                else if (btn == UIButton::ROBBERY) game.doRobbery();
                else if (btn == UIButton::SAVE_EXIT) {
                    game.saveGame();
                    window.close();
                }
                // Пока эти кнопки-заглушки, они просто не будут ничего делать при клике
                else if (btn == UIButton::HOLD) game.holdReel(0);
                else if (btn == UIButton::HACK) game.useHack(Symbol::SWORD);
            }
        }

        window.clear(sf::Color(10, 10, 20)); // Темный фон
        ui.draw(window, game); // Рисуем всё
        window.display();
    }
    return 0;
}