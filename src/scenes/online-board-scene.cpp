#include "online-board-scene.h"
#include "scenes.h"

OnlineBoardScene::OnlineBoardScene(GfxSystem &system, IBoardEvent &event, PlayerContext &ctx, INetClient &net, IApplication &app)
    : BoardScene(system, event, ctx, SCENE_ONLINE_GAME)
    , mNet(net)
    , mApp(app)
{

}

void OnlineBoardScene::OnCreate(SDL_Renderer *renderer)
{
   BoardScene::OnCreate(renderer);
}

void OnlineBoardScene::OnActivate(SDL_Renderer *renderer, const std::map<std::string, Value> &args)
{
    BoardScene::OnActivate(renderer, args);

    if (args.count("name"))
    {
        Value n = args.at("name");
        mServerName = n.GetString();
    }

    if (args.count("id"))
    {
        Value n = args.at("id");
        mServerId = n.GetString();
    }

    if (args.count("tcp_port"))
    {
        Value p = args.at("tcp_port");
        mTcpPort = p.GetInteger();
    }

    mApp.ConnectToServer(mServerId);

    mWelcome = true;
}

void OnlineBoardScene::Update(double deltaTime)
{
    BoardScene::Update(deltaTime);
}

void OnlineBoardScene::Draw(SDL_Renderer *renderer)
{
    BoardScene::Draw(renderer);

    DrawWelcomeScreen();
}

void OnlineBoardScene::ProcessEvent(const SDL_Event &event)
{
    BoardScene::ProcessEvent(event);
}

void OnlineBoardScene::OnMessage(const std::map<std::string, Value> &message)
{
    if (message.size() > 0)
    {
        if (message.count("event"))
        {
            std::string ev = message.at("event").GetString();

            if (ev == "accessGranted")
            {
                // Connect to remote server
                TLogNetwork("[ONLINE] Access granted, connect to game server");
                mNet.ConnectToHost(mApp.GetHost(), mTcpPort);
            }
        }

    }
}

void OnlineBoardScene::DrawWelcomeScreen()
{
    ImGui::SetNextWindowPos(ImVec2(200, 120));
    ImGui::SetNextWindowSize(ImVec2(250, 200));

    ImGuiWindowFlags window_flags = 0
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_NoSavedSettings
        ;
    ImGui::Begin("WelcomeScreen", NULL, window_flags);
    ImGui::Text("Bienvenue sur le serveur ");
    ImGui::Separator();

    ImGui::End();
}

