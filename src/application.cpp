#include "application.h"
#include "Embedded.h"
#include "scenes.h"
#include "System.h"
#include "story-mode-scene.h"
#include "ai-editor-scene.h"
#include "online-board-scene.h"
#include "local-game-scene.h"
#include "JsonReader.h"

#include "SDL2/SDL_mixer.h"

Application::Application(INetClient &net)
    : Observer(Log::All)
    , mNet(net)
{


}

Application::~Application()
{

}

bool Application::Initialize()
{

    bool gfxInit = mGfx.Initialize("TarotClub");

    //Initialize SDL_mixer
  if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
  {
      printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );

  }

//    mGfx.AddFont("menu", "assets/fonts/roboto.ttf", 20);
//    mGfx.AddFont("icons", "assets/fonts/MaterialIcons-Regular.ttf", 20);

    return gfxInit;
}


void Application::Update(const Log::Infos &info)
{
    std::cout << info.ToString() << std::endl;
}

int Application::Loop()
{
    // Initialize all scenes and stuff
    mGfx.AddScene(std::make_shared<TitleScene>(mGfx, *this, TAROTCLUB_APP_VERSION), SCENE_TITLE);
    mGfx.AddScene(std::make_shared<StoryModeScene>(mGfx, *this), SCENE_STORY_MODE);
    mGfx.AddScene(std::make_shared<AiEditorScene>(mGfx, *this), SCENE_AI_EDITOR);

    mGfx.SwitchSceneTo(SCENE_TITLE); // First scene

    mGfx.Warmup();


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
        // 3. On récupère les éventuels autres messages en provenance d'autres threads
        //    et à destination de la scène courante
        msg.clear();
        mAsyncMessages.TryPop(msg);

        // 4. On traite les entrées (clavier/souris) et on affiche le jeu
        if (mGfx.Process(msg) == SCENE_EXIT)
        {
            loop = false;
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
    return false;
}

void Application::SetLogged(const Identity &ident)
{
    mLogged = true;

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

}


void Application::ChatMessage(const std::string &msg)
{

}

void Application::ExitGame()
{

}

void Application::SendMyBid()
{

}

void Application::SendMyCard(const Card &c)
{

}

void Application::ConfigChanged()
{

}

void Application::ApplyOptions()
{
    // Reinit stuff

}

void Application::ClickOnBoard()
{

}

void Application::OnWsData(const std::string &data)
{

}

// Callback qui provien du décodeur
// Il ajoute des réponses à envoyer au network si besoin
void Application::HandleRequest(const Request &req)
{


}


