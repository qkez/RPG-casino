#include "UI.h"
#include <iostream>

UI::UI() {
    // Настраиваем кнопку
    spinButton.setSize(sf::Vector2f(200.f, 50.f));
    spinButton.setFillColor(sf::Color::Green);
    spinButton.setPosition(300.f, 500.f);

    // Настраиваем барабан
    reelBox.setSize(sf::Vector2f(100.f, 100.f));
    reelBox.setFillColor(sf::Color(50, 50, 70));
    reelBox.setPosition(250.f, 300.f);
    reelBox.setOutlineColor(sf::Color::Cyan);
    reelBox.setOutlineThickness(2.f);
}

void UI::draw(sf::RenderWindow& window, const GameLogic& game) {
    // Рисуем 3 квадрата-барабана
    for (int i = 0; i < 3; i++) {
        reelBox.setPosition(250.f + (i * 120.f), 300.f);
        window.draw(reelBox);
    }

    // Рисуем кнопку
    window.draw(spinButton);

    // Загружаем шрифт 
    sf::Font font;
    if (font.loadFromFile("arial.ttf")) {
        // Вывод сообщения игры
        sf::Text msgText;
        msgText.setFont(font);
        msgText.setString(game.lastMessage);
        msgText.setCharacterSize(20);
        msgText.setFillColor(sf::Color::White);
        msgText.setPosition(50.f, 100.f);
        window.draw(msgText);

        // Вывод HP игрока
        sf::Text hpText;
        hpText.setFont(font);
        hpText.setString("Player HP: " + std::to_string(game.player.hp));
        hpText.setCharacterSize(24);
        hpText.setFillColor(sf::Color::Green);
        hpText.setPosition(50.f, 50.f);
        window.draw(hpText);

        // Вывод Баланса
        sf::Text balText;
        balText.setFont(font);
        balText.setString("Balance: " + std::to_string(game.player.balance));
        balText.setCharacterSize(24);
        balText.setFillColor(sf::Color::Yellow);
        balText.setPosition(350.f, 50.f);
        window.draw(balText);
    }
}

// Проверка клика
bool UI::isSpinButtonClicked(int mouseX, int mouseY) {
    return spinButton.getGlobalBounds().contains(mouseX, mouseY);
}