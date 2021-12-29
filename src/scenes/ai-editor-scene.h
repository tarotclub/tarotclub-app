#ifndef AI_EDITOR_SCENE_H
#define AI_EDITOR_SCENE_H


#include <vector>
#include <map>
#include <memory>
#include "gfx-engine.h"
#include "i-board-event.h"
#include "PlayerContext.h"
#include "board-hud.h"
#include "DataBase.h"
#include <imgui_node_editor.h>
#include "ai-editor-view.h"

namespace ed = ax::NodeEditor;


class AiEditorScene : public Scene
{
public:
    AiEditorScene(GfxSystem &system, IBoardEvent &event);
    ~AiEditorScene();

    virtual void OnCreate(SDL_Renderer *renderer) override;

    virtual void OnActivate(SDL_Renderer *renderer) override;

    virtual void Update(double deltaTime) override;

    virtual void Draw(SDL_Renderer *renderer) override;

    virtual void ProcessEvent(const SDL_Event &event);

private:
    IBoardEvent &mEvent;

    DataBase mDb;

    ed::EditorContext* m_Context = nullptr;

    Example mView;

    void ToolbarUI();
};

#endif // AI_EDITOR_SCENE_H
