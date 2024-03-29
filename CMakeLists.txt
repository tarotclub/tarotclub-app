# ==================================================================================================
# GLOBAL VARIABLES AND PACKAGES DISCOVERY
# ==================================================================================================
cmake_minimum_required(VERSION 3.5)

cmake_minimum_required(VERSION 3.14)
set(PROJECT_NAME tarotclub-app)
project(${PROJECT_NAME} LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TAROT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

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
    /usr/local/include
    /usr/local/include/SDL2
)

set(SDL2_LIBS_DIR
    /usr/local/lib
)

set(SDL2_LIBS
    SDL2
)

# ==================================================================================================
# TARGET AND LINK STUFF
# ==================================================================================================

# Activate the rpath in the executable: dynamic librairies will be searched in the relative ./lib
# directory from the executable path
set(CMAKE_INSTALL_RPATH "\$ORIGIN/lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

#add_compile_options(-fsanitize=undefined,address)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${TAROT_CORE_SRCS})
target_link_libraries(${PROJECT_NAME} boost_system ${SDL2_LIBS} OpenGL::GL  pthread ssl crypto dl mbedtls)
target_link_directories(${PROJECT_NAME} PUBLIC ${SDL2_LIBS_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_HEADERS} ${TAROT_INCLUDES})
target_compile_definitions(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:TAROT_DEBUG>")

# ==================================================================================================
# CPACK CONFIGURATION (INSTALLER)
# ==================================================================================================
# CPack doit être situé à la fin du CMake pour bénéficier de toutes les variables

set(BINARY_DESTINATION_DIR ".")
set(ASSETS_DESTINATION_DIR "./assets")
set(DOCS_DESTINATION_DIR ".")
set(MAN_DESTINATION_DIR ".")

set(CPACK_SYSTEM_NAME "x86_64")

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "D8S")
set(CPACK_PACKAGE_DESCRIPTION ${PROJECT_NAME})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_NAME})
set(CPACK_PACKAGE_HOMEPAGE_URL "https://www.d8s.eu")
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})
set(CPACK_PACKAGE_CHECKSUM "SHA256")

set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_MONOLITHIC_INSTALL YES)
set(CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME})
set(CPACK_STRIP_FILES YES)
set(CPACK_SET_DESTDIR true)
set(CPACK_INSTALL_PREFIX /) # suppress /usr/lib, default install prefix on Unix

# On dit ici à CMake que le type de package est une simple archive compressée
set(CPACK_GENERATOR "TXZ")

include(CPack)


# Transfer the executable name to "install" world
install(CODE "set(PROJECT_NAME \"${PROJECT_NAME}\")")

# Thank-you OpenTTD:
# Install all dependencies we can resolve, with the exception of ones that
# every Linux machine should have. This should make this build as generic
# as possible, where it runs on any machine with the same or newer libc
# than the one this is compiled with.
# We copy these libraries into lib/ folder, so they can be found on game
# startup. See comment in root CMakeLists.txt for how this works exactly.
install(CODE [[
    file(GET_RUNTIME_DEPENDENCIES
            RESOLVED_DEPENDENCIES_VAR DEPENDENCIES
            UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED_DEPENDENCIES
            EXECUTABLES ${PROJECT_NAME}
            POST_EXCLUDE_REGEXES "ld-linux|libc.so|libdl.so|libm.so|libgcc_s.so|libpthread.so|librt.so|libstdc...so")
    file(INSTALL
            DESTINATION "${CMAKE_INSTALL_PREFIX}/lib"
            FILES ${DEPENDENCIES}
            FOLLOW_SYMLINK_CHAIN)

    # This should not be possible, but error out when a dependency cannot
    # be resolved.
    list(LENGTH UNRESOLVED_DEPENDENCIES UNRESOLVED_LENGTH)
    if(${UNRESOLVED_LENGTH} GREATER 0)
        message(FATAL_ERROR "Unresolved dependencies: ${UNRESOLVED_DEPENDENCIES}")
    endif()
]])

# Copie de l'exécutable
install(TARGETS ${PROJECT_NAME}
        RUNTIME
            DESTINATION ${BINARY_DESTINATION_DIR}
            COMPONENT Runtime
        )

# Copie des ressources (images, sons, fichiers divers ...)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/cards/default DESTINATION ${ASSETS_DESTINATION_DIR}/cards)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/fonts DESTINATION ${ASSETS_DESTINATION_DIR})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/assets/hud DESTINATION ${ASSETS_DESTINATION_DIR})
