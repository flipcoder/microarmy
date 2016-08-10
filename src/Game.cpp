#include "Game.h"
#include "Thing.h"
#include "Monster.h"
#include "Player.h"
#include "Qor/BasicPartitioner.h"
#include "Qor/Input.h"
#include "Qor/Qor.h"
#include "Qor/Shader.h"
#include <glm/glm.hpp>
#include <cstdlib>
#include <chrono>
#include <thread>

using namespace std;
using namespace glm;


namespace _ = std::placeholders;

Game :: Game(Qor* engine):
    m_pQor(engine),
    m_pResources(engine->resources()),
    m_pInput(engine->input()),
    m_pRoot(make_shared<Node>()),
    m_pPipeline(engine->pipeline()),
    m_pPartitioner(engine->pipeline()->partitioner()),
    m_pController(engine->session()->active_profile(0)->controller()),
    m_pTimeline(engine->timer()->timeline())
{}


void Game :: preload() {
    float sw = m_pQor->window()->size().x;
    float sh = m_pQor->window()->size().y;

    m_pCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pRoot->add(m_pCamera);
    
    m_pHUD = make_shared<HUD>(
        m_pQor->window(),
        m_pQor->input(),
        m_pQor->resources()
    );

    m_pOrthoCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pOrthoRoot = make_shared<Node>();
    m_pOrthoCamera->ortho(false);
    m_pOrthoRoot->add(m_pHUD);

    string lev = m_pQor->args().value("map");
    
    if (lev.empty())
        lev = "1";
    
    m_pMap = m_pQor->make<TileMap>(lev + ".tmx");
    m_pRoot->add(m_pMap);
    
    m_pMusic = m_pQor->make<Sound>(lev + ".ogg");
    m_pRoot->add(m_pMusic);
    
    auto scale = 250.0f / std::max<float>(sw* 1.0f, 1.0f);
    m_pCamera->rescale(glm::vec3(scale, scale, 1.0f));

    m_pChar = make_shared<Player>(
        m_pQor->resource_path("guy.json"),
        m_pResources,
        m_pCamera.get(),
        m_pController.get(),
        m_pTimeline,
        m_pPartitioner,
        this
    );
    //m_pChar->texture()->ambient(Color::red(1.0f)*0.5f);
    m_pRoot->add(m_pChar);
    
    m_Players.push_back(m_pChar);
    
    m_pCamera->mode(Tracker::FOLLOW);
    //m_pCamera->threshold(1.0f);
    m_pCamera->track(m_pChar->focus_right());
    m_pCamera->focus_time(Freq::Time::ms(200));
    m_pCamera->focal_offset(vec3(
        -m_pQor->window()->center().x * 1.0f,
        -m_pQor->window()->center().y * 1.2f,
        0.0f
    ));
    m_pCamera->listen(true);

    m_pViewLight = make_shared<Light>();
    m_pViewLight->ambient(Color::white() * 0.50f);
    m_pViewLight->diffuse(Color::white());
    m_pViewLight->specular(Color::white());
    m_pViewLight->dist(sw / 1.5f);
    m_pViewLight->position(glm::vec3(
        m_pQor->window()->center().x * 1.0f,
        m_pQor->window()->center().y * 1.2f,
        1.0f
    ));
    m_pCamera->add(m_pViewLight);

    m_Stars = { 0, 0, 0 };
    m_MaxStars = { 0, 0, 0 };
    
    vector<vector<shared_ptr<TileLayer>>*> layer_types {
        &m_pMap->layers(),
        &m_pMap->object_layers()
    };

    for (auto&& layers: layer_types) {
        for (auto&& layer: *layers) {
            layer->set_main_camera(m_pCamera.get());
            layer->bake_visible();

            auto camera = m_pCamera.get();
            m_pCamera->on_move.connect([layer, camera]{
                layer->bake_visible();
            });
            
            if (layer->config()->has("parallax")) {
                float parallax = boost::lexical_cast<float>(
                    layer->config()->at<string>("parallax", "1.0")
                );
                Color color = Color(
                    layer->config()->at<string>("color", "ffffff")
                );
                auto pl = ParallaxLayer();

                pl.root = layer;
                pl.scale = parallax;

                m_ParallaxLayers.push_back(pl);
                auto l = make_shared<Light>();
                l->ambient(color);
                l->diffuse(color);
                l->specular(color);
                l->dist(10000.0f);
                pl.root->add(l);
                pl.light = l;

                continue;
            }
            
            for (auto&& tile_ptr: layer->all_descendants()) {
                if (not tile_ptr)
                    continue;

                auto tile = tile_ptr->as_node();
                // read object properties and replace
                auto obj = std::dynamic_pointer_cast<MapTile>(tile);

                if(obj) {
                    auto obj_cfg = obj->config();
                    obj->box() = obj->mesh()->box();
                    auto name = obj_cfg->at<string>("name", "");

                    if(name=="spawn") {
                        obj->visible(false);
                        obj->mesh()->visible(false);
                        m_Spawns.push_back(obj.get());
                        
                        continue;

                    } else if (name=="altspawn") {
                        obj->visible(false);
                        obj->mesh()->visible(false);
                        m_AltSpawns.push_back(obj.get());

                        continue;

                    } else if (Monster::get_type(obj_cfg)) {
                        auto monster = make_shared<Monster>(
                            obj_cfg,
                            obj.get(),
                            this,
                            m_pMap.get(),
                            m_pPartitioner,
                            m_pQor->timer()->timeline(),
                            m_pQor->resources()
                        );
                        obj->add(monster);
                        setup_monster(monster);
                        continue;
                    } else if (Thing::get_id(obj_cfg)) {
                        if (name == "star") {
                            auto typ = obj_cfg->at<string>("type");

                            if (typ == "bronze")
                                ++m_MaxStars[0];
                            else if (typ == "silver")
                                ++m_MaxStars[1];
                            else if (typ == "gold")
                                ++m_MaxStars[2];
                        }
                        
                        auto thing = make_shared<Thing>(
                            obj_cfg,
                            obj.get(),
                            this,
                            m_pMap.get(),
                            m_pPartitioner,
                            m_pQor->timer()->timeline(),
                            m_pQor->resources()
                        );
                        obj->add(thing);
                        setup_thing(thing);

                        continue;
                    }

                    bool depth = layer->depth() || obj->config()->has("depth");

                    if (depth) {
                        m_pMainLayer = layer.get();

                        // make a provider function that queries the map layer
                        // for currently visible objects identifiable by a string
                        // in their config
                        auto provider_for = [layer](string s){
                            return [s,layer](Box box){
                                vector<std::weak_ptr<Node>> r;
                                auto nodes = layer->query(box, [s](Node* n){
                                    return n->config()->has(s);
                                });
                                std::transform(ENTIRE(nodes), back_inserter(r), [](Node* n){
                                    return std::weak_ptr<Node>(n->as_node());
                                });

                                return r;
                            };
                        };

                        m_pPartitioner->register_provider(STATIC, provider_for("static"));
                        m_pPartitioner->register_provider(LEDGE, provider_for("ledge"));
                        m_pPartitioner->register_provider(FATAL, provider_for("fatal"));
                        
                        auto n = make_shared<Node>();
                        n->name("mask");
                        auto mask = obj_cfg->at<shared_ptr<Meta>>("mask", shared_ptr<Meta>());
                        bool hflip = obj->orientation() & (unsigned)MapTile::Orientation::H;
                        bool vflip = obj->orientation() & (unsigned)MapTile::Orientation::V;

                        if (mask && mask->size() == 4) {
                            n->box() = Box(
                                vec3(mask->at<double>(0), mask->at<double>(1), -5.0f),
                                vec3(mask->at<double>(2), mask->at<double>(3), 5.0f)
                            );
                        } else {
                            n->box() = Box(
                                vec3(0.0f, 0.0f, -5.0f),
                                vec3(1.0f, 1.0f, 5.0f)
                            );
                        }

                        if(hflip) {
                            n->box().min().x = 1.0f - n->box().min().x;
                            n->box().max().x = 1.0f - n->box().max().x;
                        }

                        if(vflip) {
                            n->box().min().y = 1.0f - n->box().min().y;
                            n->box().max().y = 1.0f - n->box().max().y;
                        }

                        obj->mesh()->add(n);

                        if (obj_cfg->has("fatal")) {
                            obj_cfg->set<string>("fatal", "");
                        }

                        else if (obj_cfg->has("ledge"))
                            obj_cfg->set<string>("ledge", "");
                        else {
                            obj_cfg->set<string>("static", "");
                        }
                    }
                }
            }
        }
    }

    m_pHUD->set(m_StarLevel, m_Stars[0], m_MaxStars[0]);

    for (auto&& player: m_Players) {
        auto _this = this;

        player->event("battery", [_this, &player](const shared_ptr<Meta>&){
            player->battery(1);
        });
        player->event("star", [_this](const shared_ptr<Meta>& m){
            auto typs = m->at<string>("type");
            int typ = 0;

            if(typs == "bronze")
                typ = 0;
            else if(typs == "silver")
                typ = 1;
            else if(typs == "gold")
                typ = 2;
            ++_this->m_Stars[typ];
            
            int stars=0;
            int max_stars=0;
            
            do {
                stars = _this->m_Stars[_this->m_StarLevel];
                max_stars = _this->m_MaxStars[_this->m_StarLevel];

                if (stars == max_stars)
                    _this->m_StarLevel = std::min<int>(_this->m_StarLevel + 1, 2);

            } while (stars == max_stars && _this->m_StarLevel <= 1);
            
            _this->m_pHUD->set(_this->m_StarLevel, stars, max_stars);
        });

        event("stardoor", [_this](const shared_ptr<Meta>& m){
            if (_this->m_Stars[0] == _this->m_MaxStars[0]) {
                auto mapname = _this->m_pQor->args().value_or("map","1");
                auto nextmap = to_string(boost::lexical_cast<int>(mapname) + 1);

                _this->m_pQor->args().set("map", nextmap);
                _this->m_pQor->change_state("pregame");
            }
        });

        setup_player(player);

    //// TESTING
    //auto square = make_shared<Mesh>(
    //    make_shared<MeshGeometry>(Prefab::quad(vec2(-50.0,-50.0),vec2(50.0,50.0))),
    //    vector<shared_ptr<IMeshModifier>>{
    //        make_shared<Wrap>(Prefab::quad_wrap(
    //            glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 0.0f)
    //        ))
    //    },
    //    make_shared<MeshMaterial>("duck.png", m_pResources)
    //);

    //square->position(vec3(250.0f, 250.0f, 0.0f));

    //m_pCamera->add(square);

    //auto line = Mesh::line(
    //    vec3(0.0f, 0.0f, 0.0f), // start
    //    vec3(100.0f, 100.0f, 0.0f), // end
    //    m_pResources->cache_as<Texture>("white.png"), // tex
    //    2.0f // width
    //);

    //m_pCamera->add(line);
    //line->material()->emissive(Color::white());
    //// END TESTING
    }

    reset();

    m_pPartitioner->on_collision(
        CHARACTER, STATIC, std::bind(&Game::cb_to_tile, this, _::_1, _::_2)
    );
    m_pPartitioner->on_collision(
        CHARACTER, LEDGE, std::bind(&Game::cb_to_ledge, this, _::_1, _::_2)
    );
    m_pPartitioner->on_collision(
        CHARACTER, FATAL, std::bind(&Game::cb_to_fatal, this, _::_1, _::_2)
    );
    m_pPartitioner->on_collision(
        BULLET, STATIC, std::bind(&Game::cb_bullet_to_static, this, _::_1, _::_2)
    );
    
    m_pPartitioner->on_collision(
        THING, STATIC, std::bind(&Thing::cb_to_static, _::_1, _::_2)
    );
    m_pPartitioner->on_collision(
        THING, FATAL, std::bind(&Thing::cb_to_static, _::_1, _::_2)
    );
    m_pPartitioner->on_collision(
        THING, BULLET, std::bind(&Thing::cb_to_bullet, _::_1, _::_2)
    );
    m_pPartitioner->on_collision(
        CHARACTER, THING, std::bind(&Thing::cb_to_player, _::_1, _::_2)
    );
    
    m_pPartitioner->on_collision(
        MONSTER, STATIC, std::bind(&Monster::cb_to_static, _::_1, _::_2)
    );
    m_pPartitioner->on_collision(
        MONSTER, FATAL, std::bind(&Monster::cb_to_static, _::_1, _::_2)
    );
    m_pPartitioner->on_collision(
        MONSTER, BULLET, std::bind(&Monster::cb_to_bullet, _::_1, _::_2)
    );
    m_pPartitioner->on_collision(
        CHARACTER, MONSTER, std::bind(&Monster::cb_to_player, _::_1, _::_2)
    );
    m_pPartitioner->on_collision(
        CHARACTER, BULLET, std::bind(&Player::cb_to_bullet, _::_1, _::_2)
    );

    //m_pPartitioner->on_collision(
    //    SENSOR, STATIC,
    //        std::function<void(Node*,Node*)>(), // col
    //        std::bind(&Monster::cb_sensor_to_no_static, _::_1, _::_2) // no col
    //);

    //m_JumpTimer.set(Freq::Time::ms(0));
    //m_ShootTimer.set(Freq::Time::ms(0));
}


void Game :: reset() {
    try {
        m_pChar->position(m_Spawns.at(0)->position());
    } catch(...) {
        WARNING("Map has no spawn points");
    }

    m_pChar->move(glm::vec3(0.0f, 0.0f, 1.0f));

    on_reset();
}


Game :: ~Game() {
    m_pPipeline->partitioner()->clear();
}


void Game :: setup_thing(std::shared_ptr<Thing> thing) {
    thing->initialize();
    m_Things.push_back(thing);

    for(auto&& player: m_Players)
        setup_player_to_thing(player,thing);
}

void Game :: setup_monster(std::shared_ptr<Monster> monster) {
    monster->initialize();
    m_Monsters.push_back(monster);

    for(auto&& player: m_Players)
        setup_player_to_monster(player,monster);
}


void Game :: setup_player(std::shared_ptr<Player> player) {

    // create masks
    auto n = make_shared<Node>();
    n->name("mask");
    n->box() = Box(
        vec3(-4.0f, -12.0f, -5.0f),
        vec3(4.0f, 2.0f, 5.0f)
    );

    player->add(n);
    n->config()->set<Player*>("player", player.get());
    m_pPartitioner->register_object(n, CHARACTER);

    // create masks
    n = make_shared<Node>();
    n->name("feetmask");
    n->box() = Box(
        vec3(-4.0f, 0.0f, -5.0f),
        vec3(4.0f, 4.0f, 5.0f)
    );

    player->add(n);
    m_pPartitioner->register_object(n, CHARACTER_FEET);

    n = make_shared<Node>();
    n->name("sidemask");
    n->box() = Box(
        vec3(-8.0f, -10.0f, -5.0f),
        vec3(8.0f, -2.0f, 5.0f)
    );

    player->add(n);
    m_pPartitioner->register_object(n, CHARACTER_SIDES);

    //setup_player_to_map(player);

    // // TESTING - ADD BOX AROUND PLAYER

    // // Draw a white wireframe box around the player
    // auto origin = vec2(player->origin()) ; // returns vec3
    // auto size = vec2(player->size()); // returns vec3

    // auto min = -size * origin; // top left point
    // auto max = size * vec2(1.0f - origin.x, 1.0f - origin.y); // bottom right point

    // LOGf("Player origin: %s", Vector::to_string(origin));
    // LOGf("Player size: %s", Vector::to_string(size));

    // // Getting corners
    // std::vector<glm::vec2> coordinate_list;

    // // Starting at top left going clockwise
    // coordinate_list.push_back(vec2(min.x, min.y));
    // coordinate_list.push_back(vec2(max.x, min.y));
    // coordinate_list.push_back(vec2(max.x, max.y));
    // coordinate_list.push_back(vec2(min.x, max.y));


    // LOGf("Min: (%s, %s)", min.x % min.y);
    // LOGf("Max: (%s, %s)", max.x % max.y);

    // LOGf("Coordinate 1: (%s, %s)", coordinate_list[0].x % coordinate_list[0].y);
    // LOGf("Coordinate 2: (%s, %s)", coordinate_list[1].x % coordinate_list[1].y);
    // LOGf("Coordinate 3: (%s, %s)", coordinate_list[2].x % coordinate_list[2].y);
    // LOGf("Coordinate 4: (%s, %s)", coordinate_list[3].x % coordinate_list[3].y);

    // LOGf("Coordinate 1 (World Space): (%s, %s)", coordinate_list[0].x % coordinate_list[0].y);
    // LOGf("Coordinate 2 (World Space): (%s, %s)", coordinate_list[1].x % coordinate_list[1].y);
    // LOGf("Coordinate 3 (World Space): (%s, %s)", coordinate_list[2].x % coordinate_list[2].y);
    // LOGf("Coordinate 4 (World Space): (%s, %s)", coordinate_list[3].x % coordinate_list[3].y);

    // auto root_node = m_pRoot;

    // for(std::vector<glm::vec2>::iterator coord = coordinate_list.begin(); coord != coordinate_list.end(); ++coord) {
        
    //    // if not the last coordinate
    //    if (coord + 1 != coordinate_list.end()) {

    //        // Drawing Lines
    //        auto line = Mesh::line(
    //            vec3(coord->x, coord->y, 0.0f), // start
    //            vec3((coord + 1)->x, (coord + 1)->y, 0.0f), // end
    //            m_pResources->cache_as<Texture>("white.png"), // tex
    //            1.0f // width
    //        );
    //        root_node->add(line->position(Space::WORLD));
    //    }
    //    else {
    //        auto line = Mesh::line(
    //            vec3(coord->x, coord->y, 0.0f), // start
    //            vec3(coordinate_list[0].x, coordinate_list[0].y, 0.0f), // end
    //            m_pResources->cache_as<Texture>("white.png"), // tex
    //            1.0f // width
    //        );

    //        root_node->add(line->position(Space::WORLD));
    //    }
    // }

    //// END TESTING

    for (auto&& thing: m_Things)
        setup_player_to_thing(player, thing);
}


void Game :: setup_player_to_thing(std::shared_ptr<Player> player, std::shared_ptr<Thing> thing) {}
void Game :: setup_player_to_monster(std::shared_ptr<Player> player, std::shared_ptr<Monster> monster) {}


std::vector<Node*> Game :: get_static_collisions(Node* a) {
    auto static_cols = m_pPartitioner->get_collisions_for(a, STATIC);
    auto ledge_cols = m_pPartitioner->get_collisions_for(a, LEDGE);
    
    auto m = a->parent();
    vec3 old_pos;
    if (m->num_snapshots())
        old_pos = Matrix::translation(kit::safe_ptr(m->snapshot(0))->world_transform);
    else
        old_pos = m->position(Space::WORLD);
    
    std::copy_if(ENTIRE(ledge_cols), std::back_inserter(static_cols), [old_pos,a](Node* n){
        return old_pos.y <= n->position(Space::WORLD).y;
    });

    return static_cols;
}


void Game :: cb_to_static(Node* a, Node* b, Node* m) {
    if (not m)
        m = a;
    
    auto p = m->position(Space::PARENT);
    auto v = m->velocity();
    auto col = [this, a, b]() -> bool {
        return (not get_static_collisions(a).empty()) ||
            a->world_box().collision(b->world_box());
    };

    //Box overlap = a->world_box().intersect(b->world_box());
    vec3 old_pos;
    try {
        old_pos = Matrix::translation(kit::safe_ptr(m->snapshot(0))->world_transform);
    } catch(...) {
        return;
    }

    auto np = vec3(p.x, old_pos.y, p.z);
    m->position(np);

    if(not col()){
        m->velocity(glm::vec3(v.x, 0.0f, v.z));
        return;
    }

    np = vec3(old_pos.x, p.y, p.z);
    m->position(np);

    if(not col())
        return;

    m->position(vec3(old_pos.x, old_pos.y, p.z));
    m->velocity(glm::vec3(0.0f));
}


void Game :: cb_to_ledge(Node* a, Node* b) {
    auto m = a->parent();
    auto old_pos = Matrix::translation(kit::safe_ptr(m->snapshot(0))->world_transform);

    if (m->velocity().y >= K_EPSILON and old_pos.y <= b->position(Space::WORLD).y)
        cb_to_static(a, b, a->parent());
}


void Game :: cb_to_tile(Node* a, Node* b) {
    cb_to_static(a, b, a->parent());
}


void Game :: cb_to_fatal(Node* a, Node* b) {
    Sound::play(m_pCamera.get(), "die.wav", m_pResources);
    reset();
    m_pChar->velocity(glm::vec3(0.0f));
}


void Game :: cb_bullet_to_static(Node* a, Node* b) {
    Sound::play(a, "hit.wav", m_pResources);
    a->safe_detach();
}


void Game :: enter() {
    m_pMusic->play();
    
    if(m_pQor->args().has("--low"))
        m_Shader = m_pPipeline->load_shaders({"lit"});
    else
        m_Shader = m_pPipeline->load_shaders({"detail2d"});

    m_pPipeline->override_shader(PassType::NORMAL, m_Shader);

    for (int i=0; i<2; ++i){
        m_pQor->pipeline()->shader(i)->use();
        int u = m_pQor->pipeline()->shader(i)->uniform("Ambient");

        if(u >= 0)
            m_pQor->pipeline()->shader(i)->uniform(u,0.1f);
    }

    m_pPipeline->override_shader(PassType::NORMAL, (unsigned)PassType::NONE);
    
    m_pCamera->ortho();
    m_pPipeline->blend(false);
    m_pPipeline->winding(true);
    m_pPipeline->bg_color(Color::black());
    m_pInput->relative_mouse(false);

    for (auto&& player: m_Players)
        player->enter();
}


void Game :: logic(Freq::Time t) {
    Actuation::logic(t);
    
    if (m_pInput->key(SDLK_ESCAPE))
        m_pQor->quit();

    m_pRoot->logic(t);
    m_pOrthoRoot->logic(t);

    m_VisibleNodes = m_pMainLayer->visible_nodes(m_pCamera.get());
}


void Game :: render() const {
    m_pPipeline->override_shader(PassType::NORMAL, m_Shader);

    unsigned idx = 0;
    auto pos = m_pCamera->position();

    for (auto&& layer: m_ParallaxLayers) {
        layer.root->visible(true);
        m_pCamera->position(pos.x * layer.scale, pos.y * layer.scale, 5.0f);
        m_pPipeline->render(layer.root.get(), m_pCamera.get(), nullptr, Pipeline::LIGHTS | (idx == 0 ? 0 : Pipeline::NO_CLEAR));
        layer.root->visible(false);
        ++idx;
    }

    m_pCamera->position(pos);
    m_pPipeline->render(m_pRoot.get(), m_pCamera.get(), nullptr, Pipeline::LIGHTS | (idx == 0 ? 0 : Pipeline::NO_CLEAR));
    m_pPipeline->override_shader(PassType::NORMAL, (unsigned)PassType::NONE);
    
    m_pPipeline->winding(true);
    m_pPipeline->render(m_pOrthoRoot.get(), m_pOrthoCamera.get(), nullptr, Pipeline::NO_CLEAR | Pipeline::NO_DEPTH);
}
