cmake_minimum_required(VERSION 3.22)
project(ft_traceroute C)

set(CMAKE_C_STANDARD 11)

add_compile_options(-Wall -Werror -Wextra -g)

add_subdirectory(./src/libft)

add_executable(ft_traceroute src/main.c)
target_link_libraries(ft_traceroute ft)