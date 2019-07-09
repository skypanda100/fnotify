cmake_minimum_required(VERSION 2.6)
project(fnotify)

if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set (CMAKE_C_FLAGS "-std=gnu99 ${CMAKE_C_FLAGS}")
    endif ()
else ()
    set (CMAKE_C_STANDARD 99)
endif ()

set(SOURCE_FILES fnotify.c notify.h notify.c def.h watch.h watch.c config.h config.c)
add_executable(fnotify ${SOURCE_FILES})

INSTALL(TARGETS fnotify
        RUNTIME DESTINATION bin
        )