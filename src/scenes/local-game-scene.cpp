#include "local-game-scene.h"

LocalGameScene::LocalGameScene(GfxSystem &system, IBoardEvent &event, PlayerContext &ctx, INetClient &net)
    : BoardScene(system, event, ctx, SCENE_LOCAL_GAME)
    , mNet(net)
{

}

void LocalGameScene::OnActivate(SDL_Renderer *renderer, const std::map<std::string, Value> &args)
{
    BoardScene::OnActivate(renderer, args);
    mNet.ConnectToHost("127.0.0.1", 4269U);
}
