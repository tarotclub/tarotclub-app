#include "title-scene.h"
#include "scenes.h"
#include "http-client.h"
#include "JsonValue.h"
#include "JsonReader.h"
#include <string>
#include "Log.h"

#include <cstdlib>
#include <iostream>
#include <string>

class Background : public Entity
{

public:
    Background(GfxSystem &s)
        : Entity(s)
    {

    }

    virtual void OnCreate(SDL_Renderer *renderer) override
    {
        Entity::OnCreate(renderer);

        mScreenWidth = GetSystem().GetWindowSize().w;
        mScreenHeight = GetSystem().GetWindowSize().h;

        Uint32 pixelFormat;
        int mTileWidth = 0;
        int mTileHeight = 0;

        SDL_Texture *tileTexture = GfxEngine::LoadImage(renderer, "assets/hud/tile_bg1.png");


        // get the width and height of the texture
        if (SDL_QueryTexture(tileTexture, &pixelFormat, NULL, &mTileWidth, &mTileHeight) == 0)
        {
            SetSize(mTileWidth, mTileHeight);
        }

        mBigTexture = SDL_CreateTexture(renderer, pixelFormat, SDL_TEXTUREACCESS_TARGET, mScreenWidth, mScreenHeight);

        SDL_SetRenderTarget(renderer, mBigTexture);
        // Your draw loop here
        SDL_Rect r;
        r.w = mTileWidth;
        r.h = mTileHeight;
        r.x = 0;
        r.y = 0;
        // La tile est multiple de la résolution
        for (int i = 0; i < mScreenWidth / mTileWidth; i++)
        {
            r.y = 0;
            for (int j = 0; j < GetSystem().GetWindowSize().h / mTileHeight; j++)
            {
                SDL_RenderCopyEx(renderer, tileTexture, NULL, &r, 0.0, NULL, SDL_FLIP_NONE);
                r.y += mTileHeight;
            }
            r.x += mTileWidth;
        }

        SDL_SetRenderTarget(renderer, NULL);
        SDL_DestroyTexture(tileTexture);

    }

    virtual void Update(double deltaTime) override
    {
        move -= 0.03 * deltaTime;

        if (move <= mScreenWidth * -1) {
            move = 0;
        }
    }

    virtual void Draw(SDL_Renderer *renderer) override
    {
        SDL_Rect r;

        r.w = mScreenWidth;
        r.h = mScreenHeight;
        r.x = move;
        r.y = 0;
        SDL_RenderCopyEx(renderer, mBigTexture, NULL, &r, 0.0, NULL, SDL_FLIP_NONE);
        r.x += mScreenWidth;
        SDL_RenderCopyEx(renderer, mBigTexture, NULL, &r, 0.0, NULL, SDL_FLIP_NONE);
    }
private:
    double move = 0.0;
    SDL_Texture* mBigTexture = nullptr;

    int mScreenWidth = 0;
    int mScreenHeight = 0;
};


class Logo : public Image
{

public:
    Logo(GfxSystem &s)
        : Image(s, "assets/hud/logo.png")
    {
        mYPos = GetSystem().GetWindowSize().h / 3;
        SetPos(50, mYPos);
        SetScale(0.5, 0.5);
    }

    virtual void Draw(SDL_Renderer *renderer) override
    {
        DrawEx(renderer, GetX(), mYPos + 6*SDL_sin(SDL_GetTicks()*(3.14/1500)));
    }
private:
    int mYPos;
};


TitleScene::TitleScene(GfxSystem &system, IApplication &app, const std::string &version)
    : Scene(system)
    , mApp(app)
    , mVersion(version)
    , mWsClient(*this)
{
    auto bg = std::make_shared<Background>(GetSystem());
    mLogo = std::make_shared<Logo>(GetSystem());

    AddEntity(bg);
    AddEntity(mLogo);

    mHttpThread = std::thread(&TitleScene::RunHttp, this);
}

TitleScene::~TitleScene()
{
    mHttpQueue.Push(HttpClient::Request(true));
    if (mHttpThread.joinable())
    {
        mHttpThread.join();
    }

    mWsClient.Close();
    if (mWsThread.joinable())
    {
        mWsThread.join();
    }
}

void TitleScene::OnCreate(SDL_Renderer *renderer)
{
    Scene::OnCreate(renderer);
    mWsThread = std::thread(&TitleScene::RunWebSocket, this);
}

void TitleScene::OnActivate(SDL_Renderer *renderer)
{
    Scene::OnActivate(renderer);
}

void TitleScene::Draw(SDL_Renderer *renderer)
{
    Scene::Draw(renderer);
    if (mMenu == MENU_MAIN)
    {
        DrawMainMenu();
        DrawInfoMenu();
    }
    else if (mMenu == MENU_ONLINE)
    {
        DrawOnlineMenu();
    }
}

void TitleScene::OnWsData(const std::string &data)
{
    JsonReader reader;
    JsonValue json;
    std::scoped_lock<std::mutex> lock(mMutex);

    if (reader.ParseString(json, data))
    {
        if (json.HasValue("event"))
        {
            std::string event = json.FindValue("event").GetString();
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
        }
        else
        {
            TLogError("[WEBSOCKET] No event field in JSON");
        }
    }
    else
    {
        TLogError("[WEBSOCKET] Failed to parse event");
    }
}

void TitleScene::DrawMainMenu()
{
    // get the window size as a base for calculating widgets geometry
    int controls_width = 0;

    Rect rect = GetSystem().GetWindowSize();

    controls_width = rect.w;
    // make controls widget width to be 1/3 of the main window width
    if ((controls_width /= 3) < 300)
    {
        controls_width = 300;
    }

    // position the controls widget in the top-right corner with some margin
    ImGui::SetNextWindowPos(ImVec2(400, (rect.h /3)), ImGuiCond_Always);
    // here we set the calculated width and also make the height to be
    // be the height of the main window also with some margin
    ImGui::SetNextWindowSize(
                ImVec2(static_cast<float>(controls_width), static_cast<float>(rect.h / 3)),
                ImGuiCond_Always
                );
    // create a window and append into it
    ImGui::Begin("MainMenu", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);

    if (ImGui::Button("Story mode game"))
    {
        SwitchToScene(SCENE_STORY_MODE);
    }

    if (ImGui::Button("Start local game"))
    {
        SwitchToScene(SCENE_LOCAL_GAME);
    }

    if (ImGui::Button("Online game"))
    {
        mLogo->SetVisible(false);
        mMenu = MENU_ONLINE;
    }

    if (ImGui::Button("AI Editor"))
    {
        SwitchToScene(SCENE_AI_EDITOR);
    }

    if (ImGui::Button("Quit"))
    {
        SwitchToScene(SCENE_EXIT);
    }
    ImGui::End();
}
#ifdef TAROT_DEBUG
    static const std::string TAROTCLUB_HOST = "127.0.0.1";
#else
    static const std::string TAROTCLUB_HOST = "tarotclub.fr";
#endif    



void TitleScene::RunHttp()
{
    bool quit = false;
    HttpClient::Request req;
    while(!quit)
    {
        if (mHttpQueue.TryPop(req))
        {
            if (!req.quit)
            {
                std::string response = mHttpClient.ExecuteAsync(req);

                JsonReader reader;
                JsonValue json;

                if (reader.ParseString(json, response))
                {
                    if (json.HasValue("success"))
                    {
                        if (json.FindValue("success").GetBool())
                        {
                            TLogInfo("[WEB] Connected: " + json.ToString());
                            Identity ident;

                            ident.username = json.FindValue("data:profile:username").GetString();
                            ident.token = json.FindValue("data:profile:attr:token").GetString();
                            mApp.SetLogged(ident);
                            mConnectState = tribool::True;
                        }
                        else
                        {
                            TLogError("[ONLINE] Cannot connect: " + json.FindValue("message").GetString());
                            mConnectState = tribool::False;
                        }
                    }
                    else
                    {
                        TLogError("[ONLINE] Malformed JSON reply");
                        mConnectState = tribool::False;
                    }
                }
                else
                {
                    TLogError("[ONLINE] Cannot parse reply");
                    mConnectState = tribool::False;
                }
            }
            quit = req.quit;
        }
    }
}

void TitleScene::RunWebSocket()
{
    bool quit = false;
    while(!quit)
    {
        mWsClient.Run(TAROTCLUB_HOST, "9998");
//    mWsClient.Run("fjdskqshkdsqgldl.fr", "9998");
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

void TitleScene::HandleHttpReply()
{


}

void TitleScene::Login(const std::string &login, const std::string &password)
{
    JsonObject obj;
    obj.AddValue("login", login);
    obj.AddValue("password", password);

    HttpClient::Request req;

    req.body = obj.ToString();
    req.host = TAROTCLUB_HOST;
    req.port = "443";
    req.target = "/api/v1/auth/signin";

    mHttpQueue.Push(req);
}

void TitleScene::DrawOnlineMenu()
{
    // get the window size as a base for calculating widgets geometry
    int controls_width = 0;

    Rect rect = GetSystem().GetWindowSize();

    controls_width = rect.w;
    // make controls widget width to be 1/3 of the main window width
    if ((controls_width /= 3) < 400)
    {
        controls_width = 400;
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));


//    // position the controls widget in the top-right corner with some margin
//    ImGui::SetNextWindowPos(ImVec2(400, (rect.h /3)), ImGuiCond_Always);
    // here we set the calculated width and also make the height to be
    // be the height of the main window also with some margin
    ImGui::SetNextWindowSize(
                ImVec2(static_cast<float>(controls_width), static_cast<float>(rect.h / 2)),
                ImGuiCond_Always
                );
    // create a window and append into it
    ImGui::Begin("OnlineMenu", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    if (!mApp.IsLogged())
    {
        if (mWsClient.IsConnected())
        {
            if (mConnectState == tribool::False)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,0,0,255));
                ImGui::Text("Connection error");
                ImGui::PopStyleColor();
            }
            else if (mConnectState == tribool::True)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0,255,0,255));
                ImGui::Text("Vous êtes connecté.");
                ImGui::PopStyleColor();
            }
            else
            {
                ImGui::Text("Not connected! You cannot join any game server");
            }

            static char password[64] = "";
            static char login[64] = "";
            ImGui::InputText("Login",     login, 64);
            ImGui::InputText("Password", password, IM_ARRAYSIZE(password), ImGuiInputTextFlags_Password);

            if (ImGui::Button("Connect"))
            {
                Login(login, password);
            }
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,0,0,255));
            ImGui::Text("Internet not detected on this device, check connection.");
            ImGui::PopStyleColor();
        }
    }
    else
    {
        static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
        const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

        // When using ScrollX or ScrollY we need to specify a size for our table container!
        // Otherwise by default the table will fit all available space, like a BeginChild() call.
        ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 8);
        if (ImGui::BeginTable("table1", 3, flags, outer_size))
        {
            // Display headers so we can inspect their interaction with borders.
            // (Headers are not the main purpose of this section of the demo, so we are not elaborating on them too much. See other sections for details)
            ImGui::TableSetupColumn("Server name");
            ImGui::TableSetupColumn("Number of players");
            ImGui::TableSetupColumn("Number of tables");
            ImGui::TableHeadersRow();

            char buf[32];
            static int selected_row = -1;

            std::scoped_lock<std::mutex> lock(mMutex);
            for (int row = 0; row < mServers.size(); row++)
            {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
//                ImGui::TextUnformatted(mServers[row].name.c_str());

                ImGuiSelectableFlags selectable_flags =  ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
                if (ImGui::Selectable(mServers[row].name.c_str(), selected_row == row, selectable_flags, ImVec2(0, TEXT_BASE_HEIGHT)))
                {
                    selected_row = row;
                }


                ImGui::TableSetColumnIndex(1);
                sprintf(buf, "%d", mServers[row].nb_players);
                ImGui::TextUnformatted(buf);

                ImGui::TableSetColumnIndex(2);
                sprintf(buf, "%d", mServers[row].nb_tables);
                ImGui::TextUnformatted(buf);
            }
            ImGui::EndTable();
        }
    }

    if (ImGui::Button("Return to main menu"))
    {
        mLogo->SetVisible(true);
        mMenu = MENU_MAIN;
    }
    ImGui::End();
}

void TitleScene::DrawInfoMenu()
{
    Rect rect = GetSystem().GetWindowSize();
    const int windowHeight = 30;

    ImGui::SetNextWindowPos(ImVec2(0, rect.h - windowHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(100, windowHeight), ImGuiCond_Always);

    ImGui::Begin("InfoMenu", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground  | ImGuiWindowFlags_NoScrollbar);
    ImGui::Text("%s", mVersion.c_str());

    ImGui::End();
}
