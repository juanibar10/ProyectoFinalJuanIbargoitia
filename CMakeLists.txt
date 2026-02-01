cmake_minimum_required(VERSION 3.16)
project(ProyectoFinalJuanIbargoitia)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.x
)
FetchContent_MakeAvailable(SFML)

FetchContent_Declare(
        JSON
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(JSON)

add_executable(ProyectoFinalJuanIbargoitia
        src/main.cpp
        src/Game.cpp
        src/Entities.cpp
        src/Level.cpp
        src/Collision.cpp
        src/UI.cpp
        src/Config.cpp
)

target_include_directories(ProyectoFinalJuanIbargoitia PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ProyectoFinalJuanIbargoitia PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
        nlohmann_json::nlohmann_json
)

if (WIN32)
    add_custom_command(TARGET ProyectoFinalJuanIbargoitia POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:sfml-graphics>
            $<TARGET_FILE:sfml-window>
            $<TARGET_FILE:sfml-system>
            $<TARGET_FILE_DIR:ProyectoFinalJuanIbargoitia>
    )
endif()

add_custom_command(TARGET ProyectoFinalJuanIbargoitia POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf
        $<TARGET_FILE_DIR:ProyectoFinalJuanIbargoitia>/assets

        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:ProyectoFinalJuanIbargoitia>/assets
)
