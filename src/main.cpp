

#include <iostream>

#include "Log.h"
#include "scenes.h"

#include "story-mode-scene.h"
#include "title-scene.h"

/**
 * @brief The LivreurApp class
 *
 * Couleurs:
 *             Foncé          Clair
 *           -------------------------
 *  Sable    |  ffd863        fef3d5
 *  Gris     |  515050        80828f
 *  Bleu     |  4793e6        ddeafa
 *
 */



class LivreurApp : public IApplication
{
public:

    void Loop() {
         bool gfxInit = mGfx.Initialize("Livreur de Magazines : l'Ascension de Denis");


        mGfx.AddScene(std::make_shared<TitleScene>(mGfx, *this, "v3.velo"), SCENE_TITLE);
        mGfx.AddScene(std::make_shared<StoryModeScene>(mGfx), SCENE_STORY_MODE);


        mGfx.SwitchSceneTo(SCENE_TITLE); // First scene

        mGfx.Warmup();

        bool loop = true;

        GfxEngine::Message msg;


        // Boucle principale de l'application qui se charge de l'affichage et des données du joueur humain
        // Ici on tourne dans un thread, si d'autres threads veulent communiquer avec nous, il faut passer par une queue de messages
        // ainsi, on n'a quasiment aucun "lock" trop long
        while (loop)
        {
            //        // 1. On récupère et on décode les trames réseau
            //        if (mNetRequests.TryPop(req))
            //        {
            //            // Il est important de décoder dans le même thread que la boucle d'affichage
            //            // de cette façon, on n'a pas d'accès concurrentiel à gérer (mutex)
            //            HandleRequest(req);
            //        }

            // 2. On avance des éventuels timings / delay / timeouts
            //        mCtx.Update();

            // 3. On récupère les éventuels autres messages en provenance d'autres threads
            //    et à destination de la scène courante
            msg.clear();


            // 4. On traite les entrées (clavier/souris) et on affiche le jeu
            if (mGfx.Process(msg) == SCENE_EXIT)
            {
                loop = false;
            }
        }
    }

    virtual bool IsLogged() override {
        return true;
    }
    virtual bool IsInternetDetected() override {
        return true;
    }

    virtual std::string GetHost() const override {
        return "";
    }
    virtual void ConnectToServer(const std::string &serverId) override {

    }



private:

    GfxEngine mGfx;
};

#include <windows.h>

extern "C" int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    HWND windowHandle = GetConsoleWindow();
        ShowWindow(windowHandle,SW_HIDE);

    Log::SetLogPath("");

    LivreurApp app;
    app.Loop();

    return 0;
}
