cmake_minimum_required(VERSION 3.22.0)

project(HomeWork-5.04.1)
set(PostgreSQL_ADDITIONAL_VERSION "15")
add_subdirectory(..\\libpqxx-7.7.5  libpqxx-build)

add_executable(HomeWork HomeWork-5.04.1.cpp ClientDB.h ClientDB.cpp)

target_compile_features(HomeWork PRIVATE cxx_std_17)
target_link_libraries(HomeWork pqxx)