#pragma once
#include <string>
#include <vector>


// Наши 10 символов
enum class Symbol { SWORD, BOMB, SHIELD, LOCK, MEDKIT, BATTERY, COIN, DIAMOND, SKULL, JOKER };

// Наши боссы
enum class BossType { WOLF, DRAGON, CUTE_FINAL };

// Состояния игры
enum class GameState { PLAYING, BLOOD_SPIN, ROBBERY, GAME_OVER, WIN };

struct Player {
    int hp = 6666;
    int maxHp = 6666;
    int balance = 3427; // Стипендия:)
    int energy = 0;
    int maxEnergy = 100;
    bool isHpRed = false; // Флаг для красной шкалы после кров. спина
    int spinsSinceHold = 0; // Счетчик для кнопки "Захват"
};

struct Boss {
    std::string name;
    BossType type;
    int hp;
    int maxHp;
    int atk;
    int specAtk;
    int chargeTimer = 0; // 0 - не заряжается, 1 - заряжается, 2 - ударит в след. раз
    bool isBlocking = false; // Для дракона
};

class GameLogic {
public:
    GameLogic();

    // Данные для интерфейса
    Player player;
    Boss currentBoss;
    int bossIndex = 0; // 0 - волк, 1 - дракон, 2 - няшка
    Symbol reels[3];
    GameState currentState = GameState::PLAYING;
    std::string lastMessage = "Добро пожаловать в Казино!";
    int spinCost = 250;

    // Действия игрока
    void spin();
    void holdReel(int index); // Захват символа
    void useHack(Symbol targetSymbol); // Взлом 
    void doBloodSpin();
    void doRobbery();
    void nextBoss(); // Переход на следующий уровень

    // Сохранение
    void saveGame();
    void loadGame();

private:
    int shieldBlocksRemaining = 0;
    bool locksActive = false;
    void bossTurn();
    void checkWinOrLose();
    Symbol getRandomSymbol();
};
