#include "Game.h"
#include "Thing.h"
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
    m_JumpTimer(engine->timer()->timeline()),
    m_ShootTimer(engine->timer()->timeline())
{
}

void Game :: preload()
{
    float sw = m_pQor->window()->size().x;
    float sh = m_pQor->window()->size().y;

    m_pCamera = make_shared<Camera>(m_pQor->resources(), m_pQor->window());
    m_pConsole = make_shared<Console>(
        m_pQor->interpreter(),
        m_pQor->window(),
        m_pQor->input(),
        m_pQor->resources()
    );
    m_pRoot->add(m_pCamera);

    m_pMap = m_pQor->make<TileMap>("2.tmx");
    m_pRoot->add(m_pMap);

    m_pMusic = m_pQor->make<Sound>("thejungle.ogg");
    m_pRoot->add(m_pMusic);
    
    auto scale = 200.0f / std::max<float>(sw * 1.0f,1.0f);
    m_pCamera->rescale(glm::vec3(
        scale, scale,
        1.0f
    ));

    m_pChar = m_pQor->make<Sprite>("guy.json");
    //m_pChar->texture()->ambient(Color::red(1.0f)*0.5f);
    m_pRoot->add(m_pChar);
    m_pChar->set_states({"stand","right"});
    //m_pCamera->position(glm::vec3(-64.0f, -64.0f, 0.0f));
    m_pChar->position(glm::vec3(0.0f, 0.0f, 1.0f));

    m_pCharFocusRight = make_shared<Node>();
    m_pCharFocusRight->position(glm::vec3(32.0f, 0.0f, 0.0f));
    m_pCharFocusLeft = make_shared<Node>();
    m_pCharFocusLeft->position(glm::vec3(-32.0f, 0.0f, 0.0f));
    m_pChar->add(m_pCharFocusRight);
    m_pChar->add(m_pCharFocusLeft);
    m_Players.push_back(m_pChar);
    
    m_pCamera->mode(Tracker::FOLLOW);
    m_pCamera->track(m_pCharFocusRight);
    m_pCamera->focus_time(Freq::Time::ms(200));
    m_pCamera->focal_offset(vec3(
        -m_pQor->window()->center().x * 1.0f,
        -m_pQor->window()->center().y * 1.2f,
        0.0f
    ));
    m_pCamera->listen(true);

    m_pViewLight = make_shared<Light>();
    m_pViewLight->ambient(Color::white() * 0.75f);
    m_pViewLight->diffuse(Color::white());
    m_pViewLight->specular(Color::black());
    m_pViewLight->dist(sw / 1.5f);
    m_pViewLight->position(glm::vec3(
        m_pQor->window()->center().x * 1.0f,
        m_pQor->window()->center().y * 1.2f,
        1.0f
    ));
    m_pCamera->add(m_pViewLight);
    m_pParallaxCamera = make_shared<Camera>(m_pResources, m_pQor->window());
    m_pParallaxCamera->ortho();
    //m_pParallaxCamera->mode(Tracker::STICK);
    m_pParallaxCamera->rescale(glm::vec3(
        scale, scale,
        1.0f
    ));
    m_pRoot->add(m_pParallaxCamera);
    
    vector<vector<shared_ptr<TileLayer>>*> layer_types {
        &m_pMap->layers(),
        &m_pMap->object_layers()
    };
    for(auto&& layers: layer_types)
    for(auto&& layer: *layers)
    {
        layer->set_main_camera(m_pCamera.get());
        
        if(layer->config()->has("parallax"))
        {
            float parallax = boost::lexical_cast<float>(
                layer->config()->at<string>("parallax", "1.0")
            );
            auto pl = ParallaxLayer();
            //pl.camera = make_shared<Camera>(m_pResources, m_pQor->window());
            //pl.camera->ortho();
            //pl.camera->mode(Tracker::PARALLAX);
            //pl.camera->parallax_scale(parallax);
            //pl.camera->track(m_pCamera);
            pl.root = layer;
            pl.scale = parallax;
            //m_pRoot->add(pl.camera);
            m_ParallaxLayers.push_back(pl);
            //auto l = make_shared<Light>();
            //l->dist(1000.0f);
            //l->ambient(Color::white());
            //pl.root->add(l);
            continue;
        }
        
        for(auto&& tile_ptr: layer->all_descendants())
        {
            if(not tile_ptr)
                continue;
            auto tile = tile_ptr->as_node();
            // read object properties and replace
            auto obj = std::dynamic_pointer_cast<MapTile>(tile);
            if(obj)
            {
                auto obj_cfg = obj->config();
                obj->box() = obj->mesh()->box();
                auto name = obj_cfg->at<string>("name","");
                if(name=="spawn")
                {
                    obj->visible(false);
                    obj->mesh()->visible(false);
                    m_Spawns.push_back(obj.get());
                    continue;
                }
                else if(name=="altspawn")
                {
                    obj->visible(false);
                    obj->mesh()->visible(false);
                    m_AltSpawns.push_back(obj.get());
                    continue;
                }
                else if(Thing::get_id(obj_cfg))
                {
                    auto thing = make_shared<Thing>(
                        obj_cfg,
                        obj.get(),
                        this,
                        m_pMap.get(),
                        m_pPartitioner,
                        m_pQor->resources()
                    );
                    obj->add(thing);
                    setup_thing(thing);
                    continue;
                }
                bool depth = layer->depth() || obj_cfg->has("depth");
                if(depth)
                {
                    auto n = make_shared<Node>();
                    n->name("mask");
                    auto mask = obj_cfg->at<shared_ptr<Meta>>("mask", shared_ptr<Meta>());
                    bool hflip = obj->orientation() & (unsigned)MapTile::Orientation::H;
                    bool vflip = obj->orientation() & (unsigned)MapTile::Orientation::V;
                    if(mask && mask->size()==4)
                    {
                        n->box() = Box(
                            vec3(mask->at<double>(0), mask->at<double>(1), -5.0f),
                            vec3(mask->at<double>(2), mask->at<double>(3), 5.0f)
                        );
                    }
                    else
                    {
                        n->box() = Box(
                            vec3(0.0f, 0.0f, -5.0f),
                            vec3(1.0f, 1.0f, 5.0f)
                        );
                    }
                    if(hflip){
                        n->box().min().x = 1.0f - n->box().min().x;
                        n->box().max().x = 1.0f - n->box().max().x;
                    }
                    if(vflip){
                        n->box().min().y = 1.0f - n->box().min().y;
                        n->box().max().y = 1.0f - n->box().max().y;
                    }
                    obj->mesh()->add(n);
                    if(obj_cfg->has("fatal"))
                        m_pPartitioner->register_object(n, FATAL);
                    //else if(name == "star")
                    //{
                    //    auto l = make_shared<Light>();
                    //    l->ambient(Color::yellow());
                    //    l->diffuse(Color::black());
                    //    l->specular(Color::black());
                    //    l->dist(32.0f);
                    //    obj->add(l);
                    //    l->move(glm::vec3(8.0f, 8.0f, 0.0f));
                    //    m_pPartitioner->register_object(n, THING);
                    //}
                    //else if(name == "heart")
                    //{
                    //    auto l = make_shared<Light>();
                    //    l->ambient(Color::red());
                    //    l->diffuse(Color::black());
                    //    l->specular(Color::black());
                    //    l->dist(32.0f);
                    //    obj->stick(l);
                    //    l->move(glm::vec3(8.0f, 8.0f, 0.0f));
                    //    m_pPartitioner->register_object(n, THING);
                    //}
                    //else if(name == "battery")
                    //{
                    //    auto l = make_shared<Light>();
                    //    l->ambient(Color::green());
                    //    l->diffuse(Color::black());
                    //    l->specular(Color::black());
                    //    l->dist(32.0f);
                    //    obj->stick(l);
                    //    l->move(glm::vec3(8.0f, 8.0f, 0.0f));
                    //    m_pPartitioner->register_object(n, THING);
                    //}

                    //else if(name != "")
                    //{
                    //    m_pPartitioner->register_object(n, THING);
                    //}
                    //else
                    else if(obj_cfg->has("ledge"))
                        m_pPartitioner->register_object(n, LEDGE);
                    else{
                        m_pPartitioner->register_object(n, STATIC);
                        obj_cfg->set<string>("static", "");
                    }
                } else {
                    m_pPartitioner->register_object(obj->mesh(), GROUND);
                    obj->mesh()->config()->set<string>("static", "");
                }
            }
        }
    }
    
    for(auto&& player: m_Players)
        setup_player(player);
    reset();

    m_pPartitioner->on_collision(
        CHARACTER, STATIC,
        std::bind(&Game::cb_to_tile, this, _::_1, _::_2)
    );
    m_pPartitioner->on_collision(
        CHARACTER, LEDGE,
        std::bind(&Game::cb_to_ledge, this, _::_1, _::_2)
    );
    m_pPartitioner->on_collision(
        CHARACTER, FATAL,
        std::bind(&Game::cb_to_fatal, this, _::_1, _::_2)
    );
    m_pPartitioner->on_collision(
        THING, STATIC,
        std::bind(&Thing::cb_to_static, _::_1, _::_2)
    );
    m_pPartitioner->on_collision(
        THING, FATAL,
        std::bind(&Thing::cb_to_static, _::_1, _::_2)
    );
    m_pPartitioner->on_collision(
        BULLET, STATIC,
        std::bind(&Game::cb_bullet_to_static, this, _::_1, _::_2)
    );
    m_pPartitioner->on_collision(
        THING, BULLET,
        std::bind(&Thing::cb_to_bullet, _::_1, _::_2)
    );
    m_pPartitioner->on_collision(
        CHARACTER, THING,
        std::bind(&Thing::cb_to_player, _::_1, _::_2)
        //std::bind(&Game::cb_to_thing, this, _::_1, _::_2)
    );

    m_JumpTimer.set(Freq::Time::ms(0));
    m_ShootTimer.set(Freq::Time::ms(0));
}

void Game :: reset()
{
    //m_pChar->position(glm::vec3(0.0f, 0.0f, 0.0f));
    try{
        //m_Spawns.at(0)->stick(m_pChar);
        m_pChar->position(m_Spawns.at(0)->position());
    }catch(...){
        WARNING("Map has no spawn points");
    }
    m_pChar->move(glm::vec3(0.0f, 0.0f, 1.0f));

    on_reset();
}

Game :: ~Game()
{
    m_pPipeline->partitioner()->clear();
}

void Game :: setup_thing(std::shared_ptr<Thing> thing)
{
    thing->init_thing();
    m_Things.push_back(thing);

    for(auto&& player: m_Players)
        setup_player_to_thing(player,thing);
}

void Game :: setup_player(std::shared_ptr<Sprite> player)
{ 
    // create masks
    auto n = make_shared<Node>();
    n->name("mask");
    n->box() = Box(
        vec3(-4.0f, -12.0f, -5.0f),
        vec3(4.0f,2.0f, 5.0f)
    );
    player->add(n);
    m_pPartitioner->register_object(n, CHARACTER);

    // create masks
    n = make_shared<Node>();
    n->name("feetmask");
    n->box() = Box(
        vec3(-4.0f, 0.0f, -5.0f),
        vec3(4.0f,4.0f, 5.0f)
    );
    player->add(n);
    m_pPartitioner->register_object(n, CHARACTER_FEET);

    n = make_shared<Node>();
    n->name("sidemask");
    n->box() = Box(
        vec3(-10.0f, -10.0f, -5.0f),
        vec3(10.0f,-2.0f, 5.0f)
    );
    player->add(n);
    m_pPartitioner->register_object(n, CHARACTER_SIDES);

    setup_player_to_map(player);
    for(auto&& thing: m_Things)
        setup_player_to_thing(player,thing);
}

void Game :: setup_player_to_map(std::shared_ptr<Sprite> player)
{
    if(not m_pMap)
        return;
}

void Game :: setup_player_to_thing(std::shared_ptr<Sprite> player, std::shared_ptr<Thing> thing)
{
}

std::vector<Node*> Game :: get_static_collisions(Node* a)
{
    auto static_cols = m_pPartitioner->get_collisions_for(a, STATIC);
    auto ledge_cols = m_pPartitioner->get_collisions_for(a, LEDGE);
    
    auto m = a->parent();
    vec3 old_pos;
    if(m->num_snapshots())
        old_pos = Matrix::translation(kit::safe_ptr(m->snapshot(0))->world_transform);
    else
        old_pos = m->position(Space::WORLD);
    
    std::copy_if(ENTIRE(ledge_cols), std::back_inserter(static_cols), [old_pos,a](Node* n){
        return old_pos.y <= n->position(Space::WORLD).y;
    });
    return static_cols;
}

void Game :: cb_to_static(Node* a, Node* b, Node* m)
{
    if(not m) m = a;
    
    auto p = m->position(Space::PARENT);
    auto v = m->velocity();
    auto col = [this, a, b]() -> bool {
        return (not get_static_collisions(a).empty()) ||
            a->world_box().collision(b->world_box());
    };

    //Box overlap = a->world_box().intersect(b->world_box());
    auto old_pos = Matrix::translation(kit::safe_ptr(m->snapshot(0))->world_transform);
    
    //vec3 overlap_sz = overlap.size() + vec3(1.0f);

    //if(not floatcmp(m->velocity().y, 0.0f))
    //{
        auto np = vec3(p.x, old_pos.y, p.z);
        m->position(np);
        if(not col()){
            m->velocity(glm::vec3(v.x,0.0f,v.z));
            return;
        }
    //}
        
    //if(not floatcmp(m->velocity().x, 0.0f))
    //{
        np = vec3(old_pos.x, p.y, p.z);
        m->position(np);
        if(not col())
            return;
    //}
    
    m->position(vec3(old_pos.x, old_pos.y, p.z));
    m->velocity(glm::vec3(0.0f));
}

void Game :: cb_to_ledge(Node* a, Node* b)
{
    auto m = a->parent();
    auto old_pos = Matrix::translation(kit::safe_ptr(m->snapshot(0))->world_transform);
    if(m->velocity().y >= K_EPSILON)
        if(old_pos.y <= b->position(Space::WORLD).y){
            //LOGf("static %s %s", old_pos.y % b->position(Space::WORLD).y);
            cb_to_static(a, b, a->parent());
        }
}

void Game :: cb_to_tile(Node* a, Node* b)
{
    cb_to_static(a, b, a->parent());
}

void Game :: cb_to_fatal(Node* a, Node* b)
{
    Sound::play(m_pCamera.get(), "die.wav", m_pResources);
    reset();
    m_pChar->velocity(glm::vec3(0.0f));
    //cb_to_static(a, b, a->parent());
}

//void Game :: cb_to_thing(Node* a, Node* b)
//{
//    auto tile = (MapTile*)(b->parent()->parent()); // mask -> mesh -> tile
//    if(tile->visible()){
//        Sound::play(m_pCamera.get(), "pickup2.wav", m_pResources);
//        tile->visible(false);
//        tile->mesh()->visible(false);
//        auto lights = tile->hook_type<Light>();
//        for(auto&& light: lights){
//            LOG("light")
//            light->visible(false);
//        }
//    }
//}

void Game :: cb_bullet_to_static(Node* a, Node* b)
{
    Sound::play(a, "hit.wav", m_pResources);
    //a->on_tick.connect([a](Freq::Time){
        a->safe_detach();
    //});
}

void Game :: enter()
{
    m_pMusic->play();
    
    m_Shader = m_pPipeline->load_shaders({"detail2"});
    m_pPipeline->override_shader(PassType::NORMAL, m_Shader);
    for(int i=0; i<2; ++i){
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

    for(auto&& player: m_Players)
        player->acceleration(glm::vec3(0.0f, 500.0f, 0.0f));
}

void Game :: logic(Freq::Time t)
{
    Actuation::logic(t);
    
    if(m_pInput->key(SDLK_ESCAPE))
        m_pQor->quit();

    for(auto&& player: m_Players)
    {
        auto feet_colliders = get_static_collisions(
            player->hook("feetmask").at(0)
        );
        auto wall_colliders = m_pPartitioner->get_collisions_for(
            player->hook("sidemask").at(0), STATIC
        );
        if(not feet_colliders.empty() && wall_colliders.empty()){
            auto v = player->velocity();
            player->velocity(0.0f, v.y, v.z);
        }
        
        auto vel = player->velocity();
        bool in_air = feet_colliders.empty();
        bool walljump = feet_colliders.empty() && not wall_colliders.empty();
        if(walljump)
            player->set_state("walljump");
        else if(in_air)
            player->set_state("jump");
        
        glm::vec3 move(0.0f);
        if(player->velocity().x > -K_EPSILON && player->velocity().x < K_EPSILON){
            if(m_pController->button("left")){
                m_pCamera->track(m_pCharFocusLeft);
                move += glm::vec3(-1.0f, 0.0f, 0.0f);
            }
            if(m_pController->button("right")){
                m_pCamera->track(m_pCharFocusRight);
                move += glm::vec3(1.0f, 0.0f, 0.0f);
            }
        }

        if(m_pController->button("shoot") && m_ShootTimer.elapsed()){
            auto shot = make_shared<Mesh>(
                make_shared<MeshGeometry>(Prefab::quad(glm::vec2(8.0f, 2.0f))),
                vector<shared_ptr<IMeshModifier>>{
                    make_shared<Wrap>(Prefab::quad_wrap())
                },
                make_shared<MeshMaterial>("laser.png", m_pResources)
            );

            //m_pRoot->stick(shot);
            //shot->position(player->position());
            m_pRoot->add(shot);
            auto l = make_shared<Light>();
            l->ambient(Color::red());
            l->diffuse(Color::red()); // black
            l->specular(Color::white());
            l->dist(32.0f);
            l->move(glm::vec3(glm::vec3(4.0f, 1.0f, 0.0f)));
            shot->add(l);
            shot->position(glm::vec3(
                player->position().x +
                -player->origin().x*player->size().x +
                    player->mesh()->world_box().size().x / 2.0f,
                    //((player->check_state("left")?-1.0f:1.0f) * 4.0f),
                player->position().y +
                    -player->origin().y*player->size().y +
                    player->mesh()->world_box().size().y / 2.0f + 
                    -2.0f,
                player->position().z
            ));
            //player->stick(shot);
            shot->rotate(((std::rand() % 10)-5) / 360.0f, glm::vec3(0.0f, 0.0f, 1.0f));
            shot->velocity(shot->orient_to_world(glm::vec3(
                (player->check_state("left")?-1.0f:1.0f) * 256.0f,
                0.0f, 0.0f
            )));
            auto timer = make_shared<Freq::Alarm>(m_pQor->timer()->timeline());
            timer->set(Freq::Time::seconds(0.5f));
            auto shotptr = shot.get();
            shot->on_tick.connect([timer,shotptr](Freq::Time t){
                if(timer->elapsed())
                    shotptr->detach();
            });
            
            m_pPartitioner->register_object(shot, BULLET);
            
            Sound::play(m_pCamera.get(), "shoot.wav", m_pResources);

            m_ShootTimer.set(Freq::Time::ms(
                m_pChar->config()->at<int>("power",0)>1?50:100
            ));
            
            // increase box Z width
            auto shotbox = shot->box();
            shot->set_box(Box(
                vec3(shotbox.min().x, shotbox.min().y, -5.0),
                vec3(shotbox.max().x, shotbox.max().y, 5.0)
            ));
        }
            
        bool block_jump = false;
        if(m_pController->button("up") || m_pController->button("jump")){
            
            if(walljump || not in_air || not m_JumpTimer.elapsed())
            {
                float x = 0.0f;
                if(walljump){
                    auto last_dir = m_LastWallJumpDir;
                    auto tile_loc = wall_colliders.at(0)->world_box().center().x;
                    if(tile_loc < player->position(Space::WORLD).x + 4.0f){
                        m_LastWallJumpDir = -1;
                    //    LOG("left");
                    //    player->set_state("left");
                    //    x += (move.x < -K_EPSILON) ? 100.0f : 0.0f;
                    //}
                    }else{
                        m_LastWallJumpDir = 1;
                    //    LOG("right");
                    //    player->set_state("right");
                    //    x += (move.x > K_EPSILON) ? -100.0f : 0.0f;
                    }

                    // prevent "climbing" the wall by checking to make sure last wall jump was a diff direction (or floor)
                    // 0 means floors, -1 and 1 are directions
                    if(m_LastWallJumpDir != 0 && last_dir != 0 && m_LastWallJumpDir == last_dir)
                        block_jump = true;
                    //move = vec3(0.0f);
                }
            
                // jumping from floor or from a different wall
                if(not block_jump){
                    if(not in_air || walljump || not m_JumpTimer.elapsed()){
                        player->velocity(glm::vec3(x, -125.0f, 0.0f));
                        if(not in_air || walljump){
                            auto sounds = m_pCamera->hook_type<Sound>();
                            if(sounds.empty())
                                Sound::play(m_pCamera.get(), "jump.wav", m_pResources);
                            m_JumpTimer.set(Freq::Time::ms(200));
                        }
                    }
                }else{
                    // jumping on same wall? only allow this if jump timer is still running
                    if(not m_JumpTimer.elapsed())
                        player->velocity(glm::vec3(x, -125.0f, 0.0f));
                }
            }
        }else{
            m_JumpTimer.set(Freq::Time::ms(0));
        }

        if(not in_air && m_WasInAir)
            Sound::play(m_pCamera.get(), "touch.wav", m_pResources);
        m_WasInAir = in_air;
        //if(m_pController->button("down"))
        //    move += glm::vec3(0.0f, 1.0f, 0.0f);
        
        if(not in_air)
            m_LastWallJumpDir = 0;
        if(glm::length(move) > K_EPSILON)
        {
            if(not in_air)
                player->set_state("walk");
            move = glm::normalize(move);
            if(move.x < -K_EPSILON)
                player->set_state("left");
            else if(move.x > K_EPSILON)
                player->set_state("right");
            move *= 100.0f * t.s();
            player->clear_snapshots();
            player->snapshot();
            player->move(move);
        }
        else
        {
            if(not in_air)
                player->set_state("stand");
            player->clear_snapshots();
            player->snapshot();
            //player->move(glm::vec3(0.0f));
        }
    }
    //for(auto&& layer: m_ParallaxLayers){
    //    layer.camera->logic(t); // tile partitioner bypasses logic
    //}
    m_pRoot->logic(t);
}

void Game :: render() const
{
    m_pPipeline->override_shader(PassType::NORMAL, m_Shader);
    unsigned idx = 0;
    //auto pos = m_pCamera->position();
    //for(auto&& layer: m_ParallaxLayers){
        //layer.root->visible(true);
    //    //m_pParallaxCamera->position(
    //    //    m_pCamera->position().x * layer.scale,
    //    //    m_pCamera->position().y * layer.scale,
    //    //    0.0f
    //    //);
        //m_pCamera->position(pos.x * layer.scale, pos.y * layer.scale, 0.0f);
        //m_pPipeline->render(layer.root.get(), m_pCamera.get(), nullptr, Pipeline::LIGHTS | (idx==0?0:Pipeline::NO_CLEAR));
        //layer.root->visible(false);
        //++idx;
        //layer.root->parent()->position(
        //    -m_pCamera->position().x,
        //    -m_pCamera->position().y,
        //    layer.root->parent()->position().z
        //);
        //++idx;
    //}
    //m_pCamera->position(pos);
    m_pPipeline->render(m_pRoot.get(), m_pCamera.get(), nullptr, Pipeline::LIGHTS | (idx==0?0:Pipeline::NO_CLEAR));
    m_pPipeline->override_shader(PassType::NORMAL, (unsigned)PassType::NONE);
}

