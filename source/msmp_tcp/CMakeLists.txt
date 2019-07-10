set(library_name msmp_tcp)

add_library(${library_name} STATIC)

target_sources(${library_name}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/tcp_host.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tcp_reader.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tcp_writer.hpp
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tcp_host.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tcp_reader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tcp_writer.cpp
)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS system exception)

target_include_directories(${library_name}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/source
    PRIVATE
        ${Boost_INCLUDE_DIRS}
)

link_directories(${Boost_LIBRARY_DIRS})

target_link_libraries(${library_name}
    PUBLIC
        msmp

    PRIVATE
        ${Boost_LIBRARIES}
        pthread
)