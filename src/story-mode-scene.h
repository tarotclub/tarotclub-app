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
        mOffsetX = 0;
        mOffsetY = 0;
        mZoom = 1.0;
    }

    virtual void OnCreate(SDL_Renderer *renderer) override;

    virtual void Draw(SDL_Renderer *renderer) override;

    virtual void ProcessEvent(const SDL_Event &event) override;

    int GetWZoomed() { return mWZoomed; }
    int GetHZoomed() { return mHZoomed; }

private:
    float mZoom;
    float mOffsetX;
    float mOffsetY;
    int mWZoomed;
    int mHZoomed;
};

class StoryModeScene : public Scene
{
public:
    StoryModeScene(GfxSystem &system, IBoardEvent &event);

    virtual void OnCreate(SDL_Renderer *renderer) override;

    virtual void OnActivate(SDL_Renderer *renderer) override;

    virtual void Update(double deltaTime) override;

    virtual void Draw(SDL_Renderer *renderer) override;

    virtual void ProcessEvent(const SDL_Event &event);

private:
    IBoardEvent &mEvent;

    DataBase mDb;

    double lon, lat;
    SDL_Point city;

    std::shared_ptr<Car> mCar;
    std::shared_ptr<FranceMap> mMap;
};

#endif // STORYMODESCENE_H
