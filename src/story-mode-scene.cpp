#include "story-mode-scene.h"

#include "scenes.h"
#include "IconsFontAwesome5.h"
#include "Log.h"

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

#include <random>

std::random_device rd;
std::mt19937 gen(rd());

int random(int low, int high)
{
    std::uniform_int_distribution<> dist(low, high);
    return dist(gen);
}


StoryModeScene::StoryModeScene(GfxSystem &system)
    : Scene(system)
{
    m_map = std::make_shared<FranceMap>(GetSystem());
    m_velo = std::make_shared<Velo>(GetSystem());
    m_denis = std::make_shared<Denis>(GetSystem(), *this);
    m_montargis = std::make_shared<Montargis>(GetSystem());
    
    AddEntity(m_map);

    AddEntity(m_montargis);
    AddEntity(m_velo);
    AddEntity(m_denis);

    m_denis->SetScale(0.15, 0.15);
    m_velo->SetScale(0.1, 0.1);
    m_montargis->SetScale(0.2, 0.2);

    m_questsTitle = std::make_shared<Text>(GetSystem(), "assets/fonts/Bullpen3D.ttf", "QUETES");
    m_questsTitle->SetVisible(false);
    AddEntity(m_questsTitle);

    m_infosTitle = std::make_shared<Text>(GetSystem(), "assets/fonts/Bullpen3D.ttf", "INFOS");
    m_infosTitle->SetVisible(false);
    AddEntity(m_infosTitle);

    m_pointsTitle = std::make_shared<Text>(GetSystem(), "assets/fonts/Bullpen3D.ttf", "POINTS");
    m_pointsTitle->SetVisible(false);
    AddEntity(m_pointsTitle);

    m_ivan = std::make_shared<Image>(GetSystem(), "assets/story/portrait_ivan.png");
    m_ivan->SetVisible(false);
    AddEntity(m_ivan);

    GetSystem().InitFont(0, "assets/fonts/roboto.ttf", 20);
    GetSystem().InitFont(1, "assets/fonts/Bullpen3D.ttf", 20);

    // Create cities
    for (int i = 0; i < 10; i++)
    {
        auto c = std::make_shared<City>(GetSystem(), *this, i);
        AddEntity(c);
        m_cities.push_back(c);
    }


    west = deg2rad(-4.795555555555556);
    east = deg2rad(8.230555555555556);
    south = deg2rad(42.3327778);
    north = deg2rad(51.0891667);
    ymin = mercatorY(south);
    ymax = mercatorY(north);

//    AddEntity(mCar);
}



void StoryModeScene::GeneratePath()
{
    std::vector<std::vector<Value> > results;


    // Start point : montargis
    std::string ret = mDb.Query("SELECT lon, lat FROM communes WHERE nom='Montargis';", results);

    if (results.size() > 0)
    {
        if (results[0].size() == 2)
        {
            m_denis->lon = results[0][0].GetDouble();
            m_denis->lat = results[0][1].GetDouble();

            m_montargis->lat = m_denis->lat;
            m_montargis->lon = m_denis->lon;

            SDL_Point p = GpsToPoint(lon, lat);
            m_denis->SetPos(p.x, p.y);
            m_montargis->SetPos(p.x, p.y);
        }
    }

    results.clear();
    ret = mDb.Query("SELECT COUNT(*) FROM communes;", results);

    if (results.size() > 0)
    {
        if (results[0].size() == 1)
        {
            int nbCities = results[0][0].GetInteger();

            int nb_points = 0;
            int tries = 0;
            do
            {
                results.clear();
                int line = random(1, nbCities);
                std::string ret = mDb.Query("SELECT * FROM communes LIMIT 1 OFFSET " + std::to_string(line) + ";", results);

                if (results.size() > 0)
                {
                    if (results[0].size() >= 3)
                    {
                        double lon = results[0][1].GetDouble();
                        double lat = results[0][2].GetDouble();
                        std::string city = results[0][0].GetString();
                        std::cout << city << std::endl;

                        // La ville doit être située à au moins 100 kms des autres villes
                        bool okay = true;

                        static const double distanceBetweenCities = 200.0;

                        for (int i = 0; i < nb_points; i++)
                        {
                            auto c = m_cities.at(i);

                            if (c->Initialized())
                            {
                                double distance = distanceEarth(c->lat, c->lon, lat, lon);

                               // std::cout << "Distance: " << distance << std::endl;
                                if (distance < distanceBetweenCities)
                                {
                                    okay = false;
                                }
                            }
                        }

                        // Also 200 kms from Montargis
                        {
                            double distance = distanceEarth(m_montargis->lat, m_montargis->lon, lat, lon);
                            if (distance < distanceBetweenCities)
                            {
                                okay = false;
                            }
                        }

                        if (okay)
                        {
                            auto c = m_cities.at(nb_points);

                            SDL_Point p = GpsToPoint(lon, lat);
                            c->Place(p.x, p.y);
                            c->SetName(city);
                            c->lon = lon;
                            c->lat = lat;

                            c->Initialize(true);

                            nb_points++;
                        }
                        else
                        {
                            tries++;
                            if (tries > 20)
                            {
                                tries = 0;
                                nb_points = 0; // on recommence
                                for (int i = 0; i < nb_points; i++)
                                {
                                    auto c = m_cities.at(i);
                                    c->Initialize(false);
                                }
                            }
                        }
                    }
                }

            } while (nb_points < 10);
        }
    }

}


SDL_Point StoryModeScene::GpsToPoint(double lon, double lat)
{
    SDL_Point p;
    double limalongesW = (deg2rad(lon) - west) * xFactor;
    double limalongesH = (ymax - mercatorY(deg2rad(lat))) * yFactor;

    double franceH = meridien_distance(42.3327778, 51.0891667);
    limalongesH = meridien_distance(lat, 51.0891667);

    limalongesH = limalongesH * m_map->GetHZoomed() / franceH;

    std::cout << "LIMALONGES W: " << limalongesW << std::endl;
    std::cout << "LIMALONGES H: " << limalongesH << std::endl;

    p.x = limalongesW;
    p.y = limalongesH;

    return p;
}

void StoryModeScene::OnCreate(SDL_Renderer *renderer)
{
    Scene::OnCreate(renderer);


    gMusic = Mix_LoadMUS( "assets/audio/Casa Bossa Nova.mp3" );
    if( gMusic == NULL )
    {
        printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );

    }


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
    std::string ret = mDb.Query("SELECT lon, lat FROM communes WHERE nom='Arcueil';", results);

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

            xFactor = m_map->GetWZoomed()/(east - west);
            yFactor = m_map->GetHZoomed()/(ymax - ymin);

//            lon = 0;
            double limalongesW = (deg2rad(lon) - west) * xFactor;
            double limalongesH = (ymax - mercatorY(deg2rad(lat))) * yFactor;

            double franceH = meridien_distance(42.3327778, 51.0891667);
            limalongesH = meridien_distance(lat, 51.0891667);
            
            limalongesH = limalongesH * m_map->GetHZoomed() / franceH;

            std::cout << "LIMALONGES W: " << limalongesW << std::endl;
            std::cout << "LIMALONGES H: " << limalongesH << std::endl;

            city.x = limalongesW;
            city.y = limalongesH;


         //   GeneratePath();

//            city = latLonToOffsets(46.721389, 2.510278, mMap->GetWZoomed(), mMap->GetHZoomed());
            TLogInfo("[STORY] Has results");
        }
    }
    else
    {
        TLogError("[STORY] No results");
    }
}

void StoryModeScene::OnActivate(SDL_Renderer *renderer, const std::map<std::string, Value> &args)
{
    Scene::OnActivate(renderer, args);

    GeneratePath();

    m_showPopup = true;
    m_points = 0;

    m_popupText = "Denis, nous avons besoin de toi ! Presstalis est en redressement judiciaire, ils ne distribuent que Valeurs Actuelles.\n"
                  "Ta mission sera de distribuer CanardPC aux abonnés. Débrouille toi, il faut terminer rapidement !\n"
                  "Je te dépose à Montargis, tiens, voici les magazines et un gravel Intersport.";

    m_quests.clear();
    m_quests.push_back(std::make_shared<MainQuest>(*this, "Distribuez tous les magazines"));
}

void StoryModeScene::Update(double deltaTime)
{
    Scene::Update(deltaTime);
  //  mCar->SetPos(200, 200);
    m_totalMagazines = 0;
    for (auto &c : m_cities)
    {
        if (c->IsDenisHere())
        {
            m_denis->SetPos(c->GetX(), c->GetY());
        }

        m_totalMagazines += c->GetMagazines();
    }

    m_velo->SetPos(m_denis->GetX(), m_denis->GetY() + 40);


    // Test fin de game, on
    for (auto &c : m_quests)
    {
        c->CheckObjective();

        if (c->GetFinished())
        {
            if (c->GetSuccess())
            {
                m_popupText = "Bravoooo Denis ! Tiens, une chambre à aire neuve.\nOn recommence le mois prochain ?";
                m_lastPopup = true;
                m_showPopup = true;
            }
            else
            {
                m_popupText = "Mince Denis ! Bon, on retente le mois prochain ok ?\n Prend un vélo Décath pour voir.";
                m_lastPopup = true;
                m_showPopup = true;
            }
        }
    }

}

void StoryModeScene::DrawQuestsMenu()
{
    // get the window size as a base for calculating widgets geometry
    int controls_width = 0;

    Rect r = GetSystem().GetWindowSize();

    static const uint32_t width = 400;
    static const uint32_t margins = 20;

    // position the controls widget in the top-right corner with some margin
    ImGui::SetNextWindowPos(ImVec2(r.w - width - margins, margins), ImGuiCond_Always);
    // here we set the calculated width and also make the height to be
    // be the height of the main window also with some margin
    ImGui::SetNextWindowSize(
        ImVec2(static_cast<float>(width), static_cast<float>(200)),
        ImGuiCond_Always
        );
    // create a window and append into it
    ImGui::Begin("Quests", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

    ImGui::Image((void*) m_questsTitle->GetTexture(), ImVec2(m_questsTitle->GetWidth(), m_questsTitle->GetHeight()));

    static ImGuiTableFlags flags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody;

    if (ImGui::BeginTable("table_quests", 3, flags))
    {
        ImGui::TableSetupColumn("Quête", ImGuiTableColumnFlags_WidthFixed, 150); // Default to 100.0f
        ImGui::TableSetupColumn("État", ImGuiTableColumnFlags_WidthFixed, 60); // Default to 200.0f
        ImGui::TableSetupColumn("Jours restants", ImGuiTableColumnFlags_WidthStretch);       // Default to auto
        ImGui::TableHeadersRow();
        for (auto &c : m_quests)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextWrapped("%s", c->GetDescription().c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", c->GetFinished() ? "Terminé" : "En cours");
            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", MinutesToString(c->GetMinutesLeft()).c_str());

        }
        ImGui::EndTable();
    }
/*
    if (ImGui::Button("Quitter", ImVec2(80, 40)))
    {
        SwitchToScene(SCENE_TITLE);
    }
    */

    ImGui::End();
}

void StoryModeScene::DrawInfosMenu()
{
    // get the window size as a base for calculating widgets geometry
    int controls_width = 0;

    Rect r = GetSystem().GetWindowSize();

    static const uint32_t width = 400;
    static const uint32_t margins = 20;

    // position the controls widget in the top-right corner with some margin
    ImGui::SetNextWindowPos(ImVec2(r.w - width - margins, 240), ImGuiCond_Always);
    // here we set the calculated width and also make the height to be
    // be the height of the main window also with some margin
    ImGui::SetNextWindowSize(
        ImVec2(static_cast<float>(width), static_cast<float>(350)),
        ImGuiCond_Always
        );
    // create a window and append into it
    ImGui::Begin("Infos", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

    ImGui::Image((void*) m_infosTitle->GetTexture(), ImVec2(m_infosTitle->GetWidth(), m_infosTitle->GetHeight()));

    ImGui::Text("Sélection : %s", m_currentSelection.c_str());
    ImGui::Text("Distance : %f km", m_distanceVoyage);
    ImGui::Text("Total à distribuer : %d", m_totalMagazines);

    float duree = m_distanceVoyage / 20.0;
    duree *= 60.0; // en minutes
    ImGui::Text("Temps de voyage : %s", MinutesToString(duree).c_str());

    if (m_currentSelection != "-")
    {
        if (!m_denis->IsRouling())
        {
            float besoinEnergie = (duree / 60) * 5.0;

            if ((m_endurance > 0.0) && ((m_endurance - besoinEnergie) > 0.0))
            {
                if (m_distanceVoyage > 0.0)
                {
                    if (ImGui::Button("Pédaler ici"))
                    {
                        // On retire Denis de la ville
                        for (auto &c : m_cities)
                        {
                            c->SetDenisInCity(false);
                        }

                        if (m_citySel)
                        {
                            m_denis->MoveTo(m_citySel->GetX(), m_citySel->GetY());
                            m_denis->m_inCity = m_citySel;

                            m_endurance -= besoinEnergie;
                            if (m_endurance < 0.0)
                            {
                                m_endurance = 0.0;
                            }

                            // On diminue toutes les quêtes de cette durée
                            for (auto &c : m_quests)
                            {
                                c->AddMinutes(duree * -1);
                            }

                        }
                    }
                    ImGui::SameLine();
                }


            }
            else
            {
                ImGui::Text("Vous êtes trop fatigué pour bouger ou aller ici");
            }

            if (ImGui::Button("Dormir 8h"))
            {
                m_endurance +=  8 * 6.0; // 8h * 6% par heure
                if (m_endurance >= 100.0)
                {
                    m_endurance = 100.0;
                }

                // On diminue toutes les quêtes de cette durée
                for (auto &c : m_quests)
                {
                    c->AddMinutes(8 * 60 * -1);
                }
            }

            if (m_denis->m_inCity && (m_endurance > 0.0))
            {
                if (ImGui::Button("Distributer!"))
                {
                    int mags = m_denis->m_inCity->GetMagazines();
                    m_points += 526 * mags;
                    m_denis->m_inCity->SetMagazines(0);
                    m_endurance -= mags / 10;
                }
            }

//            ImGui::SameLine();

//            if (m_endurance > 0.0)
//            {
//                if (ImGui::Button("Acheter une piscine"))
//                {

//                }
//            }

        }
    }

    ImGui::Text("Endurance");
    ImGui::ProgressBar(m_endurance/100, ImVec2(0.0f, 0.0f));

    ImGui::Image((void*) m_pointsTitle->GetTexture(), ImVec2(m_pointsTitle->GetWidth(), m_pointsTitle->GetHeight()));

    ImGui::SameLine();

    GfxEngine::PushBigFont();
    ImGui::Text("%d", m_points);
    GfxEngine::PopBigFont();

    ImGui::End();
}



void StoryModeScene::DrawToolBar()
{
    // get the window size as a base for calculating widgets geometry
    int controls_width = 0;

    Rect r = GetSystem().GetWindowSize();

    static const uint32_t width = 200;
    static const uint32_t height = 45;

    // position the controls widget in the top-right corner with some margin
    ImGui::SetNextWindowPos(ImVec2(r.w - width, r.h - height), ImGuiCond_Always);
    // here we set the calculated width and also make the height to be
    // be the height of the main window also with some margin
    ImGui::SetNextWindowSize(
        ImVec2(static_cast<float>(width), static_cast<float>(height)),
        ImGuiCond_Always
        );
    // create a window and append into it
    ImGui::Begin("ToolBar", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);

    if (ImGui::Button(ICON_FA_SIGN_OUT_ALT))
    {
        SwitchToScene(SCENE_TITLE);
    }

    ImGui::End();
}

void StoryModeScene::DrawPopupEvent()
{
    // get the window size as a base for calculating widgets geometry
    int controls_width = 0;

    Rect r = GetSystem().GetWindowSize();

    static const uint32_t width = 400;
    static const uint32_t margins = 20;

    if (m_showPopup)
    {
        ImGui::OpenPopup("Évènement !");
    }

    if (ImGui::BeginPopupModal("Évènement !", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Image((void*) m_ivan->GetTexture(), ImVec2(m_ivan->GetWidth(), m_ivan->GetHeight()));

        ImGui::Text("%s", m_popupText.c_str());
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)))
        {
            m_showPopup = false;
            ImGui::CloseCurrentPopup();
            if (m_lastPopup)
            {
                m_lastPopup = false;
                SwitchToScene(SCENE_TITLE);
            }
        }
        ImGui::SetItemDefaultFocus();

        ImGui::EndPopup();
    }
}



void StoryModeScene::Draw(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer,  0xdd, 0xea, 0xfa, 255);
    // Clear the entire screen to our selected color.
    SDL_RenderClear(renderer);

    Scene::Draw(renderer);  


    DrawQuestsMenu();
    DrawInfosMenu();
    DrawToolBar();
    DrawPopupEvent();

  //  GetSystem().DrawText("Points", 50, 50, 255, 255, 255, 1);

}

void StoryModeScene::ProcessEvent(const SDL_Event &event)
{
    Scene::ProcessEvent(event);
}

void StoryModeScene::SelectCity(int id)
{

    for (auto c : m_cities)
    {
        c->SetSelected(c->GetId() == id);

        if (c->GetId() == id)
        {
            m_currentSelection = c->GetCityName();
            m_citySel = c;
        }
    }

    if (m_citySel)
    {
        m_distanceVoyage = distanceEarth(m_denis->lat, m_denis->lon, m_citySel->lat, m_citySel->lon);
    }
}

std::string StoryModeScene::MinutesToString(float decimalMinutes) const
{
    int inputMinutes;
    double decPart;

    inputMinutes = decimalMinutes;
    decPart = decimalMinutes-inputMinutes;

    //get number of days
    int days = inputMinutes / 1440;
    inputMinutes = inputMinutes % 1440;

    //get number of hours
    int hours = inputMinutes / 60;

    //get number of minutes
    int mins = inputMinutes % 60;

    return std::to_string(days) + " j " + std::to_string(hours) + " h " + std::to_string(mins) + " m";
}

int StoryModeScene::GetTotalMagazines() const
{
    int total = 0;
    for (auto &c : m_cities)
    {
        total += c->GetMagazines();
    }
    return total;
}

void StoryModeScene::DenisArrivedInCity()
{
    m_denis->m_inCity->SetDenisInCity(true);
    m_denis->lon =  m_denis->m_inCity->lon;
    m_denis->lat =  m_denis->m_inCity->lat;
    m_distanceVoyage = 0.0;
}

SDL_Rect world = {0, 0, 1728, 972};

void FranceMap::OnCreate(SDL_Renderer *renderer)
{
    Image::OnCreate(renderer);

    // On va scale l'image pour qu'elle tienne entièrement dans la fenêtre, verticalement
    int w = GetSystem().GetWindowSize().w;
    int h = GetSystem().GetWindowSize().h;

    m_zoom = (float)h / (GetRect().h);

    mWZoomed = GetRect().w * m_zoom;
    mHZoomed = h;

    m_cursor_x = 0.0;
    m_cursor_y = 0.0;

    m_map_x = 0.0;
    m_map_y = 0.0;

    std::cout << "MAP SIZE: " << mWZoomed << ", " << mHZoomed << std::endl;
}

void FranceMap::Draw(SDL_Renderer *renderer)
{
  //  m_map_x = m_map_x - (m_cursor_x / GetSystem().GetWindowSize().w * (m_map_x - GetRect().w));
   // m_map_x = m_map_x - (m_cursor_x / GetSystem().GetWindowSize().w * (m_map_x - GetRect().w));

    SetPos(m_map_x + m_cursor_x, m_map_y + m_cursor_y);
    SetScale(m_zoom, m_zoom);
    DrawEx(renderer, GetX(), GetY());


}

void FranceMap::ProcessEvent(const SDL_Event &event)
{
    int xMouse, yMouse;
    uint32_t mask = SDL_GetMouseState(&xMouse,&yMouse);

    /*
    if(event.type == SDL_MOUSEWHEEL)
    {
        if(event.wheel.y > 0) // scroll up
        {
            // Put code for handling "scroll up" here!
            if (m_zoom < 1.8)
            {
                m_zoom += 0.1;
            }
            m_cursor_x = xMouse;
            m_cursor_y = yMouse;
        }
        else if(event.wheel.y < 0) // scroll down
        {
            // Put code for handling "scroll down" here!
            if (m_zoom > 0.3)
            {
                m_zoom -= 0.1;
            }
            m_cursor_x = xMouse;
            m_cursor_y = yMouse;
        }
    }*/

    if (event.type == SDL_MOUSEMOTION)
    {
        /*
        if ((mask & SDL_BUTTON_LMASK) == SDL_BUTTON_LMASK)
        {
            m_cursor_x += event.motion.xrel;
            m_cursor_y += event.motion.yrel;

            if (m_cursor_x > (GetSystem().GetWindowSize().w / 4))
            {
                m_cursor_x = (GetSystem().GetWindowSize().w / 4);
            }

            if (m_cursor_x < -(GetSystem().GetWindowSize().w / 4))
            {
                m_cursor_x = -(GetSystem().GetWindowSize().w / 4);
            }

            if (m_cursor_y > (GetSystem().GetWindowSize().h / 4))
            {
                m_cursor_y = (GetSystem().GetWindowSize().h / 4);
            }

            if (m_cursor_y < -(GetSystem().GetWindowSize().h / 4))
            {
                m_cursor_y = -(GetSystem().GetWindowSize().h / 4);
            }


        }*/
    }
    std::cout << "Zoom: " << m_zoom << " cursor X: " << m_cursor_x << " cursor Y: " << m_cursor_y << std::endl;
}

std::string replaceStringAll(std::string str, const std::string& old, const std::string& new_s)
{
    if(!old.empty())
    {
        size_t pos = str.find(old);
        while ((pos = str.find(old, pos)) != std::string::npos) {
             str = str.replace(pos, old.length(), new_s);
             pos += new_s.length();
        }
    }
    return str;
}


City::City(GfxSystem &s, IFranceObject &ev, int id)
    : Image(s, "assets/story/city.png")
    , m_ev(ev)
    , m_id(id)
{
    m_selection = std::make_shared<Image>(GetSystem(), "assets/story/selection.png");
    m_selection->SetVisible(false);
    AddChildEntity(m_selection);
}

void City::Initialize(bool initialized) {
    m_magazines = random(60, 150);
    m_denisIsHere = false;
    m_initialized = initialized;
}

void City::OnCreate(SDL_Renderer *renderer)
{
    Image::OnCreate(renderer);

    SetScale(0.1, 0.1);
}

void City::ProcessEvent(const SDL_Event &event)
{
    SDL_Point mousePos;
    mousePos.x = event.motion.x;
    mousePos.y = event.motion.y;

    if (event.type == SDL_MOUSEMOTION)
    {
        SetHovered(false);

        if (SDL_PointInRect(&mousePos, &m_scaledRect))
        {
            SetHovered(true);
        }
    }
    if (event.type == SDL_MOUSEBUTTONUP)
    {      
        if (event.button.button == SDL_BUTTON_LEFT)
        {
            if (SDL_PointInRect(&mousePos, &m_scaledRect))
            {
                m_ev.SelectCity(m_id);
            }
        }
    }

}

void City::Update(double deltaTime)
{
    Image::Update(deltaTime);
}

void City::Draw(SDL_Renderer *renderer)
{
    Image::Draw(renderer);


    ImDrawList* dl = ImGui::GetBackgroundDrawList();
    ImVec2 v{(float)m_x, (float)m_y};
    dl->AddText(v, IM_COL32_BLACK, std::to_string(m_magazines).c_str(), nullptr);
}

Quest::Quest(const std::string &description)
    : m_description(description)
{

}
