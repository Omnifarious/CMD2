cmake_minimum_required(VERSION "3.30")

cmake_minimum_required(VERSION 3.30)

add_subdirectory(concurrentqueue)
project ("Asynchronous Command Engine" VERSION 0.0.1 LANGUAGES CXX)

find_package(Threads REQUIRED)
#find_package(PkgConfig REQUIRED)
#pkg_check_modules(GMP REQUIRED gmp)
#pkg_check_modules(GMPXX REQUIRED gmpxx)

add_executable(test_basics src/test_basics.cpp)
target_compile_features(test_basics PUBLIC cxx_std_23)
target_link_libraries(test_basics PUBLIC Threads::Threads)
target_compile_options(test_basics PRIVATE -Wall -Wextra)

add_executable(test_timer src/test_timer.cpp)
target_compile_features(test_timer PUBLIC cxx_std_23)
target_link_libraries(test_timer PUBLIC Threads::Threads)
target_compile_options(test_timer PRIVATE -Wall -Wextra)

add_executable(cmd2_test CMD2/CMD2.cpp)
target_compile_features(cmd2_test PUBLIC cxx_std_23)
target_link_libraries(cmd2_test PUBLIC Threads::Threads)
target_compile_options(cmd2_test PRIVATE -Wall -Wextra)
