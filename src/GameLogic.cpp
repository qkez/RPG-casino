#include "GameLogic.h"
#include <random>
#include <fstream>
#include <iostream>

GameLogic::GameLogic() {
    loadGame(); // При запуске пытаемся загрузить прогресс
    if (currentState == GameState::WIN) return; // Если всё пройдено, ничего не грузим

    // Если это первый запуск, ставим Волка
    if (bossIndex == 0 && currentBoss.maxHp == 0) {
        currentBoss = { "Дикий Волк", BossType::WOLF, 6000, 6000, 200, 800 };
    }
}

Symbol GameLogic::getRandomSymbol() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 9);
    return static_cast<Symbol>(dist(gen));
}

void GameLogic::spin() {
    if (currentState != GameState::PLAYING && currentState != GameState::BLOOD_SPIN) return;

    // Проверка на кровавый спин
    if (player.balance < spinCost) {
        currentState = GameState::BLOOD_SPIN;
        lastMessage = "Мало денег! Кровавый спин будет стоить " + std::to_string((int)(player.maxHp * 0.05)) + " HP.";
        return;
    }

    player.balance -= spinCost;
    player.spinsSinceHold++; // Увеличиваем счетчик для захвата

    reels[0] = getRandomSymbol();
    reels[1] = getRandomSymbol();
    reels[2] = getRandomSymbol();

    // --- ЛОГИКА СИМВОЛОВ ---
    // Здесь мы проверяем, что выпало
    // потом допишем все 10 символов

    lastMessage = "Вы прокрутили барабаны!";

    // Логика атаки мечами
    if (reels[0] == Symbol::SWORD && reels[1] == Symbol::SWORD) {
        int dmg = currentBoss.hp * 0.05f;
        if (dmg < 300) dmg = 300;
        currentBoss.hp -= dmg;
        lastMessage = "2 меча! Урон по боссу: " + std::to_string(dmg);
    }
    else if (reels[0] == Symbol::SWORD && reels[1] == Symbol::SWORD && reels[2] == Symbol::SWORD) {
        int dmg = currentBoss.hp * 0.30f;
        if (dmg < 666) dmg = 666;
        currentBoss.hp -= dmg;
        lastMessage = "ДЖЕКПОТ! 3 меча! Урон: " + std::to_string(dmg);
    }

    // Ход босса
    bossTurn();
    checkWinOrLose();
}

void GameLogic::bossTurn() {
    if (currentBoss.hp <= 0) return;

    // Логика заряда спец-атаки
    if (currentBoss.chargeTimer == 0) {
        currentBoss.chargeTimer = 1; // Начинает заряжаться
        lastMessage += " Босс напрягается...";
    }
    else if (currentBoss.chargeTimer == 1) {
        currentBoss.chargeTimer = 2; // Зарядился
        lastMessage += " Босс готовит СУПЕР-УДАР!";
    }
    else {
        // Наносит спец-атаку
        player.hp -= currentBoss.specAtk;
        lastMessage += " Босс нанес СУПЕР-УДАР на " + std::to_string(currentBoss.specAtk) + " урона!";
        currentBoss.chargeTimer = 0; // Сброс
    }
}

void GameLogic::doBloodSpin() {
    int hpCost = player.maxHp * 0.05f;
    player.hp -= hpCost;
    player.isHpRed = true; // Шкала становится красной
    player.balance += 500; // Даем денег, чтобы игрок мог играть
    currentState = GameState::PLAYING;
    lastMessage = "Кровавый спин! -" + std::to_string(hpCost) + " HP. Выжившие получают 500 кредитов.";
    checkWinOrLose();
}

void GameLogic::checkWinOrLose() {
    if (player.hp <= 0) {
        if (player.balance <= 0) {
            currentState = GameState::ROBBERY; // Последний шанс
            lastMessage = "Вы на грани! Попробуйте ограбить казино!";
        }
        else {
            currentState = GameState::GAME_OVER;
        }
    }
    if (currentBoss.hp <= 0) {
        nextBoss();
    }
}

void GameLogic::nextBoss() {
    bossIndex++;
    player.maxHp = player.maxHp * 1.1f; // +10% к макс HP
    player.hp = player.maxHp * 0.5f; // Восстановление 50% HP

    if (bossIndex == 1) {
        currentBoss = { "Дракончик", BossType::DRAGON, 16665, 16665, 333, 1332 };
    }
    else if (bossIndex == 2) {
        currentBoss = { "Няшка", BossType::CUTE_FINAL, 33330, 33330, 466, 1864 };
    }
    else {
        currentState = GameState::WIN;
        lastMessage = "ВЫ ПРОШЛИ ИГРУ!";
    }
}