add_executable(chess_cli
    main.cpp
)

target_link_libraries(chess_cli PRIVATE chess_engine)
