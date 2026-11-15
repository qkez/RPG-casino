#include "GameLogic.h"
#include <random>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <string>

// ==========================================
// КОНСТРУКТОР
// ==========================================
GameLogic::GameLogic() {
    loadGame(); // Пытаемся загрузить прогресс

    // Если сохранения нет и босс не создан, начинаем сначала
    if (bossIndex == 0 && currentBoss.name.empty()) {
        player = { 6666, 6666, 3427, 0, 100, false, 0 }; // Начальные статы игрока
        currentBoss = { "Дикий Волк", BossType::WOLF, 6000, 6000, 200, 800, 0, false };
        currentState = GameState::PLAYING;
        lastMessage = "Добро пожаловать! Сделайте спин.";
    }
}

// ==========================================
// ГЕНЕРАЦИЯ СЛУЧАЙНОГО СИМВОЛА
// ==========================================
Symbol GameLogic::getRandomSymbol() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 9); // От 0 до 9 (10 символов)
    return static_cast<Symbol>(dist(gen));
}

// ==========================================
// ЛОГИКА СПИНА
// ==========================================
void GameLogic::spin() {
    // Если игра окончена или мы в другом режиме - не крутим
    if (currentState != GameState::PLAYING && currentState != GameState::BLOOD_SPIN) return;

    // Проверка на кровавый спин
    if (player.balance < spinCost) {
        currentState = GameState::BLOOD_SPIN;
        lastMessage = "Мало денег! Кровавый спин будет стоить " + std::to_string(static_cast<int>(player.maxHp * 0.05f)) + " HP.";
        return;
    }

    player.balance -= spinCost; // Списываем деньги
    player.spinsSinceHold++;    // Счетчик для захвата (Hold)

    // Крутим барабаны
    reels[0] = getRandomSymbol();
    reels[1] = getRandomSymbol();
    reels[2] = getRandomSymbol();

    // --- МАГИЯ ПОДСЧЕТА СИМВОЛОВ ---
    // Считаем, сколько раз выпал каждый символ (индексы 0-9)
    int counts[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    counts[static_cast<int>(reels[0])]++;
    counts[static_cast<int>(reels[1])]++;
    counts[static_cast<int>(reels[2])]++;

    // --- ЛОГИКА ДЖОКЕРА (🃏) ---
    int jokerCount = counts[static_cast<int>(Symbol::JOKER)];
    if (jokerCount == 3) {
        currentBoss.hp = 1;
        lastMessage = "СУПЕР-ДЖЕКПОТ! 🃏🃏🃏 Боссу остается 1 HP!";
    }
    else {
        // Если 2 джокера, ищем третий символ и делаем его тройным
        if (jokerCount == 2) {
            for (int i = 0; i < 9; i++) {
                if (counts[i] == 1) { counts[i] = 3; break; }
            }
        }
        // Если 1 джокер, ищем два одинаковых и делаем их тройными
        else if (jokerCount == 1) {
            for (int i = 0; i < 9; i++) {
                if (counts[i] == 2) { counts[i] = 3; break; }
            }
        }

        // --- ОБРАБОТКА ВЫПАВШИХ СИМВОЛОВ ---
        // Сначала проверяем тройки (Джекпоты), потом двойки
        if (counts[static_cast<int>(Symbol::SWORD)] == 3) {
            int dmg = static_cast<int>(currentBoss.hp * 0.30f);
            if (dmg < 666) dmg = 666;
            currentBoss.hp -= dmg;
            lastMessage = "ДЖЕКПОТ! 🗡️🗡️🗡️ Урон боссу: " + std::to_string(dmg);
        }
        else if (counts[static_cast<int>(Symbol::BOMB)] == 3) {
            int dmg = static_cast<int>(currentBoss.hp * 0.50f);
            if (dmg < 888) dmg = 888;
            currentBoss.hp -= dmg;
            lastMessage = "ДЖЕКПОТ! 💣💣💣 Взрыв! Урон: " + std::to_string(dmg);
        }
        else if (counts[static_cast<int>(Symbol::MEDKIT)] == 3) {
            player.hp = player.maxHp;
            player.isHpRed = false; // Возвращаем зеленый цвет
            lastMessage = "ДЖЕКПОТ! 🧪🧪🧪 Полное восстановление HP!";
        }
        else if (counts[static_cast<int>(Symbol::BATTERY)] == 3) {
            player.energy = player.maxEnergy;
            lastMessage = "ДЖЕКПОТ! 🔋🔋🔋 Энергия восстановлена полностью!";
        }
        else if (counts[static_cast<int>(Symbol::COIN)] == 3) {
            player.balance *= 2;
            if (player.balance == 0) player.balance = 500;
            lastMessage = "ДЖЕКПОТ! 🪙🪙🪙 Кредиты удвоены! Теперь у вас: " + std::to_string(player.balance);
        }
        else if (counts[static_cast<int>(Symbol::DIAMOND)] == 3) {
            player.balance *= 10;
            if (player.balance == 0) player.balance = 2500;
            lastMessage = "ДЖЕКПОТ! 💎💎💎 Кредиты умножены на 10! Теперь у вас: " + std::to_string(player.balance);
        }
        else if (counts[static_cast<int>(Symbol::SKULL)] == 3) {
            player.hp = 1;
            player.isHpRed = true;
            lastMessage = "ДЖЕКПОТ! 💀💀💀 У вас остался 1 HP!";
        }

        // --- ОБЫЧНЫЕ ВЫПАДЕНИЯ (ДВОЙКИ) ---
        else if (counts[static_cast<int>(Symbol::SWORD)] == 2) {
            int dmg = static_cast<int>(currentBoss.hp * 0.05f);
            if (dmg < 300) dmg = 300;
            currentBoss.hp -= dmg;
            lastMessage = "2 меча! Урон боссу: " + std::to_string(dmg);
        }
        else if (counts[static_cast<int>(Symbol::BOMB)] == 2) {
            int dmg = static_cast<int>(currentBoss.hp * 0.10f);
            if (dmg < 600) dmg = 600;
            currentBoss.hp -= dmg;
            lastMessage = "2 бомбы! Урон: " + std::to_string(dmg);
        }
        else if (counts[static_cast<int>(Symbol::MEDKIT)] == 2) {
            int heal = static_cast<int>(player.maxHp * 0.15f);
            player.hp += heal;
            if (player.hp > player.maxHp) player.hp = player.maxHp;
            player.isHpRed = false;
            lastMessage = "2 аптечки! Лечение: +" + std::to_string(heal) + " HP";
        }
        else if (counts[static_cast<int>(Symbol::BATTERY)] == 2) {
            player.energy += 20;
            if (player.energy > player.maxEnergy) player.energy = player.maxEnergy;
            lastMessage = "2 батарейки! +20 энергии.";
        }
        else if (counts[static_cast<int>(Symbol::COIN)] == 2) {
            player.balance += 666;
            lastMessage = "2 монеты! +666 кредитов.";
        }
        else if (counts[static_cast<int>(Symbol::DIAMOND)] == 2) {
            player.balance += 2222;
            lastMessage = "2 алмаза! +2222 кредита.";
        }
        else if (counts[static_cast<int>(Symbol::SKULL)] == 2) {
            int dmg = static_cast<int>(player.hp * 0.04f);
            player.hp -= dmg;
            lastMessage = "2 черепа! Вы потеряли " + std::to_string(dmg) + " HP.";
        }
        else {
            lastMessage = "Ничего не выпало. Босс атакует!";
        }
    }

    bossTurn();       // Босс отвечает
    checkWinOrLose(); // Проверяем, не умер ли кто-то
}

// ==========================================
// ХОД БОССА (АТАКИ)
// ==========================================
void GameLogic::bossTurn() {
    if (currentBoss.hp <= 0) return; // Если босс мертв, он не бьет

    // Босс всегда бьет обычной атакой в свой ход
    player.hp -= currentBoss.atk;
    lastMessage += " Босс бьет на " + std::to_string(currentBoss.atk) + " урона.";

    // Проверяем, не умер ли игрок от обычной атаки
    if (player.hp <= 0) {
        checkWinOrLose();
        return;
    }

    // Логика заряда спец-атаки (цикл 3 хода: 0 -> 1 -> 2 -> удар)
    if (currentBoss.chargeTimer == 0) {
        currentBoss.chargeTimer = 1;
        lastMessage += " Босс напрягается...";
    }
    else if (currentBoss.chargeTimer == 1) {
        currentBoss.chargeTimer = 2;
        lastMessage += " ВНИМАНИЕ! Босс готовит СУПЕР-УДАР!";
    }
    else if (currentBoss.chargeTimer == 2) {
        // Босс наносит спец-удар вместо обычного (или в дополнение, тут решай сама. 
        // В коде ниже он наносит спец-удар, а обычный урон уже прошел в начале функции)
        player.hp -= currentBoss.specAtk;
        lastMessage += " Босс нанес СУПЕР-УДАР на " + std::to_string(currentBoss.specAtk) + " урона!";
        currentBoss.chargeTimer = 0; // Сброс цикла
    }
}

// ==========================================
// КРОВАВЫЙ СПИН
// ==========================================
void GameLogic::doBloodSpin() {
    int hpCost = static_cast<int>(player.maxHp * 0.05f);
    player.hp -= hpCost;
    player.isHpRed = true; // Шкала становится красной
    player.balance += 500; // Даем денег, чтобы игрок мог играть
    currentState = GameState::PLAYING;

    lastMessage = "Кровавый спин! -" + std::to_string(hpCost) + " HP. Выжившие получают 500 кредитов.";
    checkWinOrLose();
}

// ==========================================
// ПРОВЕРКА НА ПОБЕДУ/ПОРАЖЕНИЕ
// ==========================================
void GameLogic::checkWinOrLose() {
    if (player.hp <= 0) {
        player.hp = 0;
        if (player.balance <= 0) {
            currentState = GameState::ROBBERY; // Последний шанс
            lastMessage = "Вы на грани! Попробуйте ограбить казино!";
        }
        else {
            currentState = GameState::GAME_OVER;
            lastMessage = "Вы погибли. Игра окончена.";
        }
    }

    if (currentBoss.hp <= 0) {
        nextBoss();
    }
}

// ==========================================
// ПЕРЕХОД НА СЛЕДУЮЩЕГО БОССА
// ==========================================
void GameLogic::nextBoss() {
    bossIndex++;
    player.maxHp = static_cast<int>(player.maxHp * 1.1f); // +10% к макс HP
    player.hp = static_cast<int>(player.maxHp * 0.5f);   // Восстановление 50% HP
    player.isHpRed = false;

    if (bossIndex == 1) {
        currentBoss = { "Дракончик", BossType::DRAGON, 16665, 16665, 333, 1332, 0, false };
        lastMessage = "Босс повержен! Появился Дракончик!";
    }
    else if (bossIndex == 2) {
        currentBoss = { "Милашка", BossType::CUTE_FINAL, 33330, 33330, 466, 1864, 0, false };
        lastMessage = "Дракончик повержен! Финальный босс - Милашка!";
    }
    else {
        currentState = GameState::WIN;
        lastMessage = "ВЫ ПРОШЛИ ИГРУ!";
    }
}

// ==========================================
// ЗАГЛУШКИ (Пока не реализованы)
// ==========================================
void GameLogic::holdReel(int index) {}
void GameLogic::useHack(Symbol targetSymbol) {}
void GameLogic::doRobbery() {}
void GameLogic::saveGame() {}
void GameLogic::loadGame() {}