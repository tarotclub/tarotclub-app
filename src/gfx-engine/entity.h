#ifndef ENTITY_H
#define ENTITY_H

#include <iostream>
#include <string>

#include "SDL.h"

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

    virtual void OnCreate(SDL_Renderer *renderer) { (void) renderer; }

    // Manage user interaction (mouse, keyboard...)
    virtual void ProcessEvent(const SDL_Event &event);

    // Update state
    virtual void Update(double deltaTime);

    // Draw to screen
    virtual void Draw(SDL_Renderer *renderer);

    bool IsVisible() const;


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

    uint32_t mSceneIdOnwer = 0;
    uint32_t mId = 0;
    uint32_t mZ = 0; // pseudo Z value (order of drawing)

    bool mVisible = true;
    SDL_Rect mRect;
 //   Vector2f mPos;
    float mAngle = 0;
    Vector2f mScale = Vector2f(1, 1);
};

#endif // ENTITY_H
