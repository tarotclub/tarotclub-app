#ifndef TITLESCENE_H
#define TITLESCENE_H

#include "gfx-engine.h"
#include <iostream>

class TitleScene : public Scene
{
public:
    TitleScene(GfxSystem &system, const std::string &version);

    virtual void OnCreate(SDL_Renderer *renderer) override;

    virtual void OnActivate(SDL_Renderer *renderer);

    virtual void Draw(SDL_Renderer *renderer) override;

private:
    ImFont* mMenuFont;
    std::string mVersion;

    void DrawInfoMenu();
    void DrawGui();

};


#endif // TITLESCENE_H
