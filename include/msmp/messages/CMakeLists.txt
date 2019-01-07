set(library_name "msmp_messages")

add_library(${library_name} INTERFACE)

add_subdirectory(control)

target_link_libraries(${library_name} msmp_control_messages)