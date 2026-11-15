#include "UI.h"
#include <iostream>

UI::UI() {
    if (!fontPixel.loadFromFile("font_pixel.ttf")) {
        std::cout << "ОШИБКА: Не найден font_pixel.ttf!" << std::endl;
    }
    if (!fontMain.loadFromFile("font_glitch.ttf")) {
        std::cout << "ОШИБКА: Не найден font_glitch.ttf!" << std::endl;
    }

    spinButton.setSize(sf::Vector2f(150, 50));
    spinButton.setPosition(325, 520);
    spinButton.setFillColor(sf::Color(0, 255, 100));

    // Пока другие кнопки просто созданы, даже если мы их не рисуем
    holdButton.setSize(sf::Vector2f(100, 40));
    holdButton.setPosition(100, 520);
    holdButton.setFillColor(sf::Color(255, 215, 0));

    hackButton.setSize(sf::Vector2f(100, 40));
    hackButton.setPosition(600, 520);
    hackButton.setFillColor(sf::Color(0, 255, 255));

    bloodSpinButton.setSize(sf::Vector2f(200, 50));
    bloodSpinButton.setPosition(300, 520);
    bloodSpinButton.setFillColor(sf::Color(255, 0, 0));

    robberyButton.setSize(sf::Vector2f(200, 50));
    robberyButton.setPosition(300, 520);
    robberyButton.setFillColor(sf::Color(255, 100, 0));

    saveButton.setSize(sf::Vector2f(100, 30));
    saveButton.setPosition(680, 10);
    saveButton.setFillColor(sf::Color(100, 100, 100));
}

std::string UI::getSymbolString(Symbol sym) {
    switch (sym) {
    case Symbol::SWORD: return "МЕЧ";
    case Symbol::BOMB: return "БОМБА";
    case Symbol::SHIELD: return "ЩИТ";
    case Symbol::LOCK: return "ЗАМОК";
    case Symbol::MEDKIT: return "ХИЛ";
    case Symbol::BATTERY: return "ЭНЕРГ";
    case Symbol::COIN: return "МОНЕТА";
    case Symbol::DIAMOND: return "АЛМАЗ";
    case Symbol::SKULL: return "ЧЕРЕП";
    case Symbol::JOKER: return "ДЖОКЕР";
    default: return "???";
    }
}

void UI::drawText(sf::RenderWindow& window, const std::string& text, int x, int y, int size, sf::Color color, bool isGlitch, bool useMainFont) {
    sf::Text sfText;

    // Выбираем шрифт: основной или пиксельный
    if (useMainFont) {
        sfText.setFont(fontMain);
    }
    else {
        sfText.setFont(fontPixel);
    }

    // МАГИЯ КОДИРОВКИ (чтобы русские буквы работали)
    sf::String sfString = sf::String::fromUtf8(text.begin(), text.end());
    sfText.setString(sfString);

    sfText.setCharacterSize(size);
    sfText.setPosition(x, y);

    // Если включен режим "глитча", делаем текст ярко-красным (или меняем цвет)
    if (isGlitch) {
        sfText.setFillColor(sf::Color(255, 0, 100)); // Кислотно-розовый
    }
    else {
        sfText.setFillColor(color);
    }

    window.draw(sfText);
}

void UI::drawBar(sf::RenderWindow& window, int x, int y, int value, int max, sf::Color color) {
    // Фон полоски (серый)
    sf::RectangleShape bg(sf::Vector2f(300, 25));
    bg.setPosition(x, y);
    bg.setFillColor(sf::Color(50, 50, 50));
    window.draw(bg);

    // Сама полоска (цветная)
    float percent = (float)value / max;
    if (percent < 0) percent = 0; // Защита от отрицательного значения
    sf::RectangleShape fill(sf::Vector2f(300 * percent, 25));
    fill.setPosition(x, y);
    fill.setFillColor(color);
    window.draw(fill);

    // Рамка (чтобы было красиво)
    sf::RectangleShape border(sf::Vector2f(300, 25));
    border.setPosition(x, y);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color::White);
    border.setOutlineThickness(2);
    window.draw(border);
}

void UI::draw(sf::RenderWindow& window, const GameLogic& game) {
    // --- 1. ИНФОРМАЦИЯ ОБ ИГРОКЕ ---
    // Если HP красное (после кровавого спина), рисуем красную полоску, иначе зеленую
    sf::Color hpColor = game.player.isHpRed ? sf::Color::Red : sf::Color::Green;
    drawBar(window, 50, 50, game.player.hp, game.player.maxHp, hpColor);

    // Выводим текст пиксельным шрифтом (false в предпоследнем параметре, если мы не хотим глитч)
    drawText(window, "HP: " + std::to_string(game.player.hp) + "/" + std::to_string(game.player.maxHp), 50, 80, 18, sf::Color::White, false, false);
    drawText(window, "КРЕДИТЫ: " + std::to_string(game.player.balance), 50, 110, 18, sf::Color::Yellow, false, false);
    drawText(window, "ЭНЕРГИЯ: " + std::to_string(game.player.energy) + "/100", 50, 140, 18, sf::Color::Cyan, false, false);


    // --- 2. ИНФОРМАЦИЯ О БОССЕ ---
    drawBar(window, 450, 50, game.currentBoss.hp, game.currentBoss.maxHp, sf::Color::Red);

    // Имя босса основным шрифтом (true в последнем параметре)
    drawText(window, game.currentBoss.name, 450, 80, 20, sf::Color::Magenta, false, true);
    drawText(window, "HP: " + std::to_string(game.currentBoss.hp) + "/" + std::to_string(game.currentBoss.maxHp), 450, 110, 18, sf::Color::White, false, false);

    // Если босс заряжается — выводим предупреждение с эффектом глитча (true)
    if (game.currentBoss.chargeTimer > 0) {
        drawText(window, "ВНИМАНИЕ: СУПЕР-УДАР!", 450, 140, 20, sf::Color::Red, true, true);
    }


    // --- 3. ЛОГ СООБЩЕНИЙ ---
    drawText(window, game.lastMessage, 50, 200, 18, sf::Color::White, false, false);


    // --- 4. БАРАБАНЫ ---
    for (int i = 0; i < 3; i++) {
        sf::RectangleShape reelBox(sf::Vector2f(120, 120));
        reelBox.setPosition(150.f + i * 160.f, 280.f); // Центрируем по экрану
        reelBox.setFillColor(sf::Color(30, 30, 50));
        reelBox.setOutlineColor(sf::Color::Magenta);
        reelBox.setOutlineThickness(3.f);
        window.draw(reelBox);

        // Выводим символ внутри квадрата основным шрифтом
        drawText(window, getSymbolString(game.reels[i]), 160.f + i * 160.f, 330.f, 24, sf::Color::White, false, true);
    }


    // --- 5. КНОПКИ (В зависимости от состояния) ---
    if (game.currentState == GameState::PLAYING) {
        window.draw(spinButton);
        drawText(window, "SPIN", 365, 535, 24, sf::Color::Black, false, true);

        window.draw(holdButton);
        drawText(window, "HOLD", 115, 530, 18, sf::Color::Black, false, true);

        window.draw(hackButton);
        drawText(window, "HACK", 615, 530, 18, sf::Color::Black, false, true);
    }
    else if (game.currentState == GameState::BLOOD_SPIN) {
        window.draw(bloodSpinButton);
        drawText(window, "КРОВАВЫЙ СПИН", 315, 535, 20, sf::Color::White, false, true);
    }
    else if (game.currentState == GameState::ROBBERY) {
        window.draw(robberyButton);
        drawText(window, "ОГРАБИТЬ КАЗИНО", 315, 535, 20, sf::Color::White, false, true);
    }

    // Кнопка сохранения всегда в углу
    window.draw(saveButton);
    drawText(window, "SAVE", 705, 12, 14, sf::Color::White, false, true);
}

// 6. ОБРАБОТКА КЛИКОВ
UIButton UI::getClickedButton(int mouseX, int mouseY, GameState currentState) {
    if (saveButton.getGlobalBounds().contains(mouseX, mouseY)) return UIButton::SAVE_EXIT;

    if (currentState == GameState::PLAYING) {
        if (spinButton.getGlobalBounds().contains(mouseX, mouseY)) return UIButton::SPIN;
        if (holdButton.getGlobalBounds().contains(mouseX, mouseY)) return UIButton::HOLD;
        if (hackButton.getGlobalBounds().contains(mouseX, mouseY)) return UIButton::HACK;
    }
    else if (currentState == GameState::BLOOD_SPIN) {
        if (bloodSpinButton.getGlobalBounds().contains(mouseX, mouseY)) return UIButton::BLOOD_SPIN;
    }
    else if (currentState == GameState::ROBBERY) {
        if (robberyButton.getGlobalBounds().contains(mouseX, mouseY)) return UIButton::ROBBERY;
    }

    return UIButton::NONE;
}
