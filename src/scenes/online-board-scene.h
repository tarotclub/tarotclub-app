 #ifndef ONLINE_BOARD_SCENE_H
#define ONLINE_BOARD_SCENE_H

#include "board-scene.h"
#include "i-application.h"

class OnlineBoardScene : public BoardScene
{
public:
    OnlineBoardScene(GfxSystem &system, IBoardEvent &event, PlayerContext &ctx, INetClient &net, IApplication &app);

    virtual void OnCreate(SDL_Renderer *renderer) override;
    virtual void OnActivate(SDL_Renderer *renderer, const std::map<std::string, Value> &args) override;
    virtual void Update(double deltaTime) override;
    virtual void Draw(SDL_Renderer *renderer) override;
    virtual void ProcessEvent(const SDL_Event &event) override;
    virtual void OnMessage(const std::map<std::string, Value> &message) override;
private:
    INetClient &mNet;
    IApplication &mApp;

    bool mWelcome = false;
    std::string mServerName;
    std::string mServerId;
    uint16_t mTcpPort = 4269U;

    void DrawWelcomeScreen();
};

#endif // ONLINE_BOARD_SCENE_H
