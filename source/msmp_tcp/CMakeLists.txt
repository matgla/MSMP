set(library_name msmp_tcp)

add_library(${library_name} STATIC)

target_sources(${library_name}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/msmp_tcp.hpp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/msmp_tcp.cpp
)

find_package(Boost REQUIRED COMPONENTS system)

target_include_directories(${library_name}
    PRIVATE
        ${Boost_INCLUDE_DIRS}
)

link_directories(${Boost_LIBRARY_DIRS})

target_link_libraries(${library_name}
    PRIVATE
        ${Boost_SYSTEM_LIBRARY}
)