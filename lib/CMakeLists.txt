message("-- Adding libraries")
remove_definitions(-DBUILD_TESTS)
add_subdirectory(eul)
add_subdirectory(gsl)