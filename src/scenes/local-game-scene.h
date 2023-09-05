#ifndef LOCALGAMESCENE_H
#define LOCALGAMESCENE_H

#include "scenes.h"
#include "board-scene.h"

class LocalGameScene : public BoardScene
{
public:

    LocalGameScene(GfxSystem &system, IBoardEvent &event, PlayerContext &ctx, INetClient &net);

    virtual void OnActivate(SDL_Renderer *renderer, const std::map<std::string, Value> &args);

private:
    INetClient &mNet;
};

#endif // LOCALGAMESCENE_H
