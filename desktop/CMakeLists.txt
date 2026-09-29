add_executable(chess_gui
    main.cpp
)

target_link_libraries(chess_gui PRIVATE chess_engine SFML::Graphics SFML::Window SFML::System)
