#include "GameLogic.h"
#include <random>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <string>
#include <iomanip>
#include <cstdio>

// ==========================================
// КОНСТРУКТОР
// ==========================================
GameLogic::GameLogic() {
    loadGame(); // Пытаемся загрузить прогресс

    // Если сохранения нет и босс не создан, начинаем сначала
    if (bossIndex == 0 && currentBoss.name.empty()) {
        player = { 6666, 6666, 3427, 0, 100, false, 3 }; // Первый HOLD доступен сразу
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
    combatEvents.clear();
    // Если игра окончена или мы в другом режиме - не крутим
    if (currentState != GameState::PLAYING && currentState != GameState::BLOOD_SPIN) return;

    // Проверка на кровавый спин
    if (player.balance < spinCost) {
        const int bloodCost = static_cast<int>(player.maxHp * 0.05f);
        if (player.hp <= bloodCost) {
            currentState = GameState::ROBBERY;
            lastMessage = "Недостаточно HP для кровавого спина! Попробуйте ограбить казино.";
        } else {
            currentState = GameState::BLOOD_SPIN;
            lastMessage = "Мало денег! Кровавый спин будет стоить " + std::to_string(bloodCost) + " HP.";
        }
        return;
    }

    player.balance -= spinCost; // Списываем деньги
    player.spinsSinceHold++;    // Счетчик для захвата (Hold)

    // Крутим барабаны. Активный HACK гарантирует один выбранный символ.
    for (int i = 0; i < 3; ++i) {
        if (i != heldReelIndex) reels[i] = getRandomSymbol();
    }
    if (hackPending) {
        int hackedReel = rand() % 3;
        if (hackedReel == heldReelIndex) hackedReel = (hackedReel + 1) % 3;
        reels[hackedReel] = hackTarget;
        hackPending = false;
        combatEvents.push_back({"HACK: один барабан подменён на ДЖОКЕРА", true});
    }

    // --- МАГИЯ ПОДСЧЕТА СИМВОЛОВ ---
    // Считаем, сколько раз выпал каждый символ (индексы 0-9)
    int counts[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    counts[static_cast<int>(reels[0])]++;
    counts[static_cast<int>(reels[1])]++;
    counts[static_cast<int>(reels[2])]++;

    // --- ЛОГИКА ДЖОКЕРА (🃏) ---
    int jokerCount = counts[static_cast<int>(Symbol::JOKER)];
    bool jokerCoinMultiplier = false;
    if (jokerCount == 3) {
        currentBoss.hp = 1;
        lastMessage = "СУПЕР-ДЖЕКПОТ! ТРИ ДЖОКЕРА! Боссу остается 1 HP!";
    }
    else {
        if (jokerCount == 2) {
            for (int i = 0; i < static_cast<int>(Symbol::JOKER); i++) {
                if (counts[i] == 1) { counts[i] = 3; jokerCoinMultiplier = (i == static_cast<int>(Symbol::COIN)); break; }
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
            if (pairedSymbol!=-1) {
                counts[pairedSymbol]=3;
                jokerCoinMultiplier = (pairedSymbol == static_cast<int>(Symbol::COIN));
            }
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
            int dmg = static_cast<int>(currentBoss.maxHp * 0.60f);
            currentBoss.hp -= dmg;
            if (currentBoss.hp < 0) currentBoss.hp = 0;
            lastMessage = "ДЖЕКПОТ! ТРИ МЕЧА! Урон боссу: " + std::to_string(dmg);
            if (currentBoss.type == BossType::DRAGON && currentBoss.isBlocking)
                lastMessage += " КРИТИЧЕСКИЙ ПРОБОЙ: мечи пробили блок крыльев!";
        }
        else if (counts[static_cast<int>(Symbol::BOMB)] == 3) {
            int dmg = static_cast<int>(currentBoss.maxHp * 0.50f);
            currentBoss.hp -= dmg;
            if (currentBoss.hp < 0) currentBoss.hp = 0;
            lastMessage = "ДЖЕКПОТ! ТРИ БОМБЫ! Взрыв! Урон: " + std::to_string(dmg);
        }
        else if (counts[static_cast<int>(Symbol::MEDKIT)] == 3) {
            int healed = player.maxHp - player.hp;
            player.hp = player.maxHp;
            player.isHpRed = false;
            if (healed > 0) combatEvents.push_back({"+" + std::to_string(healed) + " HP", true});
            lastMessage = "ДЖЕКПОТ! ТРИ АПТЕЧКИ! Полное восстановление HP!";
        }
        else if (counts[static_cast<int>(Symbol::BATTERY)] == 3) {
            player.energy = player.maxEnergy;
            lastMessage = "ДЖЕКПОТ! ТРИ БАТАРЕЙКИ! Энергия восстановлена полностью!";
        }
        else if (counts[static_cast<int>(Symbol::COIN)] == 3) {
            player.balance *= (jokerCoinMultiplier ? 10 : 2);
            if (player.balance == 0) player.balance = (jokerCoinMultiplier ? 2500 : 500);
            lastMessage = jokerCoinMultiplier
                ? "ДЖЕКПОТ! ДВА ДЖОКЕРА + МОНЕТА! Кредиты умножены на 10! Теперь у вас: " + std::to_string(player.balance)
                : "ДЖЕКПОТ! ТРИ МОНЕТЫ! Кредиты удвоены! Теперь у вас: " + std::to_string(player.balance);
        }
        else if (counts[static_cast<int>(Symbol::DIAMOND)] == 3) {
            player.balance *= 10;
            if (player.balance == 0) player.balance = 2500;
            lastMessage = "ДЖЕКПОТ! ТРИ АЛМАЗА! Кредиты умножены на 10! Теперь у вас: " + std::to_string(player.balance);
        }
        else if (counts[static_cast<int>(Symbol::SKULL)] == 3) {
            player.hp = 1;
            player.isHpRed = true;
            lastMessage = "ДЖЕКПОТ! ТРИ ЧЕРЕПА! У вас остался 1 HP!";
        }
        else if (counts[static_cast<int>(Symbol::SHIELD)] == 3) {
            shieldBlocksRemaining += 3;
            lastMessage = "ДЖЕКПОТ! ТРИ ЩИТА! Следующие 3 атаки босса заблокированы!";
        }
        else if (counts[static_cast<int>(Symbol::LOCK)] == 3) {
            locksActive = true;
            armorActive = false;
            lastMessage = "ДЖЕКПОТ! ТРИ ЗАМКА! Все обычные атаки босса заблокированы до конца боя!";
        }

        // --- ОБЫЧНЫЕ ВЫПАДЕНИЯ (ДВОЙКИ) ---
        else if (counts[static_cast<int>(Symbol::SWORD)] == 2) {
            int dmg = static_cast<int>(currentBoss.maxHp * 0.10f);
            if (currentBoss.type == BossType::DRAGON && currentBoss.isBlocking) {
                dmg /= 2;
                lastMessage = "Дракончик закрылся крыльями! Урон мечей снижен вдвое: ";
            }
            else {
                lastMessage = "2 меча! Урон боссу: ";
            }
            currentBoss.hp -= dmg;
            if (currentBoss.hp < 0) currentBoss.hp = 0;
            lastMessage += std::to_string(dmg);
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
            int oldHp = player.hp;
            int heal = static_cast<int>(player.maxHp * 0.20f);
            player.hp += heal;
            if (player.hp > player.maxHp) player.hp = player.maxHp;
            int actualHeal = player.hp - oldHp;
            player.isHpRed = false;
            if (actualHeal > 0) combatEvents.push_back({"+" + std::to_string(actualHeal) + " HP", true});
            lastMessage = "2 аптечки! Лечение: +" + std::to_string(actualHeal) + " HP";
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
            combatEvents.push_back({"-" + std::to_string(dmg) + " HP (черепа)", false});
            lastMessage = "2 черепа! Вы потеряли " + std::to_string(dmg) + " HP.";
        }
        else {
            lastMessage = "Комбинация не собрана. Босс может атаковать!";
        }
    }

    // Блок крыльями защищает только от следующего спина игрока.
    // Тройной меч пробивает его, двойной наносит половину урона.
    if (currentBoss.type == BossType::DRAGON) currentBoss.isBlocking = false;

    bossTurn();       // Босс отвечает
    checkWinOrLose(); // Проверяем, не умер ли кто-то
    heldReelIndex = -1; // Захват действует только на один спин
}

// ==========================================
// ХОД БОССА (АТАКИ)
// ==========================================
void GameLogic::bossTurn() {
    if (currentBoss.hp <= 0) return;

    auto takeBossAttack = [this](int damage, bool isSpecial) {
        if (shieldBlocksRemaining > 0) {
            --shieldBlocksRemaining;
            combatEvents.push_back({"АТАКА ЗАБЛОКИРОВАНА ЩИТОМ", true});
            lastMessage += " Щит полностью заблокировал атаку босса!";
            return;
        }
        if (!isSpecial && locksActive) {
            combatEvents.push_back({"ОБЫЧНАЯ АТАКА ЗАБЛОКИРОВАНА ЗАМКОМ", true});
            lastMessage += " Замок полностью остановил обычную атаку босса!";
            return;
        }

        int finalDamage = damage;
        if (!isSpecial && armorActive) {
            finalDamage = static_cast<int>(damage * 0.70f);
            combatEvents.push_back({"БРОНЯ: урон снижен на 30%", true});
        }

        player.hp -= finalDamage;
        combatEvents.push_back({"-" + std::to_string(finalDamage) + " HP", false});
        lastMessage += isSpecial
            ? " | Босс нанес УЛЬТИМАТИВНЫЙ УДАР на " + std::to_string(finalDamage) + " урона!"
            : " | Босс нанес обычный удар на " + std::to_string(finalDamage) + " урона.";
    };

    // После начала зарядки босс пропускает два полных спина.
    // На следующем спине он выпускает ультимативную атаку.
    if (currentBoss.chargeTimer == 2 || currentBoss.chargeTimer == 1) {
        --currentBoss.chargeTimer;
        lastMessage += " | Босс заряжает ульту... Ход пропущен.";
        if (currentBoss.chargeTimer == 0) currentBoss.chargeTimer = -1;
        return;
    }
    if (currentBoss.chargeTimer == -1) {
        // Состояние готовности сбрасывается ДО атаки, чтобы ульта не могла повториться.
        currentBoss.chargeTimer = 0;
        lastMessage += " | Босс выпускает ульту!";
        takeBossAttack(currentBoss.specAtk, true);
        return;
    }

    // Случайный выбор: 45% — обычная атака, 25% — зарядка ульты,
    // 30% — босс выжидает и пропускает ход.
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> action(1, 100);
    const int choice = action(gen);

    if (choice <= 45) {
        // Уникальные способности заменяют обычную атаку, не меняя
        // общие вероятности атаки, зарядки ульты и пропуска хода.
        std::uniform_int_distribution<> specialChance(1, 100);
        const int specialRoll = specialChance(gen);

        if (currentBoss.type == BossType::DRAGON && specialRoll <= 45) {
            currentBoss.isBlocking = true;
            combatEvents.push_back({"ДРАКОНЧИК ЗАКРЫЛСЯ КРЫЛЬЯМИ: двойные мечи наносят половину урона, тройные пробивают блок", true});
            lastMessage += " | Дракончик расправил крылья и готовится блокировать следующий удар!";
        }
        else if (currentBoss.type == BossType::CUTE_FINAL && specialRoll <= 20 && player.balance > 0) {
            const int stolen = std::max(1, player.balance / 10);
            player.balance -= stolen;
            combatEvents.push_back({"НЯШКА УКРАЛА -" + std::to_string(stolen) + " КРЕДИТОВ", false});
            lastMessage += " | Няшка вместо атаки украла " + std::to_string(stolen) + " кредитов!";
        }
        else {
            takeBossAttack(currentBoss.atk, false);
        }
    }
    else if (choice <= 70) {
        currentBoss.chargeTimer = 2;
        lastMessage += " | Босс начал заряжать ульту! Следующие два спина он не атакует.";
    }
    else {
        lastMessage += " | Босс выжидает и пропускает атаку.";
    }
}
void GameLogic::doBloodSpin() {
    // Кровавый спин доступен только в специальном состоянии,
    // когда обычный спин уже нельзя оплатить.
    if (currentState != GameState::BLOOD_SPIN) return;

    int hpCost = static_cast<int>(player.maxHp * 0.05f);
    if (player.hp <= hpCost) {
        currentState = GameState::ROBBERY;
        lastMessage = "Недостаточно HP для кровавого спина! Попробуйте ограбить казино.";
        return;
    }
    combatEvents.clear();
    player.hp -= hpCost;
    player.isHpRed = true; // Шкала становится красной

    // Если цена кровавого спина убила игрока, обычный спин не запускаем.
    if (player.hp <= 0) {
        player.hp = 0;
        combatEvents.push_back({"-" + std::to_string(hpCost) + " HP (кровавый спин)", false});
        checkWinOrLose();
        return;
    }

    // Кровавый спин дает 500 кредитов, после чего выполняется
    // обычный спин с обычной стоимостью в 250 кредитов.
    player.balance += 500;
    currentState = GameState::PLAYING;

    spin();
    combatEvents.insert(combatEvents.begin(), {"-" + std::to_string(hpCost) + " HP (кровавый спин)", false});

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
        if (player.balance <= 0) {
            // Ограбление — последний шанс: игроку оставляется 1 HP,
            // чтобы нажатие кнопки не блокировалось проверкой смерти.
            player.hp = 1;
            player.isHpRed = true;
            currentState = GameState::ROBBERY;
            lastMessage = "Вы на грани! Остался 1 HP. Попробуйте ограбить казино!";
        }
        else {
            player.hp = 0;
            currentState = GameState::GAME_OVER;
            lastMessage = "Вы погибли. Игра окончена.";
        }
        // Смерть игрока имеет приоритет: победа над боссом не должна
        // автоматически восстанавливать погибшего игрока на следующем уровне.
        return;
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
    hackPending = false;
    hackTarget = Symbol::SWORD;
    heldReelIndex = -1;
    player.spinsSinceHold = 3; // Новый уровень начинается с доступного HOLD
    player.maxHp = static_cast<int>(player.maxHp * 1.1f); // +10% к макс HP
    player.hp = static_cast<int>(player.maxHp * 0.75f);  // Восстановление 75% HP
    player.isHpRed = false;

    if (bossIndex == 1) {
        currentBoss = { "Дракончик", BossType::DRAGON, 12345, 12345, 333, 1332, 0, false };
        lastMessage = "Босс повержен! Появился Дракончик!";
    }
    else if (bossIndex == 2) {
        currentBoss = { "Няшка", BossType::CUTE_FINAL, 22777, 22777, 466, 1864, 0, false };
        lastMessage = "Дракончик повержен! Финальный босс - Няшка!";
    }
    else {
        currentState = GameState::WIN;
        lastMessage = "ВЫ ПРОШЛИ ИГРУ!";
    }
}

void GameLogic::setBossForTesting(int index) {
    // Временное переключение боссов для тестирования без прохождения уровней.
    if (index < 0) index = 2;
    if (index > 2) index = 0;

    bossIndex = index;
    if (bossIndex == 0) {
        currentBoss = { "Дикий Волк", BossType::WOLF, 6000, 6000, 200, 800, 0, false };
    }
    else if (bossIndex == 1) {
        currentBoss = { "Дракончик", BossType::DRAGON, 12345, 12345, 333, 1332, 0, false };
    }
    else {
        currentBoss = { "Няшка", BossType::CUTE_FINAL, 22777, 22777, 466, 1864, 0, false };
    }

    // DEV-переход повторяет обычное повышение уровня: +10% max HP
    // за каждый пройденный уровень и восстановление до 50% HP.
    player.maxHp = 6666;
    for (int level = 0; level < bossIndex; ++level)
        player.maxHp = static_cast<int>(player.maxHp * 1.1f);
    player.hp = static_cast<int>(player.maxHp * 0.75f);
    player.isHpRed = false;

    currentState = GameState::PLAYING;
    heldReelIndex = -1;
    shieldBlocksRemaining = 0;
    locksActive = false;
    armorActive = false;
    hackPending = false;
    hackTarget = Symbol::JOKER;
    player.spinsSinceHold = 3;
    combatEvents.clear();
    lastMessage = "DEV: выбран босс " + std::to_string(bossIndex + 1) + " из 3.";
}

// ==========================================
// ЗАГЛУШКИ (Пока не реализованы)
// ==========================================
void GameLogic::holdReel(int index) {
    if (currentState != GameState::PLAYING || index < 0 || index >= 3) return;
    if (heldReelIndex != -1) {
        lastMessage = "Барабан уже удерживается. Захват действует до следующего спина.";
        return;
    }
    if (player.spinsSinceHold < 3) {
        lastMessage = "HOLD перезаряжается. Нужно сделать ещё " + std::to_string(3 - player.spinsSinceHold) + " спина.";
        return;
    }
    heldReelIndex = index;
    player.spinsSinceHold = 0;
    lastMessage = "HOLD: барабан " + std::to_string(index + 1) + " удержан. Выберите SPIN.";
}
void GameLogic::useHack(Symbol targetSymbol) {
    const int hackCost = 40;
    if (currentState != GameState::PLAYING) return;
    if (hackPending) {
        lastMessage = "HACK уже активирован и ждёт следующего спина.";
        return;
    }
    if (player.energy < hackCost) {
        lastMessage = "Для HACK нужно 40 энергии. Сейчас: " + std::to_string(player.energy) + ".";
        return;
    }
    player.energy -= hackCost;
    hackTarget = targetSymbol;
    hackPending = true;
    lastMessage = "HACK активирован! Следующий спин гарантированно даст 1 Джокера. -40 энергии.";
}
void GameLogic::prepareRobberyForTesting() {
    // DEV-кнопка принудительно открывает сценарий ограбления.
    player.hp = 1;
    player.balance = 0;
    player.isHpRed = true;
    currentState = GameState::ROBBERY;
    combatEvents.clear();
    lastMessage = "DEV: тест ограбления. Шанс успеха — 50%.";
}

void GameLogic::doRobbery() {
    if (currentState != GameState::ROBBERY || player.balance > 0) {
        lastMessage = "Ограбление сейчас недоступно.";
        return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> outcome(1, 100);

    if (outcome(gen) <= 50) {
        player.balance = std::max(1, robberyReward);
        currentState = GameState::PLAYING;
        combatEvents.push_back({"+" + std::to_string(player.balance) + " КРЕДИТОВ: ограбление удалось", true});
        lastMessage = "ОГРАБЛЕНИЕ УДАЛОСЬ! Вы унесли " + std::to_string(player.balance) + " кредитов.";
    }
    else {
        currentState = GameState::GAME_OVER;
        lastMessage = "ОГРАБЛЕНИЕ ПРОВАЛИЛОСЬ. ОХРАНА ПОЙМАЛА ВАС. GAME OVER.";
    }
}

void GameLogic::saveGame() {
    std::ofstream out("savegame.txt");
    if (!out) {
        lastMessage = "ОШИБКА: не удалось создать файл сохранения.";
        return;
    }
    out << player.hp << ' ' << player.maxHp << ' ' << player.balance << ' '
        << player.energy << ' ' << player.maxEnergy << ' ' << player.isHpRed << ' '
        << player.spinsSinceHold << '\n';
    out << bossIndex << ' ' << static_cast<int>(currentBoss.type) << ' '
        << currentBoss.hp << ' ' << currentBoss.maxHp << ' ' << currentBoss.atk << ' '
        << currentBoss.specAtk << ' ' << currentBoss.chargeTimer << ' '
        << currentBoss.isBlocking << '\n';
    out << std::quoted(currentBoss.name) << '\n';
    out << static_cast<int>(currentState) << ' ' << spinCost << ' ' << heldReelIndex << '\n';
    for (int i = 0; i < 3; ++i) out << static_cast<int>(reels[i]) << ' ';
    out << '\n' << shieldBlocksRemaining << ' ' << locksActive << ' ' << armorActive
        << ' ' << hackPending << ' ' << static_cast<int>(hackTarget) << '\n';
    out << std::quoted(lastMessage) << '\n';
    out << combatEvents.size() << '\n';
    for (const auto& event : combatEvents) out << event.positive << ' ' << std::quoted(event.text) << '\n';
    if (out.good()) lastMessage = "ИГРА СОХРАНЕНА! Файл savegame.txt создан.";
    else lastMessage = "ОШИБКА при записи сохранения.";
}

void GameLogic::loadGame() {
    std::ifstream in("savegame.txt");
    if (!in) return;
    Player loadedPlayer;
    Boss loadedBoss;
    int loadedBossIndex, bossType, state, loadedSpinCost, loadedHold, reelValues[3];
    int loadedShield, loadedHackTarget;
    bool loadedLocks, loadedArmor, loadedHack;
    std::string loadedMessage;
    size_t eventCount;
    if (!(in >> loadedPlayer.hp >> loadedPlayer.maxHp >> loadedPlayer.balance
          >> loadedPlayer.energy >> loadedPlayer.maxEnergy >> loadedPlayer.isHpRed
          >> loadedPlayer.spinsSinceHold)) return;
    if (!(in >> loadedBossIndex >> bossType >> loadedBoss.hp >> loadedBoss.maxHp
          >> loadedBoss.atk >> loadedBoss.specAtk >> loadedBoss.chargeTimer
          >> loadedBoss.isBlocking)) return;
    if (!(in >> std::quoted(loadedBoss.name))) return;
    // Миграция старых сохранений после переименования финального босса.
    if (loadedBoss.name == "Милашка") loadedBoss.name = "Няшка";
    loadedBoss.type = static_cast<BossType>(bossType);
    if (!(in >> state >> loadedSpinCost >> loadedHold)) return;
    for (int i = 0; i < 3; ++i) if (!(in >> reelValues[i])) return;
    if (!(in >> loadedShield >> loadedLocks >> loadedArmor >> loadedHack >> loadedHackTarget)) return;
    if (!(in >> std::quoted(loadedMessage) >> eventCount)) return;
    // Не читаем повреждённое сохранение с некорректным количеством событий.
    if (eventCount > 100) return;
    std::vector<CombatEvent> loadedEvents;
    for (size_t i = 0; i < eventCount; ++i) {
        CombatEvent event;
        if (!(in >> event.positive >> std::quoted(event.text))) return;
        loadedEvents.push_back(event);
    }
    // Проверяем диапазоны до присваивания: повреждённый файл не должен
    // создавать недопустимые индексы барабанов или состояния игры.
    if (loadedPlayer.maxHp <= 0 || loadedPlayer.hp < 0 || loadedPlayer.hp > loadedPlayer.maxHp ||
        loadedPlayer.balance < 0 || loadedPlayer.maxEnergy <= 0 ||
        loadedPlayer.energy < 0 || loadedPlayer.energy > loadedPlayer.maxEnergy ||
        loadedPlayer.spinsSinceHold < 0 ||
        loadedBossIndex < 0 || loadedBossIndex > 3 ||
        bossType < 0 || bossType > 2 ||
        loadedBoss.maxHp <= 0 || loadedBoss.hp < 0 || loadedBoss.hp > loadedBoss.maxHp ||
        loadedBoss.atk < 0 || loadedBoss.specAtk < 0 ||
        loadedBoss.chargeTimer < -1 || loadedBoss.chargeTimer > 2 ||
        state < 0 || state > 4 || loadedSpinCost <= 0 ||
        loadedHold < -1 || loadedHold > 2 || loadedShield < 0 ||
        loadedHackTarget < 0 || loadedHackTarget > 9 || loadedBoss.name.empty()) return;
    for (int i = 0; i < 3; ++i) {
        if (reelValues[i] < 0 || reelValues[i] > 9) return;
    }

    player = loadedPlayer;
    currentBoss = loadedBoss;
    bossIndex = loadedBossIndex;
    currentState = static_cast<GameState>(state);
    spinCost = loadedSpinCost;
    heldReelIndex = loadedHold;
    for (int i = 0; i < 3; ++i) reels[i] = static_cast<Symbol>(reelValues[i]);
    shieldBlocksRemaining = loadedShield;
    locksActive = loadedLocks;
    armorActive = loadedArmor;
    hackPending = loadedHack;
    hackTarget = static_cast<Symbol>(loadedHackTarget);
    lastMessage = loadedMessage;
    combatEvents = loadedEvents;
}

// Сброс прогресса и создание новой игры
void GameLogic::restartGame() {
    player = { 6666, 6666, 3427, 0, 100, false, 3 };
    currentBoss = { "Дикий Волк", BossType::WOLF, 6000, 6000, 200, 800, 0, false };
    bossIndex = 0;
    reels[0] = Symbol::SWORD;
    reels[1] = Symbol::COIN;
    reels[2] = Symbol::SKULL;
    currentState = GameState::PLAYING;
    lastMessage = "Новая игра! Сделайте первый спин.";
    combatEvents.clear();
    spinCost = 250;
    heldReelIndex = -1;
    shieldBlocksRemaining = 0;
    locksActive = false;
    armorActive = false;
    hackPending = false;
    hackTarget = Symbol::JOKER;
    std::remove("savegame.txt");
}
