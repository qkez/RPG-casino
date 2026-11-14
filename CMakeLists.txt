cmake_minimum_required(VERSION 3.20)
project(CyberpunkCasino)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(SFML
GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x
)
FetchContent_MakeAvailable(SFML)

add_executable(game 
    src/main.cpp
    src/GameLogic.cpp
    src/UI.cpp
)

target_link_libraries(game PRIVATE sfml-graphics sfml-window sfml-system)
if(WIN32)
    add_custom_command(TARGET game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_RUNTIME_DLLS:game>
        $<TARGET_FILE_DIR:game>
        COMMAND_EXPAND_LISTS
    )
endif()