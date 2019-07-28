set(library_name "msmp_serializer")

add_library(${library_name} INTERFACE)

target_sources(${library_name}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/deserializers.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/endian_type_traits.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/endian.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/serializers.hpp
        ${CMAKE_CURRENT_SOURCE_DIR}/serialized_message.hpp
)

target_link_libraries(${library_name}
    INTERFACE
        gsl
        eul
)

target_compile_options(${library_name} INTERFACE
    $<$<COMPILE_LANGUAGE:CXX>:-std=c++2a -Wall -Werror -fPIC>
    $<$<COMPILE_LANGUAGE:C>:-std=c99 -Wall -Werror>
    $<$<CONFIG:DEBUG>:-Og -g>
    $<$<CONFIG:RELEASE>:-Os>
)

if (HOST_BIG_ENDIAN)
    set (host_byte_order "MSMP_HOST_BIG_ENDIAN")
elseif (HOST_LITTLE_ENDIAN)
    set (host_byte_order "MSMP_HOST_LITTLE_ENDIAN")
else ()
    message (STATUS "Endianness is not set! Using little endian by default!")
    set (host_byte_order "MSMP_HOST_LITTLE_ENDIAN")
endif ()

message (STATUS "Byte ordering: ${host_byte_order}")

target_compile_definitions(msmp_serializer INTERFACE
    ${host_byte_order}
)
