# TAROT_ROOT doit pointer vers la racine de tarotclub-app

set(IMGUI_DIR ${TAROT_ROOT}/libs/imgui)

set(ICL_ROOT ${TAROT_ROOT}/libs/icl)
include(${ICL_ROOT}/icl.cmake)

add_subdirectory(${ICL_ROOT} icl)

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
    ${TAROT_ROOT}/libs/tarotclub-core/TarotContext.cpp
)

set(PROJECT_SOURCES
    ${TAROT_ROOT}/src/main.cpp
    ${TAROT_ROOT}/src/scenes.h
    ${TAROT_ROOT}/src/gfx-engine.cpp
    ${TAROT_ROOT}/src/gfx-engine.h
    ${TAROT_ROOT}/src/board-scene.cpp
    ${TAROT_ROOT}/src/board-scene.h
    ${TAROT_ROOT}/src/board-hud.cpp
    ${TAROT_ROOT}/src/board-hud.h
    ${TAROT_ROOT}/src/i-board-event.h
    ${TAROT_ROOT}/src/title-scene.cpp
    ${TAROT_ROOT}/src/title-scene.h
    ${TAROT_ROOT}/src/story-mode-scene.cpp
    ${TAROT_ROOT}/src/story-mode-scene.h
    ${TAROT_ROOT}/src/application.cpp
    ${TAROT_ROOT}/src/application.h
    ${TAROT_ROOT}/src/Embedded.cpp
    ${TAROT_ROOT}/src/Embedded.h
    ${TAROT_ROOT}/src/glad.c
    ${TAROT_ROOT}/src/http-client.h
    ${TAROT_ROOT}/src/stb_image.h
    ${TAROT_ROOT}/src/SDL2_gfxPrimitives.c
    ${TAROT_ROOT}/src/SDL2_gfxPrimitives.h
    ${TAROT_ROOT}/src/SDL2_rotozoom.c
    ${TAROT_ROOT}/src/SDL2_rotozoom.h

    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp

)

set(TAROT_INCLUDES
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${TAROT_ROOT}/libs/tarotclub-core
    ${TAROT_ROOT}/libs/asio
    ${TAROT_ROOT}/src
)


