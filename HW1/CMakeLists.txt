cmake_minimum_required(VERSION 3.10)
project(HW1 VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add typical warning and optimization flags (for GNU/Clang)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")

# 1) First executable, "multi"
add_executable(multi
    src/multi.cpp
    # add any additional sources for 'multi' here
)

add_executable(multi2
    src/multi2.cpp
    # add any additional sources for 'multi' here
)

add_executable(multi3
    src/multi3.cpp
    # add any additional sources for 'multi' here
)
add_executable(multi4
    src/multi4.cpp
    # add any additional sources for 'multi' here
)

# 2) Second executable, "single"
add_executable(single
    src/single.cpp
    # add any additional sources for 'single' here
)
add_executable(single2
    src/single2.cpp
    # add any additional sources for 'single' here
)
