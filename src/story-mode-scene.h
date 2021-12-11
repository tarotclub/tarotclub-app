#ifndef STORYMODESCENE_H
#define STORYMODESCENE_H


#include <vector>
#include <map>
#include <memory>
#include "gfx-engine.h"
#include "i-board-event.h"
#include "PlayerContext.h"
#include "board-hud.h"

class FranceMap : public Image
{

public:
    FranceMap(GfxSystem &s)
        : Image(s, "assets/france.png")
    {
        SetPos(0, 0);
    }

    virtual void Draw(SDL_Renderer *renderer) override;

    virtual void ProcessEvent(const SDL_Event &event) override;

private:
    float mZoom;
    float mOffsetX;
    float mOffsetY;
};

class StoryModeScene : public Scene
{
public:
    StoryModeScene(GfxSystem &system, IBoardEvent &event);

    virtual void OnCreate(SDL_Renderer *renderer) override;

    virtual void OnActivate(SDL_Renderer *renderer) override;

    virtual void Draw(SDL_Renderer *renderer) override;

    virtual void ProcessEvent(const SDL_Event &event);

private:
    IBoardEvent &mEvent;

    std::shared_ptr<Image> mMap;
};

#endif // STORYMODESCENE_H
