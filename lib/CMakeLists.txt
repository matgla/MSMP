message("-- Adding libraries")
remove_definitions(-DBUILD_TESTS)
remove_definitions(-DBUILD_UT)

if (NOT TARGET eul)
    add_subdirectory(eul)
endif ()

if (NOT TARGET gsl)
    add_subdirectory(gsl)
endif ()

if (NOT TARGET crcpp)
    add_subdirectory(crcpp)
endif ()
