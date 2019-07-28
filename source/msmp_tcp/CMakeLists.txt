set(library_name msmp_tcp)

add_library(${library_name} STATIC)

target_sources(${library_name}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/tcp_host.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tcp_reader.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tcp_writer.hpp
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/boost_exception_handler.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tcp_host.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tcp_reader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tcp_writer.cpp
)

if (NOT BUILD_SHARED)
    set(Boost_USE_STATIC_LIBS ON)
endif ()

find_package(Boost REQUIRED COMPONENTS system exception thread)

target_include_directories(${library_name}
    PUBLIC
        ${PROJECT_SOURCE_DIR}/source
    PRIVATE
        ${Boost_INCLUDE_DIRS}
)

link_directories(${Boost_LIBRARY_DIRS})

add_compile_options(-fexceptions)

target_link_libraries(${library_name}
    PUBLIC
        msmp

    PRIVATE
        Boost::exception
        Boost::thread
        Boost::system
)