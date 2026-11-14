#pragma once
feature/graphics-ui
#include <string>

class GameLogic {
public:
    GameLogic(); // Конструктор 
    // Данные игры
    int playerHP;
    int playerBalance;
    int bossHP;
    std::string lastMessage; // Текст, который будем выводить на экран

    // Действия
    void spin(); // Функция для кручения барабанов
};

main
