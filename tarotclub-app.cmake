# TAROT_ROOT doit pointer vers la racine de tarotclub-app

set(IMGUI_DIR ${TAROT_ROOT}/libs/imgui)
set(IMGUI_NODE_EDITOR_DIR ${TAROT_ROOT}/libs/imgui-node-editor)

set(MBEDTLS_DIR ${TAROT_ROOT}/libs/tarotclub-core/mbedtls-3.4.1)

#set(ICL_ROOT ${TAROT_ROOT}/libs/icl)
#include(${ICL_ROOT}/icl.cmake)

add_subdirectory(${MBEDTLS_DIR} mbdedtls)

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
    ${TAROT_ROOT}/libs/tarotclub-core/websocket-client.h
    ${TAROT_ROOT}/libs/tarotclub-core/websocket-client.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/JsonValue.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/Value.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/JsonReader.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/JsonWriter.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/Util.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/UniqueId.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/jsengine/duk_module_duktape.c
    ${TAROT_ROOT}/libs/tarotclub-core/jsengine/duktape.c
    ${TAROT_ROOT}/libs/tarotclub-core/jsengine/JSEngine.cpp
    ${TAROT_ROOT}/libs/tarotclub-core/zip/Zip.cpp
)

set(PROJECT_SOURCES
    ${TAROT_ROOT}/src/main.cpp
    ${TAROT_ROOT}/src/scenes.h
    ${TAROT_ROOT}/src/i-board-event.h
    ${TAROT_ROOT}/src/application.cpp
    ${TAROT_ROOT}/src/application.h
    ${TAROT_ROOT}/src/Embedded.cpp
    ${TAROT_ROOT}/src/Embedded.h
    ${TAROT_ROOT}/src/http-client.h
    ${TAROT_ROOT}/src/assets.cpp
    ${TAROT_ROOT}/src/Base64Util.cpp
    ${TAROT_ROOT}/src/Log.cpp
    ${TAROT_ROOT}/src/DataBase.cpp
    ${TAROT_ROOT}/src/sqlite3.c
    ${TAROT_ROOT}/src/Console.cpp

    # ----------------  GFX ENGINE  ----------------
    ${TAROT_ROOT}/src/gfx-engine/stb_image.h
    ${TAROT_ROOT}/src/gfx-engine/SDL2_gfxPrimitives.c
    ${TAROT_ROOT}/src/gfx-engine/SDL2_gfxPrimitives.h
    ${TAROT_ROOT}/src/gfx-engine/SDL2_rotozoom.c
    ${TAROT_ROOT}/src/gfx-engine/SDL2_rotozoom.h
    ${TAROT_ROOT}/src/gfx-engine/gfx-engine.cpp
    ${TAROT_ROOT}/src/gfx-engine/gfx-engine.h
    ${TAROT_ROOT}/src/gfx-engine/entity.h
    ${TAROT_ROOT}/src/gfx-engine/entity.cpp
    ${TAROT_ROOT}/src/gfx-engine/image.h
    ${TAROT_ROOT}/src/gfx-engine/image.cpp

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

    # ----------------  BOARD SCENE  ----------------
    ${TAROT_ROOT}/src/scenes/board-scene.cpp
    ${TAROT_ROOT}/src/scenes/board-scene.h
    ${TAROT_ROOT}/src/components/hud/board-hud.cpp
    ${TAROT_ROOT}/src/components/hud/board-hud.h

    # ----------------  ONLINE BOARD SCENE  ----------------
    ${TAROT_ROOT}/src/scenes/online-board-scene.cpp
    ${TAROT_ROOT}/src/scenes/online-board-scene.h

    # ----------------  LOCAL GAME SCENE  ----------------
    ${TAROT_ROOT}/src/scenes/local-game-scene.h
    ${TAROT_ROOT}/src/scenes/local-game-scene.cpp

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
    ${TAROT_ROOT}/libs/tarotclub-core/jsengine
    ${TAROT_ROOT}/libs/tarotclub-core/zip
    ${MBEDTLS_DIR}/include
#    ${TAROT_ROOT}/libs/asio
    ${TAROT_ROOT}/libs/nanosvg/src
    ${IMGUI_NODE_EDITOR_DIR}
    ${TAROT_ROOT}/src
    ${TAROT_ROOT}/src/scenes
    ${TAROT_ROOT}/src/components/ai-editor
    ${TAROT_ROOT}/src/components/hud
    ${TAROT_ROOT}/src/gfx-engine
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
