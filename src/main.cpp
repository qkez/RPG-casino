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

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                UIButton btn = ui.getClickedButton(event.mouseButton.x, event.mouseButton.y, game.currentState);

                if (btn == UIButton::SPIN) {
                    game.spin();
                    ui.startSpinAnimation();
                }
                else if (btn == UIButton::BLOOD_SPIN) {
                    int hpBefore = game.player.hp;
                    game.doBloodSpin();
                    int hpLost = hpBefore - game.player.hp;
                    ui.triggerBloodFlash(hpLost);
                }
                else if (btn == UIButton::ROBBERY) game.doRobbery();
                else if (btn == UIButton::SAVE_EXIT) {
                    game.saveGame();
                    window.close();
                }
                else if (btn == UIButton::HOLD) game.holdReel(0);
                else if (btn == UIButton::HACK) game.useHack(Symbol::SWORD);
            }
        }

        window.clear(sf::Color(10, 10, 20));
        ui.draw(window, game);
        window.display();
    }
    return 0;
}
