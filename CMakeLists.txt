cmake_minimum_required(VERSION 3.22)
project(ft_traceroute C)

set(CMAKE_C_STANDARD 11)

add_compile_options(-Wall -Werror -Wextra -g -O2)
include_directories(./inc ./src/libft/inc)
add_subdirectory(./src/libft)

add_executable(ft_traceroute src/main.c src/utils.c
        src/init.c
        src/print.c
        src/parsing.c)
target_link_libraries(ft_traceroute ft)