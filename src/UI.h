#pragma once
#include <SFML/Graphics.hpp>
#include "GameLogic.h"
#include <string>
#include <map>

enum class UIButton { NONE, SPIN, HOLD, HACK, BLOOD_SPIN, ROBBERY, SAVE_EXIT, NEW_GAME };

class UI {
public:
    UI();
    void draw(sf::RenderWindow& window, const GameLogic& game);
    bool introActive() const;
    void drawIntro(sf::RenderWindow& window);
    void handleIntroInput();
    UIButton getClickedButton(int mouseX, int mouseY, GameState currentState);
    void startSpinAnimation();
    void startBloodSpinAnimation(int hpCost);
    bool takeSpinAnimationFinished();
    void triggerBloodFlash(int hpLost);
    void armHoldSelection();
    bool isSelectingHold() const;
    int getReelIndexAt(int mouseX, int mouseY) const;
    void cancelHoldSelection();

private:
    sf::Font fontMain;
    sf::Font fontPixel;

    bool introIsActive = true;
    sf::Clock introClock;
    sf::Clock introReelClock;
    Symbol introReels[3] = { Symbol::COIN, Symbol::JOKER, Symbol::SWORD };

    std::map<Symbol, sf::Texture> symbolTextures;

    // Текстуры главного героя и боссов
    sf::Texture playerTexture;
    std::map<BossType, sf::Texture> bossTextures;

    bool isSpinning = false;
    bool holdSelectionMode = false;
    bool bloodSpinDelay = false;
    sf::Clock bloodSpinDelayTimer;
    bool spinAnimationFinished = false;
    sf::Clock spinTimer;
    sf::Clock frameTimer;
    Symbol fakeReels[3] = {Symbol::SWORD, Symbol::COIN, Symbol::SKULL};

    bool isBloodFlashing = false;
    sf::Clock bloodFlashTimer;
    int bloodHpLost = 0;

    sf::RectangleShape spinButton;
    sf::RectangleShape holdButton;
    sf::RectangleShape hackButton;
    sf::RectangleShape bloodSpinButton;
    sf::RectangleShape robberyButton;
    sf::RectangleShape saveButton;
    sf::RectangleShape newGameButton;

    void drawBar(sf::RenderWindow& window, int x, int y, int value, int max, sf::Color color);
    void drawWrappedText(sf::RenderWindow& window, const std::string& text, float x, float y, float maxWidth, unsigned int size, sf::Color color);
    void drawText(sf::RenderWindow& window, const std::string& text, int x, int y, int size, sf::Color color, bool isGlitch = false, bool useMainFont = true);
    std::string getSymbolString(Symbol sym);
};