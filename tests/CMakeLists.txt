add_executable(chess_tests
    dummy_test.cpp
    perft_test.cpp
    rules_test.cpp
    mate_solver_test.cpp
)

target_link_libraries(chess_tests PRIVATE doctest::doctest chess_engine)

include(${doctest_SOURCE_DIR}/scripts/cmake/doctest.cmake)
doctest_discover_tests(chess_tests)
