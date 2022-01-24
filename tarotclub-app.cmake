# TAROT_ROOT doit pointer vers la racine de tarotclub-app

set(IMGUI_DIR ${TAROT_ROOT}/libs/imgui)
set(IMGUI_NODE_EDITOR_DIR ${TAROT_ROOT}/libs/imgui-node-editor)

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

    ${TAROT_ROOT}/src/application.cpp
    ${TAROT_ROOT}/src/application.h
    ${TAROT_ROOT}/src/Embedded.cpp
    ${TAROT_ROOT}/src/Embedded.h
    ${TAROT_ROOT}/src/http-client.h
    ${TAROT_ROOT}/src/websocket-client.h
    ${TAROT_ROOT}/src/websocket-client.cpp
    ${TAROT_ROOT}/src/stb_image.h
    ${TAROT_ROOT}/src/SDL2_gfxPrimitives.c
    ${TAROT_ROOT}/src/SDL2_gfxPrimitives.h
    ${TAROT_ROOT}/src/SDL2_rotozoom.c
    ${TAROT_ROOT}/src/SDL2_rotozoom.h
    ${TAROT_ROOT}/src/assets.cpp

    # ----------------  SCENE STORY  ----------------
    ${TAROT_ROOT}/src/story-mode-scene.cpp
    ${TAROT_ROOT}/src/story-mode-scene.h

    # ----------------  AI-EDITOR SCENE  ----------------
    ${TAROT_ROOT}/src/scenes/ai-editor-scene.cpp
    ${TAROT_ROOT}/src/scenes/ai-editor-scene.h
    ${TAROT_ROOT}/src/components/ai-editor/ai-editor-view.cpp
    ${TAROT_ROOT}/src/components/ai-editor/ai-editor-view.h

    # ----------------  TITLE SCENE  ----------------
    ${TAROT_ROOT}/src/scenes/title-scene.cpp
    ${TAROT_ROOT}/src/scenes/title-scene.h

    # ----------------  IMGUI  ----------------
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer.cpp

    ${IMGUI_NODE_EDITOR_DIR}/imgui_canvas.cpp
    ${IMGUI_NODE_EDITOR_DIR}/imgui_node_editor_api.cpp
    ${IMGUI_NODE_EDITOR_DIR}/imgui_node_editor.cpp
    ${IMGUI_NODE_EDITOR_DIR}/crude_json.cpp
)

set(TAROT_INCLUDES
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${TAROT_ROOT}/libs/tarotclub-core
#    ${TAROT_ROOT}/libs/asio
    ${TAROT_ROOT}/libs/nanosvg/src
    ${IMGUI_NODE_EDITOR_DIR}
    ${TAROT_ROOT}/src
    ${TAROT_ROOT}/src/scenes
    ${TAROT_ROOT}/src/components/ai-editor
)


#Creates C resources file from files in given directory
function(create_resources dir output)
    #Create empty output file
    file(WRITE ${output}.c "")
    file(WRITE ${output}.h "")
    #Collect input files
    file(GLOB bins ${dir}/*.png )
    file(APPEND ${output}.h "const char *embeddedfiles[] = {\n")
    #Iterate through input files
    foreach(bin ${bins})
        #Get short filename
        string(REGEX MATCH "([^/]+)$" filename ${bin})
        message("Files: " ${filename})
        #Replace filename spaces & extension separator for C compatibility
        string(REGEX REPLACE "\\.| |-" "_" filename ${filename})
        #Read hex data from file
        file(READ ${bin} filedata HEX)
        #Convert hex data for C compatibility
        string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
        #Append data to output file
        file(APPEND ${output}.c "const unsigned char ${filename}[] = {${filedata}};\nconst unsigned ${filename}_size = sizeof(${filename});\n")

        file(APPEND ${output}.h "\"${filename}\",\n")
    endforeach()

    file(APPEND ${output}.h "};\n")
endfunction()
