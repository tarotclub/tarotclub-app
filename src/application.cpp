#include "application.h"
#include "Embedded.h"
#include "scenes.h"
#include "System.h"
#include "story-mode-scene.h"
#include "ai-editor-scene.h"
#include "online-board-scene.h"
#include "local-game-scene.h"
#include "JsonReader.h"

Application::Application(INetClient &net)
    : Observer(Log::All)
    , mNet(net)
    , mWsClient(*this)
{

    mCtx.mOptions.identity.username = "Belegar";

    ClientConfig::Load(mCtx.mOptions, System::HomePath() + ClientConfig::DefaultConfigFile());
}

Application::~Application()
{
    mWsClient.Close();
    if (mWsThread.joinable())
    {
        mWsThread.join();
    }
    Stop();
}

bool Application::Initialize()
{

    bool gfxInit = mGfx.Initialize("TarotClub");

    mWsThread = std::thread(&Application::RunWebSocket, this);

//    mGfx.AddFont("menu", "assets/fonts/roboto.ttf", 20);
//    mGfx.AddFont("icons", "assets/fonts/MaterialIcons-Regular.ttf", 20);

    return gfxInit;
}

void Application::RunWebSocket()
{
    bool quit = false;
    while(!quit)
    {
        mWsClient.Run(GetHost(), "9998");
        // Ah on a quitté, pourquoi ?
        WebSocketClient::State state = mWsClient.GetState();
        if (state == WebSocketClient::STATE_NO_ERROR)
        {
            // pas d'erreur particulière, on quitte pour de bon
            quit = true;
        }
        else
        {
            // Quelque chose s'est mal passé, on se reconnecte au serveur
            std::this_thread::sleep_for(std::chrono::seconds(2));
            TLogError("[WEBSOCKET] Error code: " + std::to_string(state) + " restarting connection...");
        }
    }

}

void Application::Update(const Log::Infos &info)
{
    std::cout << info.ToString() << std::endl;
}

int Application::Loop()
{
    // Initialize all scenes and stuff
    mGfx.AddScene(std::make_shared<TitleScene>(mGfx, *this, TAROTCLUB_APP_VERSION), SCENE_TITLE);
    mGfx.AddScene(std::make_shared<LocalGameScene>(mGfx, *this, mCtx, mNet), SCENE_LOCAL_GAME);
    mGfx.AddScene(std::make_shared<OnlineBoardScene>(mGfx, *this, mCtx, mNet, *this), SCENE_ONLINE_GAME);
    mGfx.AddScene(std::make_shared<StoryModeScene>(mGfx, *this), SCENE_STORY_MODE);
    mGfx.AddScene(std::make_shared<AiEditorScene>(mGfx, *this), SCENE_AI_EDITOR);

    mGfx.SwitchSceneTo(SCENE_TITLE); // First scene

    mGfx.Warmup();

    mCtx.mEndOfTrickTimer.Register(std::bind(&Application::TimeoutEndOfTrickDelay, this));

    bool loop = true;

    Request req;
    GfxEngine::Message msg;

    ApplyOptions();

    // Boucle principale de l'application qui se charge de l'affichage et des données du joueur humain
    // Ici on tourne dans un thread, si d'autres threads veulent communiquer avec nous, il faut passer par une queue de messages
    // ainsi, on n'a quasiment aucun "lock" trop long
    while (loop)
    {
        // 1. On récupère et on décode les trames réseau
        if (mNetRequests.TryPop(req))
        {
            // Il est important de décoder dans le même thread que la boucle d'affichage
            // de cette façon, on n'a pas d'accès concurrentiel à gérer (mutex)
            HandleRequest(req);
        }

        // 2. On avance des éventuels timings / delay / timeouts
        mCtx.Update();

        // 3. On récupère les éventuels autres messages en provenance d'autres threads
        //    et à destination de la scène courante
        msg.clear();
        mAsyncMessages.TryPop(msg);

        // 4. On traite les entrées (clavier/souris) et on affiche le jeu
        if (mGfx.Process(msg) == SCENE_EXIT)
        {
            loop = false;
        }

        // 5. On envoie des trames réseau s'il y en a
        if (mNetReplies.size() > 0)
        {
            mNet.Send(mCtx.mMyself.uuid, mNetReplies);
            mNetReplies.clear();
        }
    }

    return 0;
}

void Application::TimeoutEndOfTrickDelay()
{
    // Même code que si on clique sur la table de jeu
    ClickOnBoard();
}

void Application::Stop()
{

}

bool Application::IsInternetDetected()
{
    return mWsClient.IsConnected();
}

void Application::SetLogged(const Identity &ident)
{
    mLogged = true;
    mCtx.mOptions.identity = ident;
}

std::string Application::GetHost() const
{
#ifdef TAROT_DEBUG
    static const std::string TAROTCLUB_HOST = "127.0.0.1";
#else
    static const std::string TAROTCLUB_HOST = "tarotclub.fr";
#endif
    return TAROTCLUB_HOST;
}

void Application::ConnectToServer(const std::string &serverId)
{
    if (mWsClient.IsConnected() && mLogged)
    {
        JsonObject order;

        order.AddValue("cmd", "join");
        order.AddValue("token", mCtx.mOptions.identity.token);
        order.AddValue("serverId", serverId);

        mWsClient.Send(order.ToString());
    }
}

std::vector<ServerState> Application::GetServers()
{
    std::scoped_lock<std::mutex> lock(mMutex);
    return mServers;
}

bool Application::Deliver(const Request &req)
{
    bool ret = true;

    mNetRequests.Push(req);
    return ret;
}

void Application::Disconnected()
{
    mCtx.AddMessage("Disconnected from server");
}


void Application::ChatMessage(const std::string &msg)
{
    // Event en provenance du joueur local
    // on l'envoie sur le réseau, vers le serveur
    mCtx.BuildChatMessage(msg, mNetReplies);
}

void Application::ExitGame()
{
    mCtx.Clear();
    // Re-init client net
    mNet.Disconnect();
    mGfx.SwitchSceneTo(SCENE_TITLE);
}

void Application::SendMyBid()
{
    mCtx.BuildReplyBid(mNetReplies);
}

void Application::SendMyCard(const Card &c)
{
    mCtx.BuildSendCard(c, mNetReplies);
}

void Application::ConfigChanged()
{
    ClientConfig::Save(mCtx.mOptions, ClientConfig::DefaultConfigFile());
    ApplyOptions();
}

void Application::ApplyOptions()
{
    // Reinit stuff
    mCtx.mEndOfTrickTimer.SetDelay(mCtx.mOptions.delayBeforeCleaning);
    mCtx.mEndOfTrickTimer.SetEnable(!mCtx.mOptions.clickToClean);
}

void Application::ClickOnBoard()
{
    if ((mCtx.mMode == PlayerContext::TABLE_MODE_WAIT_CLICK_END_OF_TRICK) ||
        (mCtx.mMode == PlayerContext::TABLE_MODE_WAIT_TIMER_END_OF_TRICK))
    {
        mCtx.mMode = PlayerContext::TABLE_MODE_BLOCKED;
        mCtx.Sync(Engine::WAIT_FOR_END_OF_TRICK, mNetReplies);
        mCtx.mCurrentTrick.Clear();
        mCtx.mEndOfTrickTimer.Stop();
    }
    else if (mCtx.mMode == PlayerContext::TABLE_MODE_SHOW_RESULTS)
    {
        mCtx.Sync(Engine::WAIT_FOR_READY, mNetReplies);
    }
    else
    {
        TLogError("ClickOnBoard() out of context!");
    }
}

void Application::OnWsData(const std::string &data)
{
    JsonReader reader;
    JsonValue json;
    std::scoped_lock<std::mutex> lock(mMutex);

    if (reader.ParseString(json, data))
    {
        if (json.HasValue("event"))
        {
            std::string event = json.FindValue("event").GetString();

            // -----------------  Réception de la liste des serveurs  -----------------
            if (event == "servers")
            {
                // On reçoit la liste des serveurs
                JsonArray serversList = json.FindValue("data").GetArray();

                mServers.clear();
                for (auto &s : serversList)
                {
                    ServerState state;
                    FromServersList(state, s.GetObj());
                    mServers.push_back(state);
                }
            }
            // -----------------  Réception du résultat de notre demande de join vers un serveur de jeu  -----------------
            else if (event == "joinReply")
            {
                // Le site Internet nous a autorisé à rejoindre un serveur
                // de jeu. En retour, il nous donne une clé de chiffrement unique à cette session de jeu
                // ainsi qu'un identifiant
                TLogInfo("[WS_RCV] joinReply");

                std::string webId = json.FindValue("data:webId").GetString();
                std::string gek = json.FindValue("data:webId").GetString();
                std::string passphrase = json.FindValue("data:webId").GetString();

                mNet.Disconnect();
                mNet.Initialize(webId, gek, passphrase);

                GfxEngine::Message msg;
                msg["event"] = "accessGranted";
                mAsyncMessages.Push(msg);
            }
        }
        else
        {
            TLogError("[WS_RCV] No event field in JSON");
        }
    }
    else
    {
        TLogError("[WS_RCV] Failed to parse event");
    }
}

// Callback qui provien du décodeur
// Il ajoute des réponses à envoyer au network si besoin
void Application::HandleRequest(const Request &req)
{
    JsonObject json;
    std::string ev;

    if (mCtx.Decode(req, json))
    {
        ev = json.GetValue("cmd").GetString();
    }

    if (!mCtx.mMyself.IsConnected())
    {
        if (ev == "RequestLogin")
        {
            mCtx.DecodeRequestLogin(json);
            mCtx.AddMessage("Connecting to lobby...");
            mCtx.BuildReplyLogin(mNetReplies);
        }
        return;
    }

    if (ev == "AccessGranted")
    {
        mCtx.AddMessage("Connected to lobby");
        mCtx.DecodeAccessGranted(json);

        if (mGameType == GAME_TYPE_LOCAL)
        {
            // As soon as we have entered into the lobby, join the assigned table
            mCtx.BuildJoinTable(Protocol::TABLES_UID, mNetReplies);
        }
    }
    else if (ev == "ReplyJoinTable")
    {
        mCtx.DecodeReplyJoinTable(json);
        std::stringstream ss;
        ss << "Entered table in position: " << mCtx.mMyself.place.ToString();
        mCtx.AddMessage(ss.str());

        mCtx.Sync(Engine::WAIT_FOR_PLAYERS, mNetReplies);
    }
    else if (ev == "NewDeal")
    {
        mCtx.DecodeNewDeal(json);
        TLogInfo("Received cards: " + mCtx.mDeck.ToString());
        mCtx.Sync(Engine::WAIT_FOR_CARDS, mNetReplies);
    }
    else if (ev == "RequestBid")
    {
        mCtx.DecodeRequestBid(json);
    }
    else if (ev == "RequestKingCall")
    {
        mCtx.Sync(Engine::WAIT_FOR_KING_CALL, mNetReplies);
    }
    else if (ev == "ShowKingCall")
    {
        mCtx.Sync(Engine::WAIT_FOR_SHOW_KING_CALL, mNetReplies);
    }
    else if (ev == "ShowBid")
    {
        mCtx.DecodeShowBid(json);
        mCtx.Sync(Engine::WAIT_FOR_SHOW_BID, mNetReplies);
    }
    else if (ev == "BuildDiscard")
    {
        mCtx.Sync(Engine::WAIT_FOR_DISCARD, mNetReplies);
    }
    else if (ev == "ShowDog")
    {
        mCtx.DecodeShowDog(json);
        mCtx.Sync(Engine::WAIT_FOR_SHOW_DOG, mNetReplies);
    }
    else if (ev == "StartDeal")
    {
        mCtx.DecodeStartDeal(json);
        std::stringstream ss;
        ss << "Start deal, taker is: " << mCtx.mBid.taker.ToString();
        mCtx.AddMessage(ss.str());
        mCtx.Sync(Engine::WAIT_FOR_START_DEAL, mNetReplies);
    }
    else if (ev == "ShowHandle")
    {
        mCtx.DecodeShowHandle(json);
        mCtx.Sync(Engine::WAIT_FOR_SHOW_HANDLE, mNetReplies);
    }
    else if (ev == "NewGame")
    {
        mCtx.DecodeNewGame(json);
        mCtx.Sync(Engine::WAIT_FOR_READY, mNetReplies);
    }
    else if (ev == "ShowCard")
    {
        mCtx.DecodeShowCard(json);
        mCtx.AddMessage("Player " + mCtx.mCurrentPlayer.ToString() + " plays " + mCtx.mCurrentTrick.Last().ToString());
        mCtx.Sync(Engine::WAIT_FOR_SHOW_CARD, mNetReplies);
    }
    else if (ev == "PlayCard")
    {
        mCtx.DecodePlayCard(json);

        if (mCtx.mOptions.autoPlay)
        {
            // Only reply a bid if it is our place to anwser
            if (mCtx.IsMyTurn())
            {
                Card c = mCtx.ChooseRandomCard();
                mCtx.mDeck.Remove(c);
                mCtx.BuildSendCard(c, mNetReplies);
            }
        }
    }
    else if (ev == "AskForHandle")
    {
        mCtx.BuildHandle(Deck(), mNetReplies);
    }
    else if (ev == "EndOfTrick")
    {
        mCtx.DecodeEndOfTrick(json);
        if (mCtx.mOptions.clickToClean)
        {
            mCtx.mMode = PlayerContext::TABLE_MODE_WAIT_CLICK_END_OF_TRICK;
        }
        else
        {
            mCtx.mMode = PlayerContext::TABLE_MODE_WAIT_TIMER_END_OF_TRICK;
            mCtx.mEndOfTrickTimer.Start();
        }
    }
    else if (ev == "EndOfGame")
    {
        mCtx.DecodeEndOfGame(json);
        std::stringstream ss;
        ss << "End of game, winner is: " << mCtx.mCurrentPlayer.ToString();
        mCtx.AddMessage(ss.str());

    }
    else if (ev == "AllPassed")
    {
        mCtx.AddMessage("All players have passed! New turn...");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        mCtx.Sync(Engine::WAIT_FOR_ALL_PASSED, mNetReplies);
    }
    else if (ev == "EndOfDeal")
    {
        mCtx.DecodeEndOfDeal(json);
        mCtx.Sync(Engine::WAIT_FOR_END_OF_DEAL, mNetReplies);
    }
    else if (ev == "ChatMessage")
    {
        mCtx.DecodeChatMessage(json);
    }
    else if (ev == "LobbyEvent")
    {
        mCtx.DecodeLobbyEvent(json);
    }
    else if (ev == "Error")
    {
        TLogInfo("[APP] Error event, reason: " + json.GetValue("reason").GetString());
    }
    else
    {
        TLogError("Unmanaged event: " + ev);
    }

}


