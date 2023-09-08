#include "entity.h"

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

const SDL_Rect &Entity::GetRect() const
{
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
