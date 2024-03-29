#ifndef APPLICATION_H
#define APPLICATION_H

#include "Log.h"
#include "BotManager.h"
#include "Console.h"
#include "PlayerContext.h"
#include "Lobby.h"
#include "Server.h"
#include "Network.h"
#include "ClientConfig.h"
#include "i-application.h"
#include "websocket-client.h"

#include <list>
#include <deque>

// Gfx
#include "title-scene.h"
#include "board-scene.h"

#define TAROTCLUB_APP_VERSION   "3.0.0"

class Application : public Observer<Log::Infos>
        , public INetClientEvent
        , public IBoardEvent
        , public IApplication
        , public WebSocketClient::IReadHandler
{
public:
    Application(INetClient &net);
    ~Application();

    bool Initialize();
    int Loop();
    void Stop();

    // From IApplication
    bool IsLogged() override { return mLogged; }
    bool IsInternetDetected() override;
    void SetLogged(const Identity &ident) override;
    std::string GetHost() const override;
    virtual void ConnectToServer(const std::string &serverId) override;
    virtual std::vector<ServerState> GetServers() override;

private:
    enum GameType {
        GAME_TYPE_LOCAL,
        GAME_TYPE_HOSTED,
        GAME_TYPE_ONLINE,
    };

    // From Observer
    virtual void Update(const Log::Infos &info) override;

    // From INetClientEvent (network layer --> application
    virtual bool Deliver(const Request &req) override ;
    virtual void Disconnected() override;

    // From IBoardEvent (board events ---> application ---> game server)
    virtual void ChatMessage(const std::string &msg) override;
    virtual void ExitGame() override;
    virtual void SendMyBid() override;
    virtual void SendMyCard(const Card &c) override;
    virtual void ConfigChanged() override;
    virtual void ClickOnBoard() override;

    INetClient &mNet;
    PlayerContext mCtx;
    GameType mGameType = GAME_TYPE_LOCAL;

    bool mLogged = false;

    ThreadQueue<Request> mNetRequests;
    ThreadQueue<GfxEngine::Message> mAsyncMessages; // messages qui peuvent venir d'autres threads

    std::vector<Reply> mNetReplies; // pas besoin d'une queue car l'envoi se réalise dans le thread applicatif (les sockets sont multi-threads

    GfxEngine mGfx;

    // ---------------------  WEB SOCKET TUNNEL WITH WEBSITE  ---------------------
    WebSocketClient mWsClient;
    std::thread mWsThread;

    std::mutex mMutex;
    std::vector<ServerState> mServers;

    // From WebSocketClient::IReadHandler
    virtual void OnWsData(const std::string &data) override;
    void RunWebSocket();


    void HandleRequest(const Request &req);
    void TimeoutEndOfTrickDelay();
    void ApplyOptions();
};

#endif // APPLICATION_H
