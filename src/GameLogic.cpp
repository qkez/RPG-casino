#include "GameLogic.h"
#include <iostream>
#include <random>

// Конструктор: задаем начальные значения
GameLogic::GameLogic() {
    playerHP = 100;
    playerBalance = 100;
    bossHP = 100;
    lastMessage = "Система инициализирована. Готов к спину!";
}

// Функция спина (кручения)
void GameLogic::spin() {
    // Проверка на кровавый спин (если нет денег)
    if (playerBalance < 10) {
        lastMessage = "Нет денег! Активирован КРОВАВЫЙ СПИН (Цена: 20 HP).";
        playerHP -= 20;
        if (playerHP <= 0) {
            lastMessage = "Вы умерли от потери крови. GAME OVER.";
            return;
        }
    }
    else {
        playerBalance -= 10; // Списываем 10 кредитов за спин
        lastMessage = "Барабаны крутятся...";
    }

    // Генератор случайных чисел (выпадет 0, 1 или 2)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, 2);
    int slot1 = dist(gen);
    int slot2 = dist(gen);
    int slot3 = dist(gen);

    // Проверка на выигрыш (если все 3 совпали)
    if (slot1 == slot2 && slot2 == slot3) {
        int damage = 25; // Наносимый урон
        bossHP -= damage;
        playerBalance += 50; // Выигрыш
        lastMessage = "ДЖЕКПОТ! Урон боссу: " + std::to_string(damage) + ". +50 кредитов!";
    }
    else {
        lastMessage = "Мимо! Босс атакует в ответ.";
        playerHP -= 10; // Босс бьет в ответ
    }

    // Проверка на смерть игрока или убийство босса
    if (bossHP <= 0) {
        bossHP = 0;
        lastMessage = "БОСС ПОВЕРЖЕН! ВЫ ПОБЕДИЛИ!";
    }
    else if (playerHP <= 0) {
        playerHP = 0;
        lastMessage = "ВЫ ПОГИБЛИ. GAME OVER.";
    }
}