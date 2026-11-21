#pragma once
#include <SFML/Graphics.hpp>
#include "GameLogic.h"
#include <string>
#include <map>

enum class UIButton { NONE, SPIN, HOLD, HACK, BLOOD_SPIN, ROBBERY, SAVE_EXIT };

class UI {
public:
    UI();
    void draw(sf::RenderWindow& window, const GameLogic& game);
    UIButton getClickedButton(int mouseX, int mouseY, GameState currentState);
    void startSpinAnimation();
    void triggerBloodFlash(int hpLost);

private:
    sf::Font fontMain;
    sf::Font fontPixel;

    std::map<Symbol, sf::Texture> symbolTextures;

    // Текстуры главного героя и боссов
    sf::Texture playerTexture;
    std::map<BossType, sf::Texture> bossTextures;

    bool isSpinning = false;
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

    void drawBar(sf::RenderWindow& window, int x, int y, int value, int max, sf::Color color);
    void drawText(sf::RenderWindow& window, const std::string& text, int x, int y, int size, sf::Color color, bool isGlitch = false, bool useMainFont = true);
    std::string getSymbolString(Symbol sym);
};