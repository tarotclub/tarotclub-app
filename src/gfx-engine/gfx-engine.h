#ifndef GFX_ENGINE_H
#define GFX_ENGINE_H

// SDL
#include <SDL2/SDL.h>
#include "SDL2/SDL_ttf.h"

// Dear ImGui
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "imgui_internal.h"

// C++
#include <vector>
#include <iostream>
#include <list>
#include <memory>
#include <map>
#include <algorithm>
#include <functional>

// ICL
#include "Value.h"

#include <iostream>
#include <string>


class GfxSystem;

struct Vector2f
{
    Vector2f()
        :x(0.0f), y(0.0f)
    {}

    Vector2f(float p_x, float p_y)
        :x(p_x), y(p_y)
    {}

    void print()
    {
        std::cout << x << ", " << y << std::endl;
    }

    float x, y;
};

struct Vector2
{
    Vector2()
        : x(0)
        , y(0)
    {}

    Vector2(int p_x, int p_y)
        : x(p_x)
        , y(p_y)
    {}

    void print()
    {
        std::cout << x << ", " << y << std::endl;
    }

    int x, y;
};


struct Rect : Vector2f
{
    int w;
    int h;

    Rect() {
        w = 0;
        h = 0;
    }
};


class Entity
{
public:
    Entity(GfxSystem &s);

    virtual ~Entity() {}

    virtual void OnCreate(SDL_Renderer *renderer) { (void) renderer; }

    // Manage user interaction (mouse, keyboard...)
    virtual void ProcessEvent(const SDL_Event &event);

    // Update state
    virtual void Update(double deltaTime);

    // Draw to screen
    virtual void Draw(SDL_Renderer *renderer);

    bool IsVisible() const;

    std::vector<std::shared_ptr<Entity>> GetChilds() const;

    void AddChildEntity(std::shared_ptr<Entity> e);

    int GetX() const;
    int GetY() const;
    int GetWidth() const;
    int GetHeight() const;
    const SDL_Rect &GetRect() const;
    Vector2f &GetScale();
    float GetAngle() const;
    GfxSystem &GetSystem() { return mSystem; }

    void SetVisible(bool visible);
    void SetPos(int x, int y);
    void SetSize(int w, int h);
    void SetScale(float x, float y);
    void SetAngle(float angle);
    void SetSceneIdOnwer(uint32_t sceneId);
    void SetId(uint32_t id);
    void SetZ(uint32_t z);
    uint32_t GetZ() const { return mZ; }

private:
    GfxSystem &mSystem; // keep it first please

    std::vector<std::shared_ptr<Entity>> m_childs;

    uint32_t mSceneIdOnwer = 0;
    uint32_t mId = 0;
    uint32_t mZ = 0; // pseudo Z value (order of drawing)

    bool mVisible = true;
    SDL_Rect mRect;

    //   Vector2f mPos;
    float mAngle = 0;
    Vector2f mScale = Vector2f(1, 1);
};


class Image : public Entity
{

public:
    enum Type {
        IMG_RASTER,
        IMG_SVG,
        IMG_TEXT
    };

    Image(GfxSystem &system, const std::string &path, Type type = IMG_RASTER);

    virtual ~Image();

    virtual void OnCreate(SDL_Renderer *renderer) override;
    virtual void Draw(SDL_Renderer *renderer) override;

    void DrawEx(SDL_Renderer *renderer, int x, int y);
    void SetHighlight(bool enable) { mHighlight = enable; }
    bool IsHighlighted() const { return mHighlight; }

    SDL_Texture *GetTexture() const { return mTexture; }
    void SetTexture(SDL_Texture *tex) { mTexture = tex; }

    std::string GetPath() const { return mPath; }

    static SDL_Texture *LoadSVG(SDL_Renderer *renderer, const char *filename);
    static SDL_Texture *RenderSVG(SDL_Renderer *renderer, const std::string &svgData);
    static SDL_Texture *LoadRasterImage(SDL_Renderer *renderer, const char *filename);
    static SDL_Texture *CreateText(SDL_Renderer *renderer, const char *fontfile, const char *text, int pixelsize);

private:
    std::string mPath;
    Image::Type m_type{IMG_RASTER};
    SDL_Texture *mTexture{nullptr};
    bool mHighlight = false;
};

class Text : public Entity
{
public:
    Text(GfxSystem &system, const std::string &path, const std::string &text);

    virtual ~Text();

    SDL_Texture *GetTexture() const { return m_texture; }

    virtual void OnCreate(SDL_Renderer *renderer) override;

    virtual void Draw(SDL_Renderer *renderer) override;

private:
    std::string m_path;
    std::string m_text;
    TTF_Font* m_font;
    SDL_Texture *m_texture{nullptr};
};


struct Glyph
{
    SDL_Rect rect;
    SDL_Texture *tex;
};

class GfxSystem
{
public:
    Rect GetWindowSize();
    SDL_Renderer *GetRenderer() { return mRenderer; }


    void InitFont(int fontType, const std::string &filename, int font_size);
    void DrawTextFromAtlas(const std::string &text, int x, int y, int r, int g, int b, int fontType);

protected:
    SDL_Window *mWindow = nullptr;
    SDL_Renderer *mRenderer = nullptr;

    std::map<int, std::map<char, Glyph>> m_atlas;
};



class Scene
{
public:

    Scene(GfxSystem &s)
        : mSystem(s)
    {
        mEntityIds = 0;
    }

    // Called when scene initially created. Called once.
    virtual void OnCreate(SDL_Renderer *renderer) {
        for (auto & e : mEntities)
        {
            e->OnCreate(renderer);
        }
    }
    // Called when scene destroyed. Called at most once (if a scene 
    // is not removed from the game, this will never be called).
    virtual void OnDestroy() {}
    // Called whenever a scene is transitioned into. Can be 
    // called many times in a typical game cycle.
    virtual void OnActivate(SDL_Renderer *renderer, const std::map<std::string, Value> &args = std::map<std::string, Value>())
    {
        mSwitchToScene = 0;
        (void) renderer;
        mArgs = args;
    }
    // Called whenever a transition out of a scene occurs. 
    // Can be called many times in a typical game cycle.
    virtual void OnDeactivate() {};
    // The below functions can be overridden as necessary in our scenes.
    virtual void ProcessEvent(const SDL_Event &event)
    {
        for (auto & e : mEntities)
        {
            e->ProcessEvent(event);
        }
    };

    virtual void Update(double deltaTime)
    {
        for (auto & e : mEntities)
        {
            e->Update(deltaTime);
        }
    };

    virtual void Draw(SDL_Renderer *renderer)
    {
        for (auto & e : mEntities)
        {
            e->Draw(renderer);
        }
    };

    virtual void OnMessage(const std::map<std::string, Value> &message){
        (void) message;
    }

    uint32_t AddEntity(std::shared_ptr<Entity> entity);

    GfxSystem &GetSystem() { return mSystem; }

    void SwitchToScene(uint32_t newScene, const std::map<std::string, Value> &args = std::map<std::string, Value>()) {
        mSwitchToScene = newScene;
        mArgs = args;
    }

    uint32_t GetNextScene() {
        return mSwitchToScene;
    }

    std::map<std::string, Value> GetArgs()
    {
        return mArgs;
    }

    // Sort according to Z value of each entity
    void Sort() {
        std::sort(mEntities.begin(), mEntities.end(), [](const std::shared_ptr<Entity> &a, const std::shared_ptr<Entity> &b) {
            return a->GetZ() < b->GetZ();
        });
    }

private:
    GfxSystem &mSystem;
    std::string mName;
    std::vector<std::shared_ptr<Entity>> mEntities;
    uint32_t mEntityIds = 0;
    uint32_t mSwitchToScene = 0;
    std::map<std::string, Value> mArgs;
};



class GfxEngine : public GfxSystem
{
public:
    typedef std::map<std::string, Value> Message;

    GfxEngine()
    {

    }

    ~GfxEngine()
    {

    }

    bool Initialize(const std::string &window_title);

    void Warmup();
    uint32_t Process(const Message &msg);

    void Close();

    static void PushBigFont() { ImGui::PushFont(mBigFont); }
    static void PopBigFont() { ImGui::PopFont(); }



    void AddScene(std::shared_ptr<Scene> scene, uint32_t id);
    void SwitchSceneTo(uint32_t sceneId, const std::map<std::string, Value> &args = std::map<std::string, Value>());

    void PlayAudio(const std::string &filename);
private:
    uint32_t mWidth = 1152;
    uint32_t mHeight = 648;

    const uint32_t mMinimumWidth = 1152;
    const uint32_t mMinimumHeight = 648;


    Uint32 totalFrameTicks = 0;
    Uint32 totalFrames = 0;

    Uint64 currentTick = 0;
    Uint64 lastTick = 0;

    std::string fpsStr;

    Uint64 nextFrame;
    Uint64 nextSecond;
    double deltaTime = 0;
    unsigned fps = 0;

    ImFont* mNormalFont = nullptr;
    static ImFont* mBigFont;

    // Key: id
    std::map<uint32_t, std::shared_ptr<Scene>> mScenes;
    bool mSceneActivated = false;
    uint32_t mCurrentSceneId = 0;

    std::map<std::string, Value> mArgs;
};

#endif // GFX_ENGINE_H
