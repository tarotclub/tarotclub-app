#include "story-mode-scene.h"

StoryModeScene::StoryModeScene(GfxSystem &system, IBoardEvent &event)
    : Scene(system)
    , mEvent(event)
{
    mMap = std::make_shared<FranceMap>(GetSystem());

    AddEntity(mMap);
}



void StoryModeScene::OnCreate(SDL_Renderer *renderer)
{
    Scene::OnCreate(renderer);
}

void StoryModeScene::OnActivate(SDL_Renderer *renderer)
{
    Scene::OnActivate(renderer);
}

void StoryModeScene::Draw(SDL_Renderer *renderer)
{
    Scene::Draw(renderer);
}

void StoryModeScene::ProcessEvent(const SDL_Event &event)
{
    Scene::ProcessEvent(event);

}

void FranceMap::Draw(SDL_Renderer *renderer)
{
    SetPos(mOffsetX, mOffsetY);
    SetScale(mZoom, mZoom);
    DrawEx(renderer, GetX(), GetY());
}

void FranceMap::ProcessEvent(const SDL_Event &event)
{
    int xMouse, yMouse;
    uint32_t mask = SDL_GetMouseState(&xMouse,&yMouse);
    if(event.type == SDL_MOUSEWHEEL)
    {
        if(event.wheel.y > 0) // scroll up
        {
            // Put code for handling "scroll up" here!
            mZoom += 0.1;
            mOffsetX = -(xMouse * 0.1);
            mOffsetY = -(yMouse * 0.1);
        }
        else if(event.wheel.y < 0) // scroll down
        {
            // Put code for handling "scroll down" here!
            mZoom -= 0.1;
            mOffsetX = (xMouse * 0.1);
            mOffsetY = (yMouse * 0.1);
        }
    }

    if (event.type == SDL_MOUSEMOTION)
    {
        if ((mask & SDL_BUTTON_LMASK) == SDL_BUTTON_LMASK)
        {
            mOffsetX += event.motion.xrel;
            mOffsetY += event.motion.yrel;
        }
    }
}
