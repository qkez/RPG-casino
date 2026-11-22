#include <SFML/Graphics.hpp>
#include "GameLogic.h"
#include "UI.h"

int main() {
    bool bloodSpinPending = false;
    sf::RenderWindow window(sf::VideoMode(1000, 750), "Cyberpunk RPG Casino");

    GameLogic game;
    UI ui;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (ui.isSelectingHold()) {
                    int selectedReel = ui.getReelIndexAt(event.mouseButton.x, event.mouseButton.y);
                    if (selectedReel >= 0) {
                        game.holdReel(selectedReel);
                        ui.cancelHoldSelection();
                        continue;
                    }
                }
                UIButton btn = ui.getClickedButton(event.mouseButton.x, event.mouseButton.y, game.currentState);

                if (btn == UIButton::SPIN) {
                    bloodSpinPending = false;
                    ui.startSpinAnimation();
                }
                else if (btn == UIButton::BLOOD_SPIN) {
                    bloodSpinPending = true;
                    ui.startBloodSpinAnimation(static_cast<int>(game.player.maxHp * 0.05f));
                }
                else if (btn == UIButton::ROBBERY) game.doRobbery();
                else if (btn == UIButton::SAVE_EXIT) {
                    game.saveGame();
                    window.close();
                }
                else if (btn == UIButton::HOLD) ui.armHoldSelection();
                else if (btn == UIButton::HACK) game.useHack(Symbol::JOKER);
            }
        }

        window.clear(sf::Color(10, 10, 20));
        ui.draw(window, game);
        window.display();

        // Сначала завершается анимация, затем применяются результаты спина.
        if (ui.takeSpinAnimationFinished()) {
            if (bloodSpinPending) {
                game.doBloodSpin();
                bloodSpinPending = false;
            } else {
                game.spin();
            }
        }
    }
    return 0;
}
