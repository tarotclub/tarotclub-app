#ifndef STORYMODESCENE_H
#define STORYMODESCENE_H


#include <vector>
#include <map>
#include <memory>
#include "gfx-engine.h"
#include "i-board-event.h"
#include "PlayerContext.h"
#include "board-hud.h"
#include "DataBase.h"

class Car : public Entity
{
public:
    Car(GfxSystem &s)
        : Entity(s)
    {

    }
    virtual void OnCreate(SDL_Renderer *renderer) override;

    virtual void Draw(SDL_Renderer *renderer) override;

private:
    std::string mDeuxCvSVG;
    SDL_Texture *mTexture = nullptr;
};


class FranceMap : public Image
{

public:
    FranceMap(GfxSystem &s)
        : Image(s, "assets/story/france.png")
    {
        m_cursor_x = 0;
        m_cursor_y = 0;
        m_zoom = 1.0;
    }

    virtual void OnCreate(SDL_Renderer *renderer) override;

    virtual void Draw(SDL_Renderer *renderer) override;

    virtual void ProcessEvent(const SDL_Event &event) override;

    int GetWZoomed() { return mWZoomed; }
    int GetHZoomed() { return mHZoomed; }

    int GetCursorX() {return m_cursor_x; }
    int GetCursorY() {return m_cursor_y; }

private:
    float m_zoom;
    float m_cursor_x;
    float m_cursor_y;
    float m_map_x;
    float m_map_y;
    int mWZoomed;
    int mHZoomed;
};

class DenisHead : public Image
{

public:
    DenisHead(GfxSystem &s)
        : Image(s, "assets/story/denis_lunettes.png")
    {

    }
private:

};

class Velo : public Image
{

public:
    Velo(GfxSystem &s)
        : Image(s, "assets/story/velo.png")
    {

    }
private:

};

class StoryModeScene : public Scene
{
public:
    StoryModeScene(GfxSystem &system, IBoardEvent &event);

    virtual void OnCreate(SDL_Renderer *renderer) override;

    virtual void OnActivate(SDL_Renderer *renderer, const std::map<std::string, Value> &args) override;

    virtual void Update(double deltaTime) override;

    virtual void Draw(SDL_Renderer *renderer) override;

    virtual void ProcessEvent(const SDL_Event &event) override;

private:
    IBoardEvent &mEvent;

    DataBase mDb;

    double lon, lat;
    SDL_Point city;

    std::shared_ptr<Car> mCar;
    std::shared_ptr<FranceMap> m_map;
    std::shared_ptr<DenisHead> m_head;
    std::shared_ptr<Velo> m_velo;
};

#endif // STORYMODESCENE_H
