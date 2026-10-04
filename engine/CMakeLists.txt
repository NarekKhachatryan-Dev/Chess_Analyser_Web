add_library(chess_engine STATIC
    src/position.cpp
    src/fen.cpp
    src/make_unmake.cpp
    src/attacks.cpp
    src/movegen.cpp
    src/perft.cpp
    src/rules.cpp
    src/mate_solver.cpp
    src/c_api.cpp
)

target_include_directories(chess_engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(EMSCRIPTEN)
    add_executable(chess_engine_wasm
        src/c_api.cpp
        src/position.cpp
        src/fen.cpp
        src/make_unmake.cpp
        src/attacks.cpp
        src/movegen.cpp
        src/perft.cpp
        src/rules.cpp
        src/mate_solver.cpp
    )
    target_include_directories(chess_engine_wasm PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_options(chess_engine_wasm PRIVATE
        "-sMODULARIZE=1"
        "-sEXPORT_ES6=1"
        "-sENVIRONMENT=web,node"
        "-sALLOW_MEMORY_GROWTH=0"
        "-sINITIAL_MEMORY=67108864"
        "-sSTACK_SIZE=1048576"
        "-sEXPORTED_FUNCTIONS=['_solve_depth','_validate','_perft','_free_json']"
        "-sEXPORTED_RUNTIME_METHODS=['cwrap','UTF8ToString']"
    )
    set_target_properties(chess_engine_wasm PROPERTIES
        OUTPUT_NAME "chess_engine"
        SUFFIX ".js"
    )
endif()
