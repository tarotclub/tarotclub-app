#include "story-mode-scene.h"
#include "SDL2_gfxPrimitives.h"


#define earthRadiusKm 6371.0

// This function converts decimal degrees to radians
double deg2rad(double deg) {
  return (deg * M_PI / 180);
}

//  This function converts radians to decimal degrees
double rad2deg(double rad) {
  return (rad * 180 / M_PI);
}

/**
 * Returns the distance between two points on the Earth.
 * Direct translation from http://en.wikipedia.org/wiki/Haversine_formula
 * @param lat1d Latitude of the first point in degrees
 * @param lon1d Longitude of the first point in degrees
 * @param lat2d Latitude of the second point in degrees
 * @param lon2d Longitude of the second point in degrees
 * @return The distance between the two points in kilometers
 */
double distanceEarth(double lat1d, double lon1d, double lat2d, double lon2d) {
  double lat1r, lon1r, lat2r, lon2r, u, v;
  lat1r = deg2rad(lat1d);
  lon1r = deg2rad(lon1d);
  lat2r = deg2rad(lat2d);
  lon2r = deg2rad(lon2d);
  u = sin((lat2r - lat1r)/2);
  v = sin((lon2r - lon1r)/2);
  return 2.0 * earthRadiusKm * asin(sqrt(u * u + cos(lat1r) * cos(lat2r) * v * v));
}


/**
 * @param {number} latitude in degrees
 * @param {number} longitude in degrees
 * @param {number} mapWidth in pixels
 * @param {number} mapHeight in pixels
 */
SDL_Point latLonToOffsets(double latitude, double longitude, int mapWidth, int mapHeight) {
  const int FE = 180; // false easting
  const double radius = mapWidth / (2 * M_PI);

  const double latRad = deg2rad(latitude);
  const double lonRad = deg2rad(longitude + FE);

  const int x = lonRad * radius;

  const int yFromEquator = radius * log(tan(M_PI / 4 + latRad / 2));
  const int y = mapHeight / 2 - yFromEquator;

  return { x, y };
}

double france_lng_radius(double lng1, double lng2)
{
    const double radius = (abs(lng1 - lng2) / 360.0) ;
    return radius;
}


double meridien_distance(double lat1, double lat2)
{
    const double lat1Rad = deg2rad(lat1);
    const double lat2Rad = deg2rad(lat2);

    return earthRadiusKm * abs(lat1Rad - lat2Rad);
}

double parallele_distance(double lat, double lng1, double lng2)
{
    // On calcule le rayon du cercle au niveau de la latitude
    const double latRad = deg2rad(lat);
    double rayonLat = earthRadiusKm * cos(latRad);

    // Calcul du périmètre d’un arc de cercle sur cette parallèle
    double angle = abs(lng1 - lng2);
    double distance = (angle / 360) * 2 * M_PI * rayonLat;

    return distance;
}

double mercatorY(double lat)
{
    return log(tan(lat/2 + M_PI/4));
}




StoryModeScene::StoryModeScene(GfxSystem &system, IBoardEvent &event)
    : Scene(system)
    , mEvent(event)
{
    mMap = std::make_shared<FranceMap>(GetSystem());

    AddEntity(mMap);
}


void StoryModeScene::OnCreate(SDL_Renderer *renderer)
{
    Scene::OnCreate(renderer);

    if (mDb.Open("assets/data/communes.db"))
    {
        TLogInfo("[STORY] Database opened");
    }
    else
    {
        TLogError("[STORY] Cannot open database");
    }

    lon = 0.;
    lat = 0.;

    std::vector<std::vector<Value> > results;
    std::string ret = mDb.Query("SELECT lon, lat FROM communes WHERE nom='Limalonges';", results);

    std::cout << ret;

    if (results.size() > 0)
    {
        if (results[0].size() == 2)
        {
            lon = results[0][0].GetDouble();
            lat = results[0][1].GetDouble();

            // Centre : Saint-Amand-Montrond (46° 43′ 17″ N, 2° 30′ 37″ E)
            // Latitude : 46.721389 | Longitude : 2.510278



            // Distance Entre limalonges et le coin en haut à gauche :
            double distance = distanceEarth(lat, lon, 42.3333333, -4.795555555555556);
            std::cout << "LIMALONGES D: " << distance << std::endl;

            double west = deg2rad(-4.795555555555556);
            double east = deg2rad(8.230555555555556);
            double south = deg2rad(42.3327778);
            double north = deg2rad(51.0891667);
            double ymin = mercatorY(south);
            double ymax = mercatorY(north);

            double xFactor = mMap->GetWZoomed()/(east - west);
            double yFactor = mMap->GetHZoomed()/(ymax - ymin);

//            lon = 0;
            double limalongesW = (deg2rad(lon) - west) * xFactor;
            double limalongesH = (ymax - mercatorY(deg2rad(lat))) * yFactor;

            double franceH = meridien_distance(42.3327778, 51.0891667);
            limalongesH = meridien_distance(lat, 51.0891667);

            limalongesH = limalongesH * mMap->GetHZoomed() / franceH;

            std::cout << "LIMALONGES W: " << limalongesW << std::endl;
            std::cout << "LIMALONGES H: " << limalongesH << std::endl;

            city.x = limalongesW;
            city.y = limalongesH;

//            city = latLonToOffsets(46.721389, 2.510278, mMap->GetWZoomed(), mMap->GetHZoomed());
            TLogInfo("[STORY] Has results");
        }
    }
    else
    {
        TLogError("[STORY] No results");
    }
}

void StoryModeScene::OnActivate(SDL_Renderer *renderer)
{
    Scene::OnActivate(renderer);
}

void StoryModeScene::Draw(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer,  0x09, 0x72, 0x00, 255);
    // Clear the entire screen to our selected color.
    SDL_RenderClear(renderer);

    Scene::Draw(renderer);

  //  filledCircleRGBA(renderer, city.x , city.y, 6, 255, 0, 0, 255);

    vlineRGBA(renderer, city.x, 0, mMap->GetHZoomed(), 255, 0, 0, 255 );
    hlineRGBA(renderer, 0, mMap->GetWZoomed(), city.y, 255, 0, 0, 255 );

    vlineRGBA(renderer, mMap->GetWZoomed(), 0, mMap->GetHZoomed(), 255, 0, 0, 255 );
}

void StoryModeScene::ProcessEvent(const SDL_Event &event)
{
    Scene::ProcessEvent(event);
}



void FranceMap::OnCreate(SDL_Renderer *renderer)
{
    Image::OnCreate(renderer);

    // On va scale l'image pour qu'elle tienne entièrement dans la fenêtre, verticalement
    int w = GetSystem().GetWindowSize().w;
    int h = GetSystem().GetWindowSize().h;

    mZoom = (float)h / (GetRect().h);

    mWZoomed = GetRect().w * mZoom;
    mHZoomed = h;

    std::cout << "MAP SIZE: " << mWZoomed << ", " << mHZoomed << std::endl;
}

void FranceMap::Draw(SDL_Renderer *renderer)
{
    SetPos(mOffsetX, mOffsetY);
    SetScale(mZoom, mZoom);
    DrawEx(renderer, GetX(), GetY());


}

void FranceMap::ProcessEvent(const SDL_Event &event)
{
    int xMouse, yMouse;
    uint32_t mask = SDL_GetMouseState(&xMouse,&yMouse);
    if(event.type == SDL_MOUSEWHEEL)
    {
        if(event.wheel.y > 0) // scroll up
        {
            // Put code for handling "scroll up" here!
            mZoom += 0.1;
            mOffsetX = -(xMouse * 0.1);
            mOffsetY = -(yMouse * 0.1);
        }
        else if(event.wheel.y < 0) // scroll down
        {
            // Put code for handling "scroll down" here!
            mZoom -= 0.1;
            mOffsetX = (xMouse * 0.1);
            mOffsetY = (yMouse * 0.1);
        }

        std::cout << "Zoom: " << mZoom << std::endl;
    }

    if (event.type == SDL_MOUSEMOTION)
    {
        if ((mask & SDL_BUTTON_LMASK) == SDL_BUTTON_LMASK)
        {
            mOffsetX += event.motion.xrel;
            mOffsetY += event.motion.yrel;
        }
    }
}
