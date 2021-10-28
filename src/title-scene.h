#ifndef TITLESCENE_H
#define TITLESCENE_H

#include "gfx-engine.h"
#include <iostream>

class TitleScene : public Scene
{
public:
    TitleScene(GfxSystem &system);

    virtual void OnCreate(SDL_Renderer *renderer) override;

    virtual void OnActivate(SDL_Renderer *renderer);

    virtual void Draw(SDL_Renderer *renderer) override;

private:
    void DrawGui();

private:
    ImFont* mMenuFont;
};


#endif // TITLESCENE_H
