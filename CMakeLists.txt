cmake_minimum_required(VERSION "3.30")

cmake_minimum_required(VERSION 3.30)

add_subdirectory(concurrentqueue)
project (transistor_clicker VERSION 0.0.1 LANGUAGES CXX)

find_package(Threads REQUIRED)
#find_package(PkgConfig REQUIRED)
#pkg_check_modules(GMP REQUIRED gmp)
#pkg_check_modules(GMPXX REQUIRED gmpxx)

add_executable(test_harness src/test_harness.cpp src/Context.cpp)
target_compile_features(test_harness PUBLIC cxx_std_23)
target_link_libraries(test_harness PUBLIC Threads::Threads)
target_compile_options(test_harness PRIVATE -Wall -Wextra)
