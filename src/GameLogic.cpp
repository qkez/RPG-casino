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
        if (jokerCount == 2) {
            for (int i = 0; i < static_cast<int>(Symbol::JOKER); i++) {
                if (counts[i] == 1) { counts[i] = 3; break; }
            }
        }
        else if (jokerCount == 1) {
            int pairedSymbol=-1, firstSingle=-1, secondSingle=-1;
            for (int i=0; i<static_cast<int>(Symbol::JOKER); i++) {
                if (counts[i]==2) pairedSymbol=i;
                else if (counts[i]==1) {
                    if (firstSingle==-1) firstSingle=i;
                    else secondSingle=i;
                }
            }
            if (pairedSymbol!=-1) counts[pairedSymbol]=3;
            else if (firstSingle!=-1 && secondSingle!=-1) {
                auto comboScore=[this](int symbol) {
                    const bool playerInDanger=player.hp<=player.maxHp/2;
                    switch(static_cast<Symbol>(symbol)) {
                    case Symbol::MEDKIT:
                        return playerInDanger ? 1000+(player.maxHp-player.hp)/10 : 250;
                    case Symbol::SWORD:
                    case Symbol::BOMB:
                        return currentBoss.hp>0 ? 800+(currentBoss.maxHp-currentBoss.hp)/20 : 0;
                    case Symbol::DIAMOND: return 500;
                    case Symbol::COIN: return 400;
                    case Symbol::BATTERY: return player.energy<player.maxEnergy ? 350 : 150;
                    case Symbol::SHIELD: return shieldBlocksRemaining == 0 ? 900 : 300;
                    case Symbol::LOCK: return !locksActive && !armorActive ? 850 : 250;
                    case Symbol::SKULL: return -100;
                    default: return -1000;
                    }
                };
                int chosen=comboScore(firstSingle)>=comboScore(secondSingle)?firstSingle:secondSingle;
                counts[chosen]=2;
            }
        }

        // --- ОБРАБОТКА ВЫПАВШИХ СИМВОЛОВ ---
        // Сначала проверяем тройки (Джекпоты), потом двойки
        if (counts[static_cast<int>(Symbol::SWORD)] == 3) {
            int dmg = static_cast<int>(currentBoss.maxHp * 0.40f);
            currentBoss.hp -= dmg;
            if (currentBoss.hp < 0) currentBoss.hp = 0;
            lastMessage = "ДЖЕКПОТ! 🗡️🗡️🗡️ Урон боссу: " + std::to_string(dmg);
        }
        else if (counts[static_cast<int>(Symbol::BOMB)] == 3) {
            int dmg = static_cast<int>(currentBoss.maxHp * 0.50f);
            currentBoss.hp -= dmg;
            if (currentBoss.hp < 0) currentBoss.hp = 0;
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
        else if (counts[static_cast<int>(Symbol::SHIELD)] == 3) {
            shieldBlocksRemaining += 3;
            lastMessage = "ДЖЕКПОТ! 🛡️🛡️🛡️ Следующие 3 атаки босса заблокированы!";
        }
        else if (counts[static_cast<int>(Symbol::LOCK)] == 3) {
            locksActive = true;
            armorActive = false;
            lastMessage = "ДЖЕКПОТ! 🔒🔒🔒 Все обычные атаки босса заблокированы до конца боя!";
        }

        // --- ОБЫЧНЫЕ ВЫПАДЕНИЯ (ДВОЙКИ) ---
        else if (counts[static_cast<int>(Symbol::SWORD)] == 2) {
            int dmg = static_cast<int>(currentBoss.maxHp * 0.05f);
            currentBoss.hp -= dmg;
            if (currentBoss.hp < 0) currentBoss.hp = 0;
            lastMessage = "2 меча! Урон боссу: " + std::to_string(dmg);
        }
        else if (counts[static_cast<int>(Symbol::BOMB)] == 2) {
            int dmg = static_cast<int>(currentBoss.maxHp * 0.10f);
            currentBoss.hp -= dmg;
            if (currentBoss.hp < 0) currentBoss.hp = 0;
            lastMessage = "2 бомбы! Урон: " + std::to_string(dmg);
        }
        else if (counts[static_cast<int>(Symbol::SHIELD)] == 2) {
            shieldBlocksRemaining += 1;
            lastMessage = "2 щита! Следующая атака босса будет полностью заблокирована.";
        }
        else if (counts[static_cast<int>(Symbol::LOCK)] == 2) {
            armorActive = true;
            lastMessage = "2 замка! Броня снижает урон обычных атак босса на 30% до конца боя.";
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
            lastMessage = "Комбинация не собрана. Босс может атаковать!";
        }
    }

    bossTurn();       // Босс отвечает
    checkWinOrLose(); // Проверяем, не умер ли кто-то
}

// ==========================================
// ХОД БОССА (АТАКИ)
// ==========================================
void GameLogic::bossTurn() {
    if (currentBoss.hp <= 0) return;

    auto takeBossAttack = [this](int damage, bool isSpecial) {
        if (shieldBlocksRemaining > 0) {
            --shieldBlocksRemaining;
            lastMessage += " Щит полностью заблокировал атаку босса!";
            return;
        }
        if (!isSpecial && locksActive) {
            lastMessage += " Замок полностью остановил обычную атаку босса!";
            return;
        }

        int finalDamage = damage;
        if (!isSpecial && armorActive) {
            finalDamage = static_cast<int>(damage * 0.70f);
        }

        player.hp -= finalDamage;
        lastMessage += isSpecial
            ? " Босс нанес УЛЬТИМАТИВНЫЙ УДАР на " + std::to_string(finalDamage) + " урона!"
            : " Босс нанес обычный удар на " + std::to_string(finalDamage) + " урона.";
    };

    // После начала зарядки босс пропускает два полных спина.
    // На следующем спине он выпускает ультимативную атаку.
    if (currentBoss.chargeTimer > 0) {
        --currentBoss.chargeTimer;
        lastMessage += " Босс заряжает ульту... Он пропускает атаку.";
        if (currentBoss.chargeTimer == 0) {
            currentBoss.chargeTimer = -1; // Зарядка завершена, ульта готова к следующему спину.
        }
        return;
    }
    if (currentBoss.chargeTimer == -1) {
        currentBoss.chargeTimer = 0;
        lastMessage += " Босс завершил зарядку!";
        takeBossAttack(currentBoss.specAtk, true);
        return;
    }

    // Случайный выбор действия: 70% — обычная атака, 30% — зарядка ульты.
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> action(1, 100);

    if (action(gen) <= 70) {
        takeBossAttack(currentBoss.atk, false);
    }
    else {
        currentBoss.chargeTimer = 2;
        lastMessage += " Босс начал заряжать ульту! Следующие два спина он не атакует.";
    }
}
void GameLogic::doBloodSpin() {
    // Кровавый спин доступен только в специальном состоянии,
    // когда обычный спин уже нельзя оплатить.
    if (currentState != GameState::BLOOD_SPIN) return;

    int hpCost = static_cast<int>(player.maxHp * 0.05f);
    player.hp -= hpCost;
    player.isHpRed = true; // Шкала становится красной

    // Если цена кровавого спина убила игрока, обычный спин не запускаем.
    if (player.hp <= 0) {
        player.hp = 0;
        checkWinOrLose();
        return;
    }

    // Кровавый спин дает 500 кредитов, после чего выполняется
    // обычный спин с обычной стоимостью в 250 кредитов.
    player.balance += 500;
    currentState = GameState::PLAYING;

    spin();

    // Сохраняем в сообщении информацию о цене кровавого спина
    // и добавляем результат обычного спина.
    lastMessage = "Кровавый спин: -" + std::to_string(hpCost)
        + " HP, +500 кредитов. " + lastMessage;
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
    shieldBlocksRemaining = 0;
    locksActive = false;
    armorActive = false;
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