#include <SFML/Graphics.hpp>
#include "GameLogic.h"
#include "UI.h"

int main() {
    bool bloodSpinPending = false;
    sf::RenderWindow window(sf::VideoMode(1000, 750), "Cyberpunk RPG Casino");
    sf::View gameView(sf::FloatRect(0.f, 0.f, 1000.f, 750.f));
    auto updateGameView = [&](unsigned int width, unsigned int height) {
        if (width == 0 || height == 0) return;
        constexpr float logicalAspect = 1000.f / 750.f;
        const float windowAspect = static_cast<float>(width) / static_cast<float>(height);
        sf::FloatRect viewport(0.f, 0.f, 1.f, 1.f);
        if (windowAspect > logicalAspect) {
            viewport.width = logicalAspect / windowAspect;
            viewport.left = (1.f - viewport.width) / 2.f;
        } else {
            viewport.height = windowAspect / logicalAspect;
            viewport.top = (1.f - viewport.height) / 2.f;
        }
        gameView.setViewport(viewport);
        window.setView(gameView);
    };
    updateGameView(window.getSize().x, window.getSize().y);

    GameLogic game;
    UI ui;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (event.type == sf::Event::Resized) updateGameView(event.size.width, event.size.height);

            if (ui.introActive()) {
                if (event.type == sf::Event::KeyPressed || event.type == sf::Event::MouseButtonPressed)
                    ui.handleIntroInput();
                continue;
            }

            if (ui.levelTransitionActive()) {
                if (event.type == sf::Event::KeyPressed || event.type == sf::Event::MouseButtonPressed)
                    ui.finishLevelTransition();
                continue;
            }
            if (ui.victoryAnimationActive()) {
                if (event.type == sf::Event::KeyPressed ||
                    event.type == sf::Event::MouseButtonPressed) {
                    ui.finishVictoryAnimation();
                }
                continue;
            }

            if (ui.robberyAnimationActive() || ui.robberyResultActive())
                continue;

            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (ui.isSelectingHold()) {
                    sf::Vector2f logicalMouse = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                    int selectedReel = ui.getReelIndexAt(static_cast<int>(logicalMouse.x), static_cast<int>(logicalMouse.y));
                    ui.cancelHoldSelection();
                    if (selectedReel >= 0) {
                        game.holdReel(selectedReel);
                        continue;
                    }
                }
                sf::Vector2f logicalMouse = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
                UIButton btn = ui.getClickedButton(static_cast<int>(logicalMouse.x), static_cast<int>(logicalMouse.y), game.currentState);

                if (btn == UIButton::SPIN) {
                    bloodSpinPending = false;
                    ui.startSpinAnimation();
                }
                else if (btn == UIButton::BLOOD_SPIN) {
                    bloodSpinPending = true;
                    ui.startBloodSpinAnimation(static_cast<int>(game.player.maxHp * 0.05f));
                }
                else if (btn == UIButton::ROBBERY) ui.startRobberyAnimation();                else if (btn == UIButton::SAVE_EXIT) {
                    game.saveGame();
                }
                else if (btn == UIButton::NEW_GAME) {
                    game.restartGame();
                    bloodSpinPending = false;
                    ui.cancelHoldSelection();
                }                else if (btn == UIButton::HOLD) ui.armHoldSelection();
                else if (btn == UIButton::HACK) game.useHack(Symbol::JOKER);
            }
        }

        if (ui.introActive()) {
            ui.drawIntro(window);
        } else {
            window.clear(sf::Color(10, 10, 20));
            ui.draw(window, game);
        }
        window.display();

        // Результат ограбления рассчитывается только после завершения заставки.
        if (ui.takeRobberyAnimationFinished()) {
            game.doRobbery();
            const bool success = game.currentState == GameState::PLAYING;
            ui.startRobberyResult(success, success ? game.player.balance : 0);
        }

        // Сначала завершается анимация, затем применяются результаты спина.
        if (ui.takeSpinAnimationFinished()) {
            const int previousBossIndex = game.bossIndex;
            if (bloodSpinPending) {
                game.doBloodSpin();
                bloodSpinPending = false;
            } else {
                game.spin();
            }

            // Если победа открыла следующего босса, запускаем отдельную заставку.
            if (game.bossIndex != previousBossIndex && game.currentState == GameState::PLAYING)
                ui.startLevelTransition();
            if (game.currentState == GameState::WIN)
                ui.startVictoryAnimation();
        }
    }
    return 0;
}
