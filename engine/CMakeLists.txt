add_library(chess_engine STATIC
    src/position.cpp
    src/fen.cpp
    src/make_unmake.cpp
    src/attacks.cpp
    src/movegen.cpp
    src/perft.cpp
    src/rules.cpp
)

target_include_directories(chess_engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
