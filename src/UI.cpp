#include "UI.h"
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <algorithm>

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

    if (bossTextures[BossType::DRAGON].loadFromFile("assets/dragon.png")) {
        bossTextures[BossType::DRAGON].setSmooth(false);
    }
    else {
        std::cout << "ОШИБКА загрузки: assets/dragon.png" << std::endl;
    }

    if (bossTextures[BossType::CUTE_FINAL].loadFromFile("assets/cute.png")) {
        bossTextures[BossType::CUTE_FINAL].setSmooth(false);
    }
    else {
        std::cout << "ОШИБКА загрузки: assets/cute.png" << std::endl;
    }

    spinButton.setSize(sf::Vector2f(150, 50));
    spinButton.setPosition(425, 650);
    spinButton.setFillColor(sf::Color(0, 255, 100));

    holdButton.setSize(sf::Vector2f(120, 48));
    holdButton.setPosition(140, 645);
    holdButton.setFillColor(sf::Color(20, 35, 50));
    holdButton.setOutlineColor(sf::Color(0, 220, 255));
    holdButton.setOutlineThickness(2.f);

    hackButton.setSize(sf::Vector2f(120, 48));
    hackButton.setPosition(740, 645);
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
    saveButton.setFillColor(sf::Color(20, 35, 50));
    saveButton.setOutlineColor(sf::Color(80, 255, 120));
    saveButton.setOutlineThickness(2.f);

    newGameButton.setSize(sf::Vector2f(180, 30));
    newGameButton.setPosition(680, 10);
    newGameButton.setFillColor(sf::Color(35, 25, 45));
    newGameButton.setOutlineColor(sf::Color(255, 170, 60));
    newGameButton.setOutlineThickness(2.f);

    // Временные кнопки разработчика для быстрого переключения боссов.
    devPrevButton.setSize(sf::Vector2f(95.f, 30.f));
    devPrevButton.setPosition(430.f, 10.f);
    devPrevButton.setFillColor(sf::Color(30, 20, 45));
    devPrevButton.setOutlineColor(sf::Color(170, 70, 255));
    devPrevButton.setOutlineThickness(2.f);

    devNextButton.setSize(sf::Vector2f(95.f, 30.f));
    devNextButton.setPosition(535.f, 10.f);
    devNextButton.setFillColor(sf::Color(30, 20, 45));
    devNextButton.setOutlineColor(sf::Color(170, 70, 255));
    devNextButton.setOutlineThickness(2.f);

    devRobberyButton.setSize(sf::Vector2f(200.f, 28.f));
    devRobberyButton.setPosition(430.f, 45.f);
    devRobberyButton.setFillColor(sf::Color(45, 20, 15));
    devRobberyButton.setOutlineColor(sf::Color(255, 110, 40));
    devRobberyButton.setOutlineThickness(2.f);
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

void UI::startLevelTransition() {
    levelTransitionIsActive = true;
    levelTransitionClock.restart();
    transitionReelClock.restart();
    transitionReels[0] = Symbol::COIN;
    transitionReels[1] = Symbol::JOKER;
    transitionReels[2] = Symbol::SWORD;
    holdSelectionMode = false;
    isSpinning = false;
    bloodSpinDelay = false;
}

bool UI::levelTransitionActive() const {
    return levelTransitionIsActive;
}

void UI::finishLevelTransition() {
    levelTransitionIsActive = false;
}

void UI::startRobberyAnimation() {
    robberyAnimationIsActive = true;
    robberyResultIsActive = false;
    robberyAnimationClock.restart();
    holdSelectionMode = false;
}

bool UI::robberyAnimationActive() const {
    return robberyAnimationIsActive;
}

bool UI::takeRobberyAnimationFinished() {
    if (!robberyAnimationIsActive || robberyAnimationClock.getElapsedTime().asSeconds() < 3.4f)
        return false;
    robberyAnimationIsActive = false;
    return true;
}

void UI::startRobberyResult(bool success, int reward) {
    robberyResultSuccess = success;
    robberyResultReward = reward;
    robberyResultIsActive = true;
    robberyResultClock.restart();
}

bool UI::robberyResultActive() const {
    return robberyResultIsActive && robberyResultClock.getElapsedTime().asSeconds() < 1.9f;
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


bool UI::introActive() const { return introIsActive; }

void UI::handleIntroInput() {
    if (introClock.getElapsedTime().asSeconds() >= 4.25f) introIsActive = false;
}

void UI::drawIntro(sf::RenderWindow& window) {
    float t = introClock.getElapsedTime().asSeconds();
    window.clear(sf::Color(3, 2, 5));
    auto centered = [&](const std::string& str, float y, unsigned size, sf::Color color, bool mainFont = true, float xCenter = 500.f) {
        sf::Text txt;
        txt.setFont(mainFont ? fontMain : fontPixel);
        txt.setString(sf::String::fromUtf8(str.begin(), str.end()));
        txt.setCharacterSize(size);
        txt.setFillColor(color);
        auto b = txt.getLocalBounds();
        txt.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
        txt.setPosition(xCenter, y);
        window.draw(txt);
    };
    auto glitchBar = [&](float y, float width, sf::Color color) {
        sf::RectangleShape bar(sf::Vector2f(width, 2.f + static_cast<float>(rand() % 5)));
        bar.setPosition(static_cast<float>(rand() % 1000), y);
        bar.setFillColor(color);
        window.draw(bar);
    };

    // Короткие вспышки и фрагменты заголовка до появления барабанов.
    if (t > 0.45f && t < 3.45f) {
        float flicker = (rand() % 100 < 18) ? 0.35f : 1.f;
        sf::Color red(255, 35, 55, static_cast<sf::Uint8>(220 * flicker));
        centered("THE HOUSE", 230.f, 42, red);
        centered("ALWAYS WINS", 285.f, 38, sf::Color(190, 15, 30, static_cast<sf::Uint8>(220 * flicker)));
        if (rand() % 100 < 35) {
            glitchBar(220.f + static_cast<float>(rand() % 100), 250.f + static_cast<float>(rand() % 500), sf::Color(255, 0, 40, 180));
            centered("// SIGNAL INTERRUPTED //", 330.f, 13, sf::Color(220, 40, 60), false);
        }
    }

    // Барабаны прокручиваются, а затем фиксируют символы.
    if (t >= 1.35f && t < 3.45f) {
        if (introReelClock.getElapsedTime().asSeconds() > 0.075f) {
            for (int i = 0; i < 3; ++i) introReels[i] = static_cast<Symbol>(rand() % 10);
            introReelClock.restart();
        }
        centered("- - - - - - - - - - - - - - -", 390.f, 16, sf::Color(160, 20, 35), false);
        for (int i = 0; i < 3; ++i) {
            float x = 310.f + i * 130.f;
            sf::RectangleShape box(sf::Vector2f(115.f, 115.f));
            box.setPosition(x, 420.f);
            box.setFillColor(sf::Color(12, 8, 15));
            box.setOutlineColor(sf::Color(255, 25, 65));
            box.setOutlineThickness(3.f);
            window.draw(box);
            if (symbolTextures.find(introReels[i]) != symbolTextures.end()) {
                sf::Sprite s(symbolTextures[introReels[i]]);
                auto sz = symbolTextures[introReels[i]].getSize();
                s.setScale(90.f / sz.x, 90.f / sz.y);
                s.setPosition(x + 12.f, 432.f);
                window.draw(s);
            } else centered(getSymbolString(introReels[i]), 475.f, 20, sf::Color::White, true, x + 57.f);
        }
    }

    // Финальный сбой.
    if (t >= 3.45f && t < 4.25f) {
        sf::RectangleShape wash(sf::Vector2f(1000.f, 750.f));
        wash.setFillColor(sf::Color(80, 0, 10, static_cast<sf::Uint8>(rand() % 65)));
        window.draw(wash);
        centered("!  SYSTEM ERROR  !", 335.f, 42, sf::Color(255, 30, 45));
        centered("NO EXIT FOUND", 390.f, 24, sf::Color(180, 70, 90), false);
        for (int i = 0; i < 12; ++i) glitchBar(static_cast<float>(rand() % 750), 200.f + static_cast<float>(rand() % 650), sf::Color(255, 0, 35, 170));
    }

    // Финальный экран ожидания.
    if (t >= 4.25f) {
        centered("CASINO", 290.f, 72, sf::Color(255, 35, 55));
        centered("THE HOUSE ALWAYS WINS", 360.f, 20, sf::Color(160, 25, 40), false);
        for (int i = 0; i < 3; ++i) {
            float x = 310.f + i * 130.f;
            sf::RectangleShape box(sf::Vector2f(115.f, 115.f));
            box.setPosition(x, 410.f);
            box.setFillColor(sf::Color(12, 8, 15));
            box.setOutlineColor(sf::Color(180, 20, 40));
            box.setOutlineThickness(2.f);
            window.draw(box);
            Symbol sym = (i == 0) ? Symbol::COIN : (i == 1 ? Symbol::JOKER : Symbol::SWORD);
            if (symbolTextures.find(sym) != symbolTextures.end()) {
                sf::Sprite s(symbolTextures[sym]);
                auto sz = symbolTextures[sym].getSize();
                s.setScale(90.f / sz.x, 90.f / sz.y);
                s.setPosition(x + 12.f, 422.f);
                window.draw(s);
            }
        }
        if (static_cast<int>(t * 2.f) % 2 == 0) centered("PRESS ANY KEY TO ENTER", 590.f, 22, sf::Color(255, 170, 170), false);
    }

    // CRT scanlines и лёгкий шум по всему экрану.
    for (int y = 0; y < 750; y += 5) {
        sf::RectangleShape line(sf::Vector2f(1000.f, 1.f));
        line.setPosition(0.f, static_cast<float>(y));
        line.setFillColor(sf::Color(0, 0, 0, 55));
        window.draw(line);
    }
    if (rand() % 100 < 25) glitchBar(static_cast<float>(rand() % 750), 300.f + static_cast<float>(rand() % 700), sf::Color(220, 0, 30, 80));
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
    else if (game.currentBoss.type == BossType::DRAGON && game.currentBoss.isBlocking) {
        drawText(window, "БЛОК КРЫЛЬЯМИ АКТИВЕН!", 650, 140, 15, sf::Color::Cyan, true, true);
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
    auto drawCenteredButtonText = [&](const std::string& label, const sf::RectangleShape& button,
                                      unsigned int size, sf::Color color) {
        sf::Text text;
        text.setFont(fontMain);
        text.setString(sf::String::fromUtf8(label.begin(), label.end()));
        text.setCharacterSize(size);
        text.setFillColor(color);
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
        sf::Vector2f buttonPos = button.getPosition();
        sf::Vector2f buttonSize = button.getSize();
        text.setPosition(buttonPos.x + buttonSize.x / 2.f, buttonPos.y + buttonSize.y / 2.f);
        window.draw(text);
    };

    if (game.currentState == GameState::PLAYING) {
        window.draw(spinButton);
        drawCenteredButtonText("SPIN", spinButton, 24, sf::Color::Black);

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
        else drawText(window, game.player.spinsSinceHold >= 3 ? "READY" :
                      "RECHARGE: " + std::to_string(game.player.spinsSinceHold) + "/3",
                      155, 695, 10, game.player.spinsSinceHold >= 3 ? sf::Color(0, 220, 255) : sf::Color(160, 160, 180), false, true);

        window.draw(hackButton);
        sf::RectangleShape hackInner(sf::Vector2f(112.f, 40.f));
        hackInner.setPosition(744.f, 649.f);
        hackInner.setFillColor(sf::Color::Transparent);
        hackInner.setOutlineColor(sf::Color(120, 50, 220, 200));
        hackInner.setOutlineThickness(1.f);
        window.draw(hackInner);
        drawCenteredButtonText("HACK", hackButton, 18, sf::Color(255, 80, 210));
    }
    else if (game.currentState == GameState::BLOOD_SPIN) {
        window.draw(bloodSpinButton);
        drawCenteredButtonText("КРОВАВЫЙ СПИН", bloodSpinButton, 20, sf::Color::White);
    }
    else if (game.currentState == GameState::ROBBERY) {
        window.draw(robberyButton);
        drawCenteredButtonText("ОГРАБИТЬ КАЗИНО", robberyButton, 20, sf::Color::White);
    }

    window.draw(newGameButton);
    sf::RectangleShape newGameInner(sf::Vector2f(172.f, 22.f));
    newGameInner.setPosition(684.f, 14.f);
    newGameInner.setFillColor(sf::Color::Transparent);
    newGameInner.setOutlineColor(sf::Color(180, 100, 40, 180));
    newGameInner.setOutlineThickness(1.f);
    window.draw(newGameInner);
    drawCenteredButtonText("NEW GAME", newGameButton, 13, sf::Color(255, 190, 90));

    window.draw(saveButton);
    sf::RectangleShape saveInner(sf::Vector2f(92.f, 22.f));
    saveInner.setPosition(884.f, 14.f);
    saveInner.setFillColor(sf::Color::Transparent);
    saveInner.setOutlineColor(sf::Color(40, 170, 90, 180));
    saveInner.setOutlineThickness(1.f);
    window.draw(saveInner);
    drawCenteredButtonText("SAVE", saveButton, 14, sf::Color(100, 255, 140));

    // Временная панель разработчика.
    window.draw(devPrevButton);
    drawCenteredButtonText("< PREV", devPrevButton, 12, sf::Color(210, 150, 255));
    window.draw(devNextButton);
    drawCenteredButtonText("NEXT >", devNextButton, 12, sf::Color(210, 150, 255));
    window.draw(devRobberyButton);
    drawCenteredButtonText("DEV ROBBERY", devRobberyButton, 12, sf::Color(255, 150, 80));

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

    if (levelTransitionIsActive)
        drawLevelTransition(window, game);

    if (robberyAnimationIsActive)
        drawRobberyAnimation(window);
    if (robberyResultActive())
        drawRobberyResult(window);
}

void UI::drawRobberyAnimation(sf::RenderWindow& window) {
    const float elapsed = robberyAnimationClock.getElapsedTime().asSeconds();
    const float progress = std::max(0.f, std::min(1.f, elapsed / 3.4f));
    sf::RectangleShape shade(sf::Vector2f(1000.f, 750.f));
    shade.setFillColor(sf::Color(2, 1, 5, 238));
    window.draw(shade);
    for (int y = 0; y < 750; y += 6) {
        sf::RectangleShape line(sf::Vector2f(1000.f, 1.f));
        line.setPosition(0.f, static_cast<float>(y));
        line.setFillColor(sf::Color(0, 0, 0, 75));
        window.draw(line);
    }
    auto centered = [&](const std::string& value, float y, unsigned int size, sf::Color color, bool mainFont = true) {
        sf::Text text;
        text.setFont(mainFont ? fontMain : fontPixel);
        text.setString(sf::String::fromUtf8(value.begin(), value.end()));
        text.setCharacterSize(size);
        text.setFillColor(color);
        const sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
        text.setPosition(500.f, y);
        window.draw(text);
    };
    const bool alarm = (static_cast<int>(elapsed * 7.f) % 2) == 0;
    if (alarm) {
        sf::RectangleShape flash(sf::Vector2f(1000.f, 750.f));
        flash.setFillColor(sf::Color(120, 0, 18, 24));
        window.draw(flash);
    }
    centered("/// SECURITY BREACH ///", 175.f, 18, sf::Color(255, 65, 75), false);
    centered("ROBBERY IN PROGRESS", 260.f, 46, sf::Color(255, 25, 50));
    centered(elapsed < 1.15f ? "BYPASSING SECURITY" :
             (elapsed < 2.45f ? "ACCESSING THE VAULT" : "TRANSFERRING CREDITS..."),
             330.f, 19, sf::Color(220, 95, 105), false);
    sf::RectangleShape barBack(sf::Vector2f(520.f, 18.f));
    barBack.setPosition(240.f, 390.f);
    barBack.setFillColor(sf::Color(35, 8, 15));
    barBack.setOutlineColor(sf::Color(180, 25, 45));
    barBack.setOutlineThickness(2.f);
    window.draw(barBack);
    sf::RectangleShape barFill(sf::Vector2f(516.f * progress, 14.f));
    barFill.setPosition(242.f, 392.f);
    barFill.setFillColor(sf::Color(255, 25, 55));
    window.draw(barFill);
    for (int i = 0; i < 13; ++i) {
        sf::RectangleShape tick(sf::Vector2f(2.f, 26.f));
        tick.setPosition(240.f + i * 43.f, 386.f);
        tick.setFillColor(sf::Color(5, 3, 8, 210));
        window.draw(tick);
    }
    centered("CASINO MAINFRAME // CONNECTION UNSTABLE", 445.f, 13, sf::Color(155, 55, 70), false);
    centered(elapsed < 2.7f ? "DO NOT INTERRUPT" : "FINALIZING...", 545.f, 17,
             alarm ? sf::Color(255, 45, 60) : sf::Color(190, 25, 45), false);
    if (rand() % 100 < 40) {
        sf::RectangleShape glitch(sf::Vector2f(180.f + static_cast<float>(rand() % 500), 2.f + static_cast<float>(rand() % 4)));
        glitch.setPosition(static_cast<float>(rand() % 800), static_cast<float>(rand() % 730));
        glitch.setFillColor(sf::Color(255, 15, 45, 150));
        window.draw(glitch);
    }
}

void UI::drawRobberyResult(sf::RenderWindow& window) {
    const float elapsed = robberyResultClock.getElapsedTime().asSeconds();
    const float fade = std::max(0.f, 1.f - elapsed / 1.9f);
    const sf::Color accent = robberyResultSuccess ? sf::Color(70, 255, 135) : sf::Color(255, 35, 55);
    sf::RectangleShape shade(sf::Vector2f(1000.f, 750.f));
    shade.setFillColor(sf::Color(2, 1, 5, static_cast<sf::Uint8>(225.f * fade)));
    window.draw(shade);
    auto centered = [&](const std::string& value, float y, unsigned int size, sf::Color color) {
        sf::Text text;
        text.setFont(fontMain);
        text.setString(sf::String::fromUtf8(value.begin(), value.end()));
        text.setCharacterSize(size);
        text.setFillColor(color);
        const sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
        text.setPosition(500.f, y);
        window.draw(text);
    };
    centered(robberyResultSuccess ? "ACCESS GRANTED" : "CAUGHT", 310.f, 58, accent);
    centered(robberyResultSuccess ? "+" + std::to_string(robberyResultReward) + " CREDITS" : "SECURITY LOCKDOWN // GAME OVER",
             390.f, 20, sf::Color(235, 190, 195));
    for (int i = 0; i < 7; ++i) {
        if (rand() % 100 < 65) {
            sf::RectangleShape glitch(sf::Vector2f(120.f + static_cast<float>(rand() % 650), 2.f));
            glitch.setPosition(static_cast<float>(rand() % 850), 270.f + static_cast<float>(rand() % 180));
            glitch.setFillColor(sf::Color(accent.r, accent.g, accent.b, static_cast<sf::Uint8>(120.f * fade)));
            window.draw(glitch);
        }
    }
}

void UI::drawLevelTransition(sf::RenderWindow& window, const GameLogic& game) {
    const float elapsed = levelTransitionClock.getElapsedTime().asSeconds();
    const float duration = 6.8f;
    if (elapsed >= duration) {
        levelTransitionIsActive = false;
        return;
    }

    // Затемняем текущий интерфейс, сохраняя его под заставкой.
    const float transitionFade = std::max(0.f, std::min(1.f, (elapsed - 6.25f) / 0.55f));
    sf::RectangleShape shade(sf::Vector2f(1000.f, 750.f));
    shade.setFillColor(sf::Color(3, 2, 6, static_cast<sf::Uint8>(242.f * (1.f - transitionFade))));
    window.draw(shade);

    auto centered = [&](const std::string& value, float y, unsigned int size,
                        sf::Color color, bool mainFont = true, float centerX = 500.f) {
        sf::Text text;
        text.setFont(mainFont ? fontMain : fontPixel);
        text.setString(sf::String::fromUtf8(value.begin(), value.end()));
        text.setCharacterSize(size);
        text.setFillColor(color);
        const sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
        text.setPosition(centerX, y);
        window.draw(text);
    };

    auto glitchBar = [&](float y, float width, sf::Uint8 alpha) {
        sf::RectangleShape bar(sf::Vector2f(width, 2.f + static_cast<float>(rand() % 4)));
        bar.setPosition(static_cast<float>(rand() % 1000), y);
        bar.setFillColor(sf::Color(255, 15, 45, alpha));
        window.draw(bar);
    };

    // Короткие CRT scanlines объединяют переход со стартовой заставкой.
    for (int y = 0; y < 750; y += 6) {
        sf::RectangleShape line(sf::Vector2f(1000.f, 1.f));
        line.setPosition(0.f, static_cast<float>(y));
        line.setFillColor(sf::Color(0, 0, 0, 65));
        window.draw(line);
    }

    if (elapsed < 1.15f) {
        if (elapsed < 0.16f || (rand() % 100 < 12)) {
            sf::RectangleShape flash(sf::Vector2f(1000.f, 750.f));
            flash.setFillColor(sf::Color(180, 0, 20, elapsed < 0.16f ? 75 : 25));
            window.draw(flash);
        }
        centered("VICTORY", 300.f, 66, sf::Color(255, 35, 55));
        centered("TARGET ELIMINATED", 365.f, 19, sf::Color(210, 70, 85), false);
        if (rand() % 100 < 45) {
            glitchBar(275.f + static_cast<float>(rand() % 100), 260.f + static_cast<float>(rand() % 500), 190);
            centered("// SIGNAL INTERRUPTED //", 420.f, 14, sf::Color(190, 35, 55), false);
        }
    }
    else if (elapsed < 2.35f) {
        centered("WARNING", 260.f, 22, sf::Color(255, 90, 100), false);
        centered("SYSTEM OVERRIDE", 335.f, 48, sf::Color(255, 25, 55));
        centered("UNAUTHORIZED PROGRESSION", 395.f, 16, sf::Color(190, 55, 70), false);
        for (int i = 0; i < 9; ++i)
            glitchBar(static_cast<float>(rand() % 750), 180.f + static_cast<float>(rand() % 650), 150);
    }
    else if (elapsed < 3.75f) {
        if (transitionReelClock.getElapsedTime().asSeconds() > 0.055f) {
            for (int i = 0; i < 3; ++i)
                transitionReels[i] = static_cast<Symbol>(rand() % 10);
            transitionReelClock.restart();
        }
        centered("REINITIALIZING GAME STATE", 300.f, 18, sf::Color(220, 65, 80), false);
        for (int i = 0; i < 3; ++i) {
            const float x = 320.f + i * 125.f;
            sf::RectangleShape reel(sf::Vector2f(105.f, 105.f));
            reel.setPosition(x, 350.f);
            reel.setFillColor(sf::Color(12, 7, 14));
            reel.setOutlineColor(sf::Color(255, 25, 60));
            reel.setOutlineThickness(3.f);
            window.draw(reel);

            const Symbol symbol = transitionReels[i];
            auto found = symbolTextures.find(symbol);
            if (found != symbolTextures.end() && found->second.getSize().x > 0) {
                sf::Sprite sprite(found->second);
                const sf::Vector2u size = found->second.getSize();
                sprite.setScale(82.f / size.x, 82.f / size.y);
                sprite.setPosition(x + 11.f, 361.f);
                window.draw(sprite);
            } else {
                centered(getSymbolString(symbol), 402.f, 24, sf::Color::White, true, x + 52.f);
            }
        }
        centered(" // RANDOMIZING NEXT TARGET // ", 500.f, 13, sf::Color(170, 35, 55), false);
        for (int i = 0; i < 5; ++i)
            glitchBar(340.f + static_cast<float>(rand() % 130), 100.f + static_cast<float>(rand() % 300), 120);
    }
    else {
        const bool reveal = elapsed >= 5.35f;
        const float scanProgress = (elapsed - 3.75f) / 1.6f;
        const bool finalScreen = reveal;
        const BossType type = game.currentBoss.type;
        auto boss = bossTextures.find(type);

        if (!finalScreen) {
            centered("SEARCHING NEXT TARGET...", 185.f, 20, sf::Color(255, 55, 75), false);
            centered("TARGET FOUND", 590.f, 17, sf::Color(220, 65, 85), false);
        } else {
            centered("LEVEL " + (game.bossIndex + 1 < 10 ? std::string("0") : std::string())
                     + std::to_string(game.bossIndex + 1), 145.f, 48, sf::Color(255, 35, 60));
            centered("NEW THREAT DETECTED", 205.f, 17, sf::Color(220, 65, 85), false);
            centered(game.currentBoss.name, 625.f, 30, sf::Color(255, 90, 120));
            centered("THE HOUSE GETS STRONGER", 675.f, 13, sf::Color(175, 45, 65), false);
        }

        if (boss != bossTextures.end() && boss->second.getSize().x > 0) {
            sf::Sprite sprite(boss->second);
            const sf::Vector2u size = boss->second.getSize();
            const float scale = std::min(300.f / static_cast<float>(size.x),
                                         340.f / static_cast<float>(size.y));
            sprite.setScale(scale, scale);
            const sf::FloatRect bounds = sprite.getGlobalBounds();
            sprite.setPosition(500.f - bounds.width / 2.f, 390.f - bounds.height / 2.f);
            if (!finalScreen) {
                const sf::Uint8 alpha = static_cast<sf::Uint8>(55.f + 150.f * std::max(0.f, std::min(1.f, scanProgress)));
                sprite.setColor(sf::Color(255, 35, 55, alpha));
            }
            window.draw(sprite);
        } else {
            centered("/// UNKNOWN ENTITY ///", 390.f, 23, sf::Color(255, 35, 65), false);
        }

        if (!finalScreen) {
            const float scanY = 230.f + std::max(0.f, std::min(1.f, scanProgress)) * 330.f;
            sf::RectangleShape scanLine(sf::Vector2f(420.f, 3.f));
            scanLine.setPosition(290.f, scanY);
            scanLine.setFillColor(sf::Color(255, 20, 50, 220));
            window.draw(scanLine);
            sf::RectangleShape glow(sf::Vector2f(420.f, 15.f));
            glow.setPosition(290.f, scanY - 6.f);
            glow.setFillColor(sf::Color(255, 0, 35, 35));
            window.draw(glow);
            if (rand() % 100 < 35)
                glitchBar(250.f + static_cast<float>(rand() % 300), 180.f + static_cast<float>(rand() % 500), 150);
        }
    }
}
UIButton UI::getClickedButton(int mouseX, int mouseY, GameState currentState) {
    // Блокируем управление во время анимаций спина и ограбления.
    if (isSpinning || bloodSpinDelay || robberyAnimationActive() || robberyResultActive()) return UIButton::NONE;
    if (newGameButton.getGlobalBounds().contains(mouseX, mouseY)) return UIButton::NEW_GAME;
    if (saveButton.getGlobalBounds().contains(mouseX, mouseY)) return UIButton::SAVE_EXIT;
    if (devPrevButton.getGlobalBounds().contains(mouseX, mouseY)) return UIButton::DEV_PREV;
    if (devNextButton.getGlobalBounds().contains(mouseX, mouseY)) return UIButton::DEV_NEXT;
    if (devRobberyButton.getGlobalBounds().contains(mouseX, mouseY)) return UIButton::DEV_ROBBERY;
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