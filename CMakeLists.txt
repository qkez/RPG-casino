cmake_minimum_required(VERSION 3.20)
project(CyberpunkCasino)

set(CMAKE_CXX_STANDARD 17)

# Автоматически скачиваем и подключаем SFML
include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
)
FetchContent_MakeAvailable(SFML)

# Наш исполняемый файл
add_executable(game src/main.cpp)

# Подключаем библиотеки графики
target_link_libraries(game PRIVATE sfml-graphics sfml-window sfml-system)