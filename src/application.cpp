#include "application.h"
#include "scenes.h"
#include "story-mode-scene.h"

#include "SDL2/SDL_mixer.h"

Application::Application()
    : Observer(Log::All)
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
    mGfx.AddScene(std::make_shared<StoryModeScene>(mGfx), SCENE_STORY_MODE);
//    mGfx.AddScene(std::make_shared<AiEditorScene>(mGfx, *this), SCENE_AI_EDITOR);

    mGfx.SwitchSceneTo(SCENE_TITLE); // First scene

    mGfx.Warmup();


    bool loop = true;


    GfxEngine::Message msg;

    ApplyOptions();

    // Boucle principale de l'application qui se charge de l'affichage et des données du joueur humain
    // Ici on tourne dans un thread, si d'autres threads veulent communiquer avec nous, il faut passer par une queue de messages
    // ainsi, on n'a quasiment aucun "lock" trop long
    while (loop)
    {
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

}

void Application::Stop()
{

}

bool Application::IsInternetDetected()
{
    return false;
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






void Application::ApplyOptions()
{
    // Reinit stuff

}





