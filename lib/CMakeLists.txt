message("-- Adding libraries")
set(BUILD_TESTS OFF)
set(BUILD_UT OFF)

if (NOT TARGET eul)
    add_subdirectory(eul)
endif ()

if (NOT TARGET gsl)
    add_subdirectory(gsl)
endif ()

if (NOT TARGET crcpp)
    add_subdirectory(crcpp)
endif ()
