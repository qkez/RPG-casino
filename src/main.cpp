#include <SFML/Graphics.hpp>

int main()
{
    // Создаем окно 800x600
    sf::RenderWindow window(sf::VideoMode(800, 600), "RPG casino");

    // 1. Создаем кнопку (зеленый прямоугольник)
    sf::RectangleShape spinButton(sf::Vector2f(200.f, 50.f)); // Размер: 200 ширина, 50 высота
    spinButton.setFillColor(sf::Color::Green);
    spinButton.setPosition(300.f, 500.f); // Ставим внизу по центру

    // 2. Создаем один барабан 
    sf::RectangleShape reelBox(sf::Vector2f(100.f, 100.f)); // Размер 100 на 100
    reelBox.setFillColor(sf::Color(50, 50, 70)); // Темно-серый
    reelBox.setPosition(250.f, 300.f); // Ставим чуть выше кнопки

    // Добавим барабану голубую рамку в стиле киберпанк
    reelBox.setOutlineColor(sf::Color::Cyan);
    reelBox.setOutlineThickness(2.f);

    // Главный цикл игры (окно работает, пока его не закроют)
    while (window.isOpen())
    {
        // Обработка событий (крестик закрытия)
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Очищаем окно темно-синим цветом
        window.clear(sf::Color(15, 15, 25));

        // Отрисовываем объекты
        window.draw(reelBox);
        window.draw(spinButton);

        // Показываем результат на экране
        window.display();
    }

    return 0;
}