#include "UI.h"
#include <iostream>
#include <cstdlib>
#include <cmath>

UI::UI() {
    if (!fontPixel.loadFromFile("assets/font_pixel.ttf")) {
        std::cout << "ОШИБКА: Не найден font_pixel.ttf!" << std::endl;
    }
    if (!fontMain.loadFromFile("assets/font_glitch.ttf")) {
        std::cout << "ОШИБКА: Не найден font_glitch.ttf!" << std::endl;
    }

    auto loadTex = [&](Symbol sym, const std::string& path) {
        sf::Texture tex;
        if (tex.loadFromFile(path)) {
            tex.setSmooth(true);
            symbolTextures[sym] = tex;
        }
        else {
            std::cout << "ОШИБКА загрузки картинки: " << path << std::endl;
        }
        };

    loadTex(Symbol::MEDKIT, "assets/aid-kit.png");
    loadTex(Symbol::BATTERY, "assets/batteries.png");
    loadTex(Symbol::BOMB, "assets/bomb.png");
    loadTex(Symbol::COIN, "assets/coin.png");
    loadTex(Symbol::DIAMOND, "assets/diamond.png");
    loadTex(Symbol::SKULL, "assets/skull.png");
    loadTex(Symbol::SWORD, "assets/sword.png");
    loadTex(Symbol::SHIELD, "assets/shield.png");
    loadTex(Symbol::LOCK, "assets/lock.png");
    loadTex(Symbol::JOKER, "assets/joker.png");

    // --- ЗАГРУЗКА КАРТИНОК ГЕРОЯ И БОССА ---
    if (playerTexture.loadFromFile("assets/knight.png")) {
        playerTexture.setSmooth(false); // Чёткие пиксели
    }
    else {
        std::cout << "ОШИБКА загрузки: assets/knight.png" << std::endl;
    }

    if (bossTextures[BossType::WOLF].loadFromFile("assets/wolf.png")) {
        bossTextures[BossType::WOLF].setSmooth(false);
    }
    else {
        std::cout << "ОШИБКА загрузки: assets/wolf.png" << std::endl;
    }

    // Когда добавите картинки для Дракончика и Милашки, просто раскомментируйте эти строчки:
    // if (bossTextures[BossType::DRAGON].loadFromFile("assets/dragon.png")) bossTextures[BossType::DRAGON].setSmooth(false);
    // if (bossTextures[BossType::CUTE_FINAL].loadFromFile("assets/cute.png")) bossTextures[BossType::CUTE_FINAL].setSmooth(false);

    spinButton.setSize(sf::Vector2f(150, 50));
    spinButton.setPosition(425, 650);
    spinButton.setFillColor(sf::Color(0, 255, 100));

    holdButton.setSize(sf::Vector2f(120, 48));
    holdButton.setPosition(140, 645);
    holdButton.setFillColor(sf::Color(20, 35, 50));
    holdButton.setOutlineColor(sf::Color(0, 220, 255));
    holdButton.setOutlineThickness(2.f);

    hackButton.setSize(sf::Vector2f(100, 40));
    hackButton.setPosition(750, 650);
    hackButton.setFillColor(sf::Color(25, 20, 40));
    hackButton.setOutlineColor(sf::Color(255, 0, 170));
    hackButton.setOutlineThickness(2.f);

    bloodSpinButton.setSize(sf::Vector2f(200, 50));
    bloodSpinButton.setPosition(400, 650);
    bloodSpinButton.setFillColor(sf::Color(255, 0, 0));

    robberyButton.setSize(sf::Vector2f(200, 50));
    robberyButton.setPosition(400, 650);
    robberyButton.setFillColor(sf::Color(255, 100, 0));

    saveButton.setSize(sf::Vector2f(100, 30));
    saveButton.setPosition(880, 10);
    saveButton.setFillColor(sf::Color(100, 100, 100));
}

void UI::armHoldSelection() { holdSelectionMode = true; }
bool UI::isSelectingHold() const { return holdSelectionMode; }
void UI::cancelHoldSelection() { holdSelectionMode = false; }
int UI::getReelIndexAt(int mouseX, int mouseY) const {
    for (int i = 0; i < 3; ++i) {
        sf::FloatRect bounds(250.f + i * 170.f, 475.f, 110.f, 110.f);
        if (bounds.contains(static_cast<float>(mouseX), static_cast<float>(mouseY)) && !isSpinning && !bloodSpinDelay)
            return i;
    }
    return -1;
}

void UI::startSpinAnimation() {
    isSpinning = true;
    bloodSpinDelay = false;
    spinAnimationFinished = false;
    spinTimer.restart();
    frameTimer.restart();
}

void UI::startBloodSpinAnimation(int hpCost) {
    triggerBloodFlash(hpCost);
    bloodSpinDelay = true;
    isSpinning = false;
    spinAnimationFinished = false;
    bloodSpinDelayTimer.restart();
}

bool UI::takeSpinAnimationFinished() {
    if (!spinAnimationFinished) return false;
    spinAnimationFinished = false;
    return true;
}

void UI::triggerBloodFlash(int hpLost) {
    isBloodFlashing = true;
    bloodHpLost = hpLost;
    bloodFlashTimer.restart();
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
    sfText.setFont(useMainFont ? fontMain : fontPixel);

    sf::String sfString = sf::String::fromUtf8(text.begin(), text.end());
    sfText.setString(sfString);
    sfText.setCharacterSize(size);
    sfText.setPosition(x, y);

    if (isGlitch) {
        sfText.setFillColor(sf::Color(255, 0, 100));
    }
    else {
        sfText.setFillColor(color);
    }

    window.draw(sfText);
}

void UI::drawWrappedText(sf::RenderWindow& window, const std::string& text, float x, float y, float maxWidth, unsigned int size, sf::Color color) {
    sf::Text line;
    line.setFont(fontPixel);
    line.setCharacterSize(size);
    line.setLineSpacing(1.25f);
    line.setFillColor(color);

    std::string wrapped;
    std::string currentLine;
    std::string word;
    auto appendWord = [&]() {
        if (word.empty()) return;
        std::string candidate = currentLine.empty() ? word : currentLine + " " + word;
        line.setString(sf::String::fromUtf8(candidate.begin(), candidate.end()));
        if (!currentLine.empty() && line.getLocalBounds().width > maxWidth) {
            wrapped += currentLine + "\n";
            currentLine = word;
        } else {
            currentLine = candidate;
        }
        word.clear();
    };

    for (char ch : text) {
        if (ch == ' ') appendWord();
        else word += ch;
    }
    appendWord();
    wrapped += currentLine;
    line.setString(sf::String::fromUtf8(wrapped.begin(), wrapped.end()));
    line.setPosition(x, y);
    window.draw(line);
}

void UI::drawBar(sf::RenderWindow& window, int x, int y, int value, int max, sf::Color color) {
    sf::RectangleShape bg(sf::Vector2f(300, 25));
    bg.setPosition(x, y);
    bg.setFillColor(sf::Color(50, 50, 50));
    window.draw(bg);

    float percent = (float)value / max;
    if (percent < 0) percent = 0;
    sf::RectangleShape fill(sf::Vector2f(300 * percent, 25));
    fill.setPosition(x, y);
    fill.setFillColor(color);
    window.draw(fill);

    sf::RectangleShape border(sf::Vector2f(300, 25));
    border.setPosition(x, y);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color::White);
    border.setOutlineThickness(2);
    window.draw(border);
}

void UI::draw(sf::RenderWindow& window, const GameLogic& game) {
    // --- 1. ИНФОРМАЦИЯ ОБ ИГРОКЕ ---
    sf::Color hpColor = game.player.isHpRed ? sf::Color::Red : sf::Color::Green;
    drawBar(window, 50, 50, game.player.hp, game.player.maxHp, hpColor);

    drawText(window, "HP: " + std::to_string(game.player.hp) + "/" + std::to_string(game.player.maxHp), 50, 95, 18, sf::Color::White, false, false);
    drawText(window, "КРЕДИТЫ: " + std::to_string(game.player.balance), 50, 125, 18, sf::Color::Yellow, false, false);
    drawText(window, "ЭНЕРГИЯ: " + std::to_string(game.player.energy) + "/100", 50, 155, 18, sf::Color::Cyan, false, false);

    // Картинка рыцаря под характеристиками игрока
    if (playerTexture.getSize().x > 0) {
        sf::Sprite playerSprite(playerTexture);
        playerSprite.setScale(125.f / playerTexture.getSize().x, 125.f / playerTexture.getSize().y);
        playerSprite.setPosition(50.f, 180.f);
        window.draw(playerSprite);
    }

    // --- 2. ИНФОРМАЦИЯ О БОССЕ ---
    drawBar(window, 650, 50, game.currentBoss.hp, game.currentBoss.maxHp, sf::Color::Red);
    drawText(window, game.currentBoss.name, 650, 80, 20, sf::Color::Magenta, false, true);
    drawText(window, "HP: " + std::to_string(game.currentBoss.hp) + "/" + std::to_string(game.currentBoss.maxHp), 650, 110, 18, sf::Color::White, false, false);

    if (game.currentBoss.chargeTimer != 0) {
        drawText(window, game.currentBoss.chargeTimer == -1 ? "УЛЬТА ГОТОВА!" : "БОСС ЗАРЯЖАЕТ УЛЬТУ", 650, 140, 17, sf::Color::Red, true, true);
    }

    // Картинка текущего босса под его характеристиками
    if (bossTextures.find(game.currentBoss.type) != bossTextures.end() && bossTextures[game.currentBoss.type].getSize().x > 0) {
        sf::Texture& bTex = bossTextures[game.currentBoss.type];
        sf::Sprite bossSprite(bTex);
        bossSprite.setScale(125.f / bTex.getSize().x, 125.f / bTex.getSize().y);
        bossSprite.setPosition(650.f, 165.f);
        window.draw(bossSprite);
    }

    // --- 3. ЛОГ СООБЩЕНИЙ (сдвинут вниз под портреты) ---
    if (isSpinning) {
        drawText(window, "БАРАБАНЫ КРУТЯТСЯ...", 332, 430, 16, sf::Color::Cyan, false, false);
    }
    else {
        sf::RectangleShape logPanel(sf::Vector2f(940.f, 125.f));
        logPanel.setPosition(30.f, 315.f);
        logPanel.setFillColor(sf::Color(25, 25, 40));
        logPanel.setOutlineColor(sf::Color(90, 90, 120));
        logPanel.setOutlineThickness(1.f);
        window.draw(logPanel);
        drawText(window, "РЕЗУЛЬТАТ СПИНА", 45, 320, 12, sf::Color(80, 255, 120), false, true);
        drawWrappedText(window, game.lastMessage, 45.f, 342.f, 910.f, 18, sf::Color(220, 240, 220));
    }

    // История боя: панель с цветным выводом полезных эффектов и атак.
    if (!game.combatEvents.empty()) {
        sf::RectangleShape eventPanel(sf::Vector2f(300.f, 130.f));
        eventPanel.setPosition(320.f, 175.f);
        eventPanel.setFillColor(sf::Color(20, 20, 35, 230));
        eventPanel.setOutlineColor(sf::Color(70, 70, 100));
        eventPanel.setOutlineThickness(1.f);
        window.draw(eventPanel);
        drawText(window, "ИСТОРИЯ БОЯ", 332, 178, 12, sf::Color(180, 180, 210), false, true);

        float eventY = 198.f;
        for (const auto& event : game.combatEvents) {
            sf::Color eventColor = event.positive
                ? sf::Color(80, 255, 120) : sf::Color(255, 90, 90);
            drawWrappedText(window, event.text, 332.f, eventY, 275.f, 11, eventColor);
            eventY += 18.f;
        }
    }

    // --- 4. БАРАБАНЫ С КАРТИНКАМИ И АНИМАЦИЕЙ (сдвинуты вниз) ---
    if (bloodSpinDelay && bloodSpinDelayTimer.getElapsedTime().asSeconds() >= 1.2f) {
        bloodSpinDelay = false;
        startSpinAnimation();
    }
    float elapsed = spinTimer.getElapsedTime().asSeconds();
    if (isSpinning) {
        if (frameTimer.getElapsedTime().asSeconds() > 0.06f) {
            for (int i = 0; i < 3; i++) {
                if (i != game.heldReelIndex) fakeReels[i] = static_cast<Symbol>(rand() % 10);
            }
            frameTimer.restart();
        }
        if (elapsed >= 1.5f) {
            isSpinning = false;
            spinAnimationFinished = true;
        }
    }

    for (int i = 0; i < 3; i++) {
        float boxX = 250.f + i * 170.f;
        float boxY = 475.f;
        float boxSize = 110.f;

        sf::RectangleShape reelBox(sf::Vector2f(boxSize, boxSize));
        reelBox.setPosition(boxX, boxY);
        reelBox.setFillColor(sf::Color(15, 15, 25));
        reelBox.setOutlineColor(i == game.heldReelIndex ? sf::Color::Cyan : sf::Color::Magenta);
        reelBox.setOutlineThickness(3.f);
        window.draw(reelBox);

        Symbol symbolToShow = game.reels[i];
        if (i != game.heldReelIndex && isSpinning && elapsed < (0.7f + i * 0.4f)) {
            symbolToShow = fakeReels[i];
        }

        if (symbolTextures.find(symbolToShow) != symbolTextures.end()) {
            sf::Sprite sprite(symbolTextures[symbolToShow]);

            float padding = 10.f;
            float targetSize = boxSize - padding * 2;
            sf::Vector2u texSize = symbolTextures[symbolToShow].getSize();

            sprite.setScale(targetSize / texSize.x, targetSize / texSize.y);
            sprite.setPosition(boxX + padding, boxY + padding);

            window.draw(sprite, sf::BlendAdd);
        }
        else {
            drawText(window, getSymbolString(symbolToShow), boxX + 10.f, boxY + 45.f, 22, sf::Color::White, false, true);
        }
    }

    // --- 5. КНОПКИ ---
    if (game.currentState == GameState::PLAYING) {
        window.draw(spinButton);
        drawText(window, "SPIN", 465, 665, 24, sf::Color::Black, false, true);

        holdButton.setFillColor(holdSelectionMode ? sf::Color(15, 65, 80) : sf::Color(20, 35, 50));
        holdButton.setOutlineColor(holdSelectionMode ? sf::Color(255, 220, 70) : sf::Color(0, 220, 255));
        window.draw(holdButton);
        sf::RectangleShape holdInner(sf::Vector2f(112.f, 40.f));
        holdInner.setPosition(144.f, 649.f);
        holdInner.setFillColor(sf::Color::Transparent);
        holdInner.setOutlineColor(holdSelectionMode ? sf::Color(255, 220, 70, 180) : sf::Color(0, 150, 190, 180));
        holdInner.setOutlineThickness(1.f);
        window.draw(holdInner);
        drawText(window, holdSelectionMode ? "ВЫБОР" : "HOLD", 165, 657, 20,
                 holdSelectionMode ? sf::Color(255, 220, 70) : sf::Color(0, 220, 255), false, true);
        if (holdSelectionMode) drawText(window, "КЛИКНИ БАРАБАН", 140, 625, 12, sf::Color(255, 220, 70), false, true);

        window.draw(hackButton);
        sf::RectangleShape hackInner(sf::Vector2f(92.f, 32.f));
        hackInner.setPosition(754.f, 654.f);
        hackInner.setFillColor(sf::Color::Transparent);
        hackInner.setOutlineColor(sf::Color(120, 50, 220, 200));
        hackInner.setOutlineThickness(1.f);
        window.draw(hackInner);
        drawText(window, "HACK", 765, 660, 18, sf::Color(255, 80, 210), false, true);
    }
    else if (game.currentState == GameState::BLOOD_SPIN) {
        window.draw(bloodSpinButton);
        drawText(window, "КРОВАВЫЙ СПИН", 415, 665, 20, sf::Color::White, false, true);
    }
    else if (game.currentState == GameState::ROBBERY) {
        window.draw(robberyButton);
        drawText(window, "ОГРАБИТЬ КАЗИНО", 415, 665, 20, sf::Color::White, false, true);
    }

    window.draw(saveButton);
    drawText(window, "SAVE", 905, 12, 14, sf::Color::White, false, true);

    // --- 6. АНИМАЦИЯ КРОВАВОЙ ВСПЫШКИ И ВЫЛЕТАЮЩИЙ ТЕКСТ ---
    if (isBloodFlashing) {
        float flashElapsed = bloodFlashTimer.getElapsedTime().asSeconds();
        float flashDuration = 1.2f;

        if (flashElapsed < flashDuration) {
            float progress = 1.0f - (flashElapsed / flashDuration);
            sf::Uint8 bgAlpha = static_cast<sf::Uint8>(190 * progress);
            sf::RectangleShape bloodOverlay(sf::Vector2f(1000.f, 750.f));
            bloodOverlay.setPosition(0.f, 0.f);
            bloodOverlay.setFillColor(sf::Color(180, 0, 0, bgAlpha));
            window.draw(bloodOverlay);
            sf::Text dmgText;
            dmgText.setFont(fontMain);
            dmgText.setString("-" + std::to_string(bloodHpLost) + " HP");
            dmgText.setCharacterSize(48);
            sf::FloatRect textBounds = dmgText.getLocalBounds();
            float textX = 500.f - (textBounds.width / 2.f);
            float textY = 350.f - (flashElapsed * 60.f);
            dmgText.setPosition(textX, textY);
            sf::Uint8 textAlpha = static_cast<sf::Uint8>(255 * progress);
            dmgText.setFillColor(sf::Color(255, 255, 255, textAlpha));
            dmgText.setOutlineColor(sf::Color(120, 0, 0, textAlpha));
            dmgText.setOutlineThickness(3.f);
            window.draw(dmgText);
        }
        else {
            isBloodFlashing = false;
        }
    }
}
UIButton UI::getClickedButton(int mouseX, int mouseY, GameState currentState) {
    if (saveButton.getGlobalBounds().contains(mouseX, mouseY)) return UIButton::SAVE_EXIT;
    if (isSpinning || bloodSpinDelay) return UIButton::NONE;
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