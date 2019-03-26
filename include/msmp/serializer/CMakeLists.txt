set(library_name "msmp_serializer")

add_library(${library_name} INTERFACE)

target_sources(${library_name}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/deserializers.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/endian_type_traits.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/serializers.hpp
)

target_link_libraries(${library_name}
    INTERFACE
        gsl
        eul
)

target_compile_options(${library_name} INTERFACE
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a -fconcepts -Wall -Werror>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)