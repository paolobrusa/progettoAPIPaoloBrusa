cmake_minimum_required(VERSION 3.31)
project(progettoAPIPaoloBrusa C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -Werror -std=gnu11 -lm")

add_executable(progettoAPIPaoloBrusa main.c)
