cmake_minimum_required(VERSION 3.14)
set(PROJECT_NAME tarotclub-app)
project(${PROJECT_NAME} LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TAROT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

find_package(OpenGL REQUIRED)

include(${TAROT_ROOT}/tarotclub-app.cmake)

#=========================================================================================================================
# COMPILER OPTIONS
#=========================================================================================================================
add_definitions(-DUSE_LINUX_OS)
add_definitions(-D_REENTRANT)
add_link_options(-static-libgcc -static-libstdc++)

#=========================================================================================================================
# SDL2
#=========================================================================================================================
set(SDL2_HEADERS
    /usr/include/SDL2
)

set(SDL2_LIBS
    SDL2
)


#=========================================================================================================================
# SDL2
#=========================================================================================================================
# Thank you OpenTTD for this cmake hint
# If we are packaging dependencies, we do two things:
# 1) set the RPATH to include $ORIGIN/lib; $ORIGIN (that literal string)
#    is a Linux indicator for "path where application is". In CMake, we
#    have to do this before add_executable() is executed.
# 2) copy the libraries that we compile against to the "lib" folder.
#    This is done in InstallAndPackage.cmake.
set(CMAKE_INSTALL_RPATH "\$ORIGIN/lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

#=========================================================================================================================
# EXECUTABLE AND LINK
#=========================================================================================================================
add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${TAROT_CORE_SRCS}
)

target_link_libraries(${PROJECT_NAME} ${SDL2_LIBS} OpenGL::GL  pthread icl dl)
target_link_directories(${PROJECT_NAME} PUBLIC ${SDL2_LIBS_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_HEADERS} ${TAROT_INCLUDES})

