#include "title-scene.h"
#include "scenes.h"
#include "http-client.h"
#include "JsonValue.h"
#include "JsonReader.h"
#include <string>
#include "Log.h"

class Background : public Entity
{

public:
    Background(GfxSystem &s)
        : Entity(s)
    {

    }

    virtual void Initialize(SDL_Renderer *renderer) override
    {
        Entity::Initialize(renderer);

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
    , resolver(io_context)
    , socket(io_context)
    , ssl_ctx(asio::ssl::context::tls)
{
    auto bg = std::make_shared<Background>(GetSystem());
    mLogo = std::make_shared<Logo>(GetSystem());

    AddEntity(bg);
    AddEntity(mLogo);
}

TitleScene::~TitleScene()
{
    if (mHttpThread.joinable())
    {
        mHttpThread.join();
    }
}

void TitleScene::OnCreate(SDL_Renderer *renderer)
{
    Scene::OnCreate(renderer);
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

    if (ImGui::Button("Start local game"))
    {
        SwitchToScene(SCENE_LOCAL_GAME);
    }

    if (ImGui::Button("Online game"))
    {
        mLogo->SetVisible(false);
        mMenu = MENU_ONLINE;
    }

    if (ImGui::Button("Quit"))
    {
        SwitchToScene(SCENE_EXIT);
    }
    ImGui::End();
}


void TitleScene::ConnectToWebsite(const std::string &login, const std::string &password)
{
    static bool busy = false;

    if (busy)
    {
        return;
    }

    busy = true;

    if (mHttpThread.joinable())
    {
        mHttpThread.join();
    }

    // give it some work, to prevent premature exit
    mHttpThread = std::thread([&] {
        try
        {
            HttpRequest r;
            JsonObject obj;
            obj.AddValue("login", login);
            obj.AddValue("password", password);

            r.body = obj.ToString();
            r.headers["Host"] = "tarotclub.fr";
            r.headers["Accept"] = "*/*";
            r.headers["Connection"] = "close";
            r.headers["Content-Type"] = "application/json";
            r.headers["Content-Length"] = std::to_string(r.body.size());

            r.method = "POST";
            r.query = "/api/v1/auth/signin";

            io_context.restart();
            asio::executor_work_guard<decltype(io_context.get_executor())> work{io_context.get_executor()};
            asio::ip::tcp::resolver::query query("tarotclub.fr", "443");
            asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);
            HttpClient c(io_context, ssl_ctx, iterator, r);
            io_context.run();

//            std::cout << "EXIT" <<std::endl;

            busy = false;

            JsonReader reader;
            JsonValue json;

            if (reader.ParseString(json, c.get_reply().body))
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
                    mConnectState = tribool::False;
                }
            }
            else
            {
                TLogError("[ONLINE] Cannot parse reply");
                mConnectState = tribool::False;
            }


//            std::string payload = c.get_data();

//              if (c.is_timeout())
//            {
//                std::cout << "Timeout!" <<std::endl;
//            }
//            else
//            {

//            }
        }
        catch (std::exception& e)
        {
            std::cerr << "Exception: " << e.what() << "\n";
            busy = false;
            mConnectState = tribool::False;
        }
    });

}

void TitleScene::DrawOnlineMenu()
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

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));


//    // position the controls widget in the top-right corner with some margin
//    ImGui::SetNextWindowPos(ImVec2(400, (rect.h /3)), ImGuiCond_Always);
    // here we set the calculated width and also make the height to be
    // be the height of the main window also with some margin
    ImGui::SetNextWindowSize(
                ImVec2(static_cast<float>(controls_width), static_cast<float>(rect.h / 3)),
                ImGuiCond_Always
                );
    // create a window and append into it
    ImGui::Begin("OnlineMenu", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

    static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

    if (! mApp.IsLogged())
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
            ConnectToWebsite(login, password);
        }

    }
    else
    {
        if (ImGui::BeginTable("table1", 3, flags))
        {
            // Display headers so we can inspect their interaction with borders.
            // (Headers are not the main purpose of this section of the demo, so we are not elaborating on them too much. See other sections for details)
                ImGui::TableSetupColumn("One");
                ImGui::TableSetupColumn("Two");
                ImGui::TableSetupColumn("Three");
                ImGui::TableHeadersRow();


            for (int row = 0; row < 5; row++)
            {
                ImGui::TableNextRow();
                for (int column = 0; column < 3; column++)
                {
                    ImGui::TableSetColumnIndex(column);
                    char buf[32];
                    sprintf(buf, "Hello %d,%d", column, row);
                    ImGui::TextUnformatted(buf);

                }
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
