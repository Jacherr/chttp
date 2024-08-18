file(GLOB_RECURSE SOURCES
    readline.h
    readline.c
    main.c
)

add_executable(chttp ${SOURCES})