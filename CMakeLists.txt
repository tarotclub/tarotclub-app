cmake_minimum_required(VERSION 3.14)
set(PROJECT_NAME tarotclub-app)
project(${PROJECT_NAME} LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TAROT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(IMGUI_DIR ${TAROT_ROOT}/libs/imgui)

find_package(OpenGL REQUIRED)

set(ICL_ROOT ${TAROT_ROOT}/libs/icl)
include(${ICL_ROOT}/icl.cmake)

add_subdirectory(${ICL_ROOT} icl)

# Creates C resources file from files in given directory
function(create_resources dir output)
    # Create empty output file
    file(WRITE ${output}.c "")
    file(WRITE ${output}.h "")
    # Collect input files
    file(GLOB bins ${dir}/*.png )
    file(APPEND ${output}.h "const char *embeddedfiles[] = {\n")
    # Iterate through input files
    foreach(bin ${bins})
        # Get short filename
        string(REGEX MATCH "([^/]+)$" filename ${bin})
        message("Files: " ${filename})
        # Replace filename spaces & extension separator for C compatibility
        string(REGEX REPLACE "\\.| |-" "_" filename ${filename})
        # Read hex data from file
        file(READ ${bin} filedata HEX)
        # Convert hex data for C compatibility
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
        # Append data to output file
        file(APPEND ${output}.c "const unsigned char ${filename}[] = {${filedata}};\nconst unsigned ${filename}_size = sizeof(${filename});\n")

        file(APPEND ${output}.h "\"${filename}\",\n")
    endforeach()

    file(APPEND ${output}.h "};\n")
endfunction()

create_resources(${TAROT_ROOT}/assets/avatars ${TAROT_ROOT}/assets/assets)

set(TAROT_CORE_SRCS
    ${TAROT_ROOT}/libs/tarotclub-core/ServerConfig.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/ClientConfig.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/TournamentConfig.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/DealGenerator.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/Deck.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/Card.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/Bot.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/Common.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/Engine.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/Protocol.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/PlayingTable.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/Score.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/System.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/Users.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/Lobby.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/BotManager.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/Server.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/PlayerContext.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/Network.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/Session.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/Identity.cpp
)

set(PROJECT_SOURCES
    src/main.cpp
    src/scenes.h
    src/gfx-engine.cpp
    src/gfx-engine.h
    src/board-scene.cpp
    src/board-scene.h
    src/board-hud.cpp
    src/board-hud.h
    src/i-board-event.h
    src/title-scene.cpp
    src/title-scene.h
    src/application.cpp
    src/application.h
    src/Embedded.cpp
    src/Embedded.h
    src/glad.c

    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp

)

#=========================================================================================================================
# COMPILER OPTIONS
#=========================================================================================================================
if (UNIX AND NOT APPLE)
    add_definitions(-DUSE_LINUX_OS)
endif()

if(WIN32)
    add_definitions(-DUSE_WINDOWS_OS)
endif()

add_definitions(-D_REENTRANT)
add_link_options(-static-libgcc -static-libstdc++)

if(WIN32)
    set(WIN32_LIBS

    )
endif()

if (UNIX AND NOT APPLE)
    set(SDL2_HEADERS
        /usr/include/SDL2
    )
endif()


#=========================================================================================================================
# SDL2
#=========================================================================================================================
if(WIN32)
    set(SDL2_HEADERS
        ${TAROT_ROOT}/libs/SDL2/i686-w64-mingw32/include
        ${TAROT_ROOT}/libs/SDL2/i686-w64-mingw32/include/SDL2
        ${TAROT_ROOT}/libs/SDL2_image/i686-w64-mingw32/include
    )

    set(SDL2_LIBS_DIR
        ${TAROT_ROOT}/libs/SDL2_image/i686-w64-mingw32/lib
        ${TAROT_ROOT}/libs/SDL2/i686-w64-mingw32/lib/
    )

    set(SDL2_LIBS
        mingw32
        SDL2main
        SDL2_image
        SDL2
    )
endif()

if (UNIX AND NOT APPLE)
    set(SDL2_HEADERS
        /usr/include/SDL2
    )

    set(SDL2_LIBS
        SDL2
        SDL2_image
    )
endif()


#=========================================================================================================================
# EXECUTABLE AND LINK
#=========================================================================================================================
add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${TAROT_CORE_SRCS}
)
# dl

if (UNIX)
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBS} OpenGL::GL  pthread icl)
elseif (WIN32)
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBS} OpenGL::GL  pthread icl
        ws2_32.lib psapi.lib setupapi.lib cfgmgr32.lib advapi32.lib iphlpapi.lib mswsock.lib)
#    target_link_options(${PROJECT_NAME} PUBLIC -Wl,-subsystem,windows)
endif()


target_link_directories(${PROJECT_NAME} PUBLIC ${SDL2_LIBS_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${SDL2_HEADERS}
    ${TAROT_ROOT}/libs/tarotclub-core
    ${TAROT_ROOT}/libs/asio
    ${TAROT_ROOT}/src
)

