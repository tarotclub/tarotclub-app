
#include "gfx-engine.h"
#include "IconsFontAwesome5.h"
#include "Util.h"
#include "Log.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#include <stdio.h>

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

    (void)pInput;
}



Entity::Entity(GfxSystem &s)
    : mSystem(s)
{
    mRect.x = 0;
    mRect.y = 0;
    mRect.w = 0;
    mRect.h = 0;
}

void Entity::ProcessEvent(const SDL_Event &event)
{
    (void) event;
}

void Entity::Update(double deltaTime) { (void) deltaTime; }

void Entity::Draw(SDL_Renderer *renderer) { (void) renderer; }

bool Entity::IsVisible() const { return mVisible; }

void Entity::SetVisible(bool visible) { mVisible = visible; }

int Entity::GetX() const
{
    return mRect.x;
}

int Entity::GetY() const
{
    return mRect.y;
}

int Entity::GetWidth() const
{
    return mRect.w;
}

int Entity::GetHeight() const
{
    return mRect.h;
}

const SDL_Rect &Entity::GetRect() const {

    return mRect;
}

Vector2f &Entity::GetScale()
{
    return mScale;
}

float Entity::GetAngle() const
{
    return mAngle;
}

void Entity::SetSize(int w, int h)
{
    mRect.w = w;
    mRect.h = h;
}

void Entity::SetPos(int x, int y)
{
    mRect.x = x;
    mRect.y = y;
}

void Entity::SetScale(float x, float y)
{
    mScale.x = x;
    mScale.y = y;
}

void Entity::SetAngle(float angle)
{
    mAngle = angle;
}

void Entity::SetSceneIdOnwer(uint32_t sceneId) {
    mSceneIdOnwer = sceneId;
}

void Entity::SetId(uint32_t id) {
    mId = id;
}

void Entity::SetZ(uint32_t z) {
    mZ = z;
}


Image::Image(GfxSystem &system, const std::string &path, Type type)
    : Entity(system)
    , mPath(path)
    , m_type(type)
{
}

Image::~Image()
{
    if (mTexture != nullptr)
    {
        SDL_DestroyTexture(mTexture);
        mTexture = nullptr;
    }
}

void Image::OnCreate(SDL_Renderer *renderer)
{
    Entity::OnCreate(renderer);

    std::cout << "Loading: " << mPath << std::endl;

    if (!Util::FileExists(mPath))
    {
        TLogError("[IMAGE] Can't find file: " + mPath);
    }

    mTexture = LoadImage(renderer, mPath.c_str());

    if (mTexture == nullptr)
    {
        TLogError("[IMAGE] Problem loading texture: " + mPath);
    }

    int w = 0;
    int h = 0;
    // get the width and height of the texture
    if (SDL_QueryTexture(mTexture, NULL, NULL, &w, &h) == 0)
    {
        SetSize(w, h);
    }
}

void Image::Draw(SDL_Renderer *renderer)
{
    if (!IsVisible())
    {
        return;
    }

    SDL_Rect rect = GetRect();
    rect.w *= GetScale().x;
    rect.h *= GetScale().y;

    SDL_SetTextureBlendMode(mTexture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureColorMod(mTexture, 255, 255, 255);
    SDL_RenderCopyEx(renderer, mTexture, NULL, &rect, GetAngle(), NULL, SDL_FLIP_NONE);
}

void Image::DrawEx(SDL_Renderer *renderer, int x, int y)
{
    if (!IsVisible())
    {
        return;
    }

    SDL_Rect r = GetRect();
    r.w *= GetScale().x;
    r.h *= GetScale().y;
    r.x = x;
    r.y = y;
    SDL_RenderCopyEx(renderer, mTexture, NULL, &r, GetAngle(), NULL, SDL_FLIP_NONE);
}


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

/* Load a SVG type image from an SDL datasource */
SDL_Texture *Image::LoadSVG(SDL_Renderer *renderer, const char *filename)
{
    std::string data = Util::FileToString(filename);
    SDL_Texture * tex = Image::RenderSVG(renderer, data.data());

    if (tex == nullptr)
    {
        tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 20, 20);
    }

    return tex;
}

SDL_Texture *Image::RenderSVG(SDL_Renderer *renderer, const std::string &svgData)
{
    std::string data = svgData; // on est obligé de créer une copie car nsvgParse modifie le code SVG
    struct NSVGimage *image;
    struct NSVGrasterizer *rasterizer;
    SDL_Surface *surface = NULL;
    float scale = 1.0f;

    /* For now just use default units of pixels at 96 DPI */
    image = nsvgParse(data.data(), "px", 96.0f);

    if ( !image ) {
        //IMG_SetError("Couldn't parse SVG image");
        return NULL;
    }

    rasterizer = nsvgCreateRasterizer();
    if ( !rasterizer ) {
        // IMG_SetError("Couldn't create SVG rasterizer");
        nsvgDelete( image );
        return NULL;
    }

    surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                   (int)(image->width * scale),
                                   (int)(image->height * scale),
                                   32,
                                   0x000000FF,
                                   0x0000FF00,
                                   0x00FF0000,
                                   0xFF000000);
    if ( !surface ) {
        nsvgDeleteRasterizer( rasterizer );
        nsvgDelete( image );
        return NULL;
    }

    nsvgRasterize(rasterizer, image, 0.0f, 0.0f, scale, (unsigned char *)surface->pixels, surface->w, surface->h, surface->pitch);
    nsvgDeleteRasterizer( rasterizer );
    nsvgDelete( image );

    return SDL_CreateTextureFromSurface(renderer, surface);
}



SDL_Texture *Image::LoadImage(SDL_Renderer *renderer, const char* filename)
{
    // Read data
    int32_t width, height, bytesPerPixel;
    void* data = stbi_load(filename, &width, &height, &bytesPerPixel, 0);

    // Calculate pitch
    int pitch;
    pitch = width * bytesPerPixel;
    pitch = (pitch + 3) & ~3;

    // Setup relevance bitmask
    int32_t Rmask, Gmask, Bmask, Amask;
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    Rmask = 0x000000FF;
    Gmask = 0x0000FF00;
    Bmask = 0x00FF0000;
    Amask = (bytesPerPixel == 4) ? 0xFF000000 : 0;
#else
    int s = (bytesPerPixel == 4) ? 0 : 8;
    Rmask = 0xFF000000 >> s;
    Gmask = 0x00FF0000 >> s;
    Bmask = 0x0000FF00 >> s;
    Amask = 0x000000FF >> s;
#endif
    SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(data, width, height, bytesPerPixel*8, pitch, Rmask, Gmask, Bmask, Amask);
    SDL_Texture* t = nullptr;
    if (surface)
    {
        t = SDL_CreateTextureFromSurface(renderer, surface);
    }
    else
    {
        t = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 20, 20);
    }

    STBI_FREE(data);
    SDL_FreeSurface(surface);
    return t;
}

// ========================================================================================================================
// TEXT
// ========================================================================================================================
Text::Text(GfxSystem &system, const std::string &path, const std::string &text)
    : Entity(system)
    , m_path(Util::GetWorkingDirectory() + "/" + path)
    , m_text(text)
{

}

Text::~Text()
{
    if (m_font != nullptr)
    {
        TTF_CloseFont(m_font);
    }
}


void Text::OnCreate(SDL_Renderer *renderer)
{
    Entity::OnCreate(renderer);

    //this opens a font style and sets a size
    m_font = TTF_OpenFont(m_path.c_str(), 24);


    if(!m_font) {
        std::cout << "TTF_OpenFont error: " << TTF_GetError();
        // handle error
    }

    SDL_Color color = { 0, 0, 0 };
    SDL_Surface * surface = TTF_RenderText_Solid(m_font, m_text.c_str(), color);

    m_texture = (SDL_CreateTextureFromSurface(renderer, surface));

    int w = 0;
    int h = 0;
    // get the width and height of the texture
    if (SDL_QueryTexture(m_texture, NULL, NULL, &w, &h) == 0)
    {
        SetSize(w, h);
    }

    SDL_FreeSurface(surface);
}

void Text::Draw(SDL_Renderer *renderer)
{
    if (!IsVisible())
    {
        return;
    }

    SDL_Rect rect = GetRect();
    rect.w *= GetScale().x;
    rect.h *= GetScale().y;

    SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureColorMod(m_texture, 255, 255, 255);
    SDL_RenderCopyEx(renderer, m_texture, NULL, &rect, GetAngle(), NULL, SDL_FLIP_NONE);
}


// ========================================================================================================================
// GFX ENGINE
// ========================================================================================================================

void GfxEngine::AddScene(std::shared_ptr<Scene> scene, uint32_t id)
{
    mScenes[id] = scene;
}

void GfxEngine::SwitchSceneTo(uint32_t sceneId, const std::map<std::string, Value> &args)
{
    mCurrentSceneId = sceneId;
    mSceneActivated = true;
    mArgs = args;
}

Rect GfxSystem::GetWindowSize()
{
    Rect rect;
    SDL_GetWindowSize(mWindow, &rect.w, &rect.h);
    return rect;
}

bool GfxEngine::Initialize(const std::string &window_title)
{
    // initiate SDL
    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS |
                 SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("[ERROR] %s\n", SDL_GetError());
        return -1;
    }

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(
        SDL_WINDOW_OPENGL
        | SDL_WINDOW_RESIZABLE
        | SDL_WINDOW_ALLOW_HIGHDPI
        );
    mWindow = SDL_CreateWindow(
        window_title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        mWidth,
        mHeight,
        window_flags
        );
    // limit to which minimum size user can resize the window
    SDL_SetWindowMinimumSize(mWindow, mMinimumWidth, mMinimumHeight);

    // setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    mNormalFont = io.Fonts->AddFontFromFileTTF( "assets/fonts/roboto.ttf", 20);

    ImFontConfig config;
    config.MergeMode = true; // ATTENTION, MERGE AVEC LA FONT PRECEDENTE !!
//    config.GlyphMinAdvanceX = 20.0f; // Use if you want to make the icon monospaced
//    config.GlyphOffset.y += 1.0;
    static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    io.Fonts->AddFontFromFileTTF("assets/fonts/fa-solid-900.ttf", 16.0f, &config, icon_ranges);

    mBigFont = io.Fonts->AddFontFromFileTTF( "assets/fonts/roboto.ttf", 60);

    io.Fonts->Build();

    io.IniFilename = nullptr; // disable .ini save/load windows sizes and locations

    ImGuiStyle& style = ImGui::GetStyle();

    ImVec4* colors = style.Colors;

    style.WindowRounding    = 0.0f;             // Radius of window corners rounding. Set to 0.0f to have rectangular windows
    style.WindowBorderSize  = 1.0f;

    style.ScrollbarRounding = 3.0f;             // Radius of grab corners rounding for scrollbar
    style.GrabRounding      = 2.0f;             // Radius of grabs corners rounding. Set to 0.0f to have rectangular slider grabs.
    style.AntiAliasedLines  = true;
    style.AntiAliasedFill   = true;
    style.ChildRounding     = 2;
    style.ScrollbarSize     = 16;
    style.ScrollbarRounding = 3;
    style.GrabRounding      = 2;
    style.ItemSpacing.x     = 10;
    style.ItemSpacing.y     = 4;
    style.IndentSpacing     = 22;

    style.Alpha             = 1.0f;

    style.FramePadding.x    = 6;
    style.FramePadding.y    = 4;
    style.FrameRounding     = 0.0f;
    style.FrameBorderSize   = 1.0f;


    colors[ImGuiCol_Text]                   = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]              = ImVec4(1.0f, 0.847f, 0.388f, 1.00f); // Jaune foncé
    //colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.93f, 0.93f, 0.93f, 0.98f);
    colors[ImGuiCol_Border]                = ImVec4(0.318f, 0.314f, 0.314f, 1.0f); // Gris foncé
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(1.0f, 0.847f, 0.388f, 1.00f); // 0xFF, 0xD8, 0x63, 0xFF
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.94f, 0.94f, 0.94f, 0.55f);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(0.71f, 0.78f, 0.69f, 0.98f);
    colors[ImGuiCol_TitleBg]               = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.82f, 0.78f, 0.78f, 0.51f);
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
    colors[ImGuiCol_MenuBarBg]             = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.20f, 0.25f, 0.30f, 0.61f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.90f, 0.90f, 0.90f, 0.30f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.92f, 0.92f, 0.92f, 0.78f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_CheckMark]             = ImVec4(0.184f, 0.407f, 0.193f, 1.00f);
    colors[ImGuiCol_SliderGrab]            = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Button]                = ImVec4(0.71f, 0.78f, 0.69f, 0.40f);
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.278, 0.576, 0.902, 1.00f);  // Bleu foncé
    colors[ImGuiCol_ButtonActive]          = ImVec4(0.793f, 0.900f, 0.836f, 1.00f);
    colors[ImGuiCol_Header]                = ImVec4(0.71f, 0.78f, 0.69f, 0.31f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.71f, 0.78f, 0.69f, 0.80f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(0.71f, 0.78f, 0.69f, 1.00f);
//     colors[ImGuiCol_Column]                = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
//     colors[ImGuiCol_ColumnHovered]         = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
//     colors[ImGuiCol_ColumnActive]          = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
    colors[ImGuiCol_ResizeGrip]            = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
    colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.26f, 0.59f, 0.98f, 0.45f);
    colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
    colors[ImGuiCol_PlotLines]             = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
//     colors[ImGuiCol_ModalWindowDarkening]  = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_NavHighlight]           = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);

    // Setup renderer    
    mRenderer =  SDL_CreateRenderer( mWindow, -1, SDL_RENDERER_ACCELERATED);

    // setup platform/renderer bindings
    ImGui_ImplSDL2_InitForSDLRenderer(mWindow);
    ImGui_ImplSDLRenderer_Init(mRenderer);


    if(TTF_Init()==-1) {
        printf("TTF_Init: %s\n", TTF_GetError());
        exit(2);
    }

    return mRenderer != nullptr;
}

void GfxEngine::Warmup()
{
   currentTick = SDL_GetPerformanceCounter();

   for (auto &s : mScenes)
   {
       s.second->OnCreate(mRenderer);
   }

}


uint32_t GfxEngine::Process(const Message &msg)
{
    uint32_t nextScene = 0;

    SDL_RenderClear(mRenderer);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        // without it you won't have keyboard input and other things
        ImGui_ImplSDL2_ProcessEvent(&event);
        // you might also want to check io.WantCaptureMouse and io.WantCaptureKeyboard
        // before processing events

        switch (event.type)
        {
        case SDL_QUIT:
            break;

        case SDL_WINDOWEVENT:
            switch (event.window.event)
            {
            case SDL_WINDOWEVENT_CLOSE:
                return 10000;
                break;
            case SDL_WINDOWEVENT_RESIZED:
                mWidth = event.window.data1;
                mHeight = event.window.data2;
                // std::cout << "[INFO] Window size: "
                //           << windowWidth
                //           << "x"
                //           << windowHeight
                //           << std::endl;
                break;
            }
            break;
        }

        if (mScenes.count(mCurrentSceneId))
        {
            auto s = mScenes.at(mCurrentSceneId);
            s->ProcessEvent(event);
        }
    }

    // start the Dear ImGui frame
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame(mWindow);
    ImGui::NewFrame();

    if (mScenes.count(mCurrentSceneId))
    {
        auto s = mScenes.at(mCurrentSceneId);
        if (mSceneActivated)
        {
            mSceneActivated = false;
            s->OnActivate(mRenderer, mArgs);
        }

        lastTick = currentTick;
        currentTick = SDL_GetPerformanceCounter();
        deltaTime = (double)((currentTick - lastTick)*1000 / (double)SDL_GetPerformanceFrequency() );

        if (msg.size() > 0)
        {
            s->OnMessage(msg);
        }

        s->Update(deltaTime);
        s->Draw(mRenderer);

        nextScene = s->GetNextScene();
        if (nextScene > 0)
        {
            SwitchSceneTo(nextScene, s->GetArgs());
        }
    }
    else
    {
        // FIXME: log error
    }

    // rendering
    ImGui::Render();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(mRenderer);

    return nextScene;
}

void GfxEngine::PlayAudio(const std::string &filename)
{

    ma_result result;
    ma_decoder decoder;
    ma_device_config deviceConfig;
    ma_device device;


    result = ma_decoder_init_file(filename.c_str(), NULL, &decoder);
    if (result != MA_SUCCESS) {
        printf("Could not load file: %s\n", filename.c_str());
    }

    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = decoder.outputFormat;
    deviceConfig.playback.channels = decoder.outputChannels;
    deviceConfig.sampleRate        = decoder.outputSampleRate;
    deviceConfig.dataCallback      = data_callback;
    deviceConfig.pUserData         = &decoder;

    if (ma_device_init(NULL, &deviceConfig, &device) != MA_SUCCESS) {
        printf("Failed to open playback device.\n");
        ma_decoder_uninit(&decoder);
    }

    if (ma_device_start(&device) != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder);
    }

    printf("Press Enter to quit...");
    getchar();

    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);



}

void GfxEngine::Close()
{
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();  

    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}


