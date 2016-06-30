#include "Monster.h"
#include "Game.h"
#include "Qor/TileMap.h" 
#include "Qor/Sprite.h"
#include "Player.h"
#include "kit/math/vectorops.h"

using namespace std;
using namespace glm;

// Monster Constructor
Monster :: Monster(
    const shared_ptr<Meta>& config,                // Parameters
    MapTile* placeholder,
    Game* game,
    TileMap* map,
    BasicPartitioner* partitioner,
    Freq::Timeline* timeline,
    Cache<Resource, string>* resources
):                                                  /* Initializers */
    Node(config),                                   // Calls the Node Constructor
    m_pPlaceholder(placeholder),                    // Set Monster Placeholder
    m_pPartitioner(partitioner),                    // Set Monster Partitioner
    m_pGame(game),                                  // Set Monster Game
    m_pMap(map),                                    // Set Monster Map
    m_pResources(resources),                        // Set Monster Resources
    m_MonsterID(get_type(config)),                  // Set Monster Type (int)
    m_Identity(config->at<string>("name", "")),     // Set Monster Type (String)
    m_StunTimer(timeline),                          // Set Monster Stun Time (Alarm)
    m_pTimeline(timeline),                          // Set Timeline
    m_ShootTimer(&m_LazyTimeline)
{}


// Static Member Variable Definitions
const vector<string> Monster :: s_TypeNames({
    // Match this to Monster Type enum
    "",
    "duck",
    "mouse",
    "robot",
    "snail",
    "wizard",
});


// Find the Monster Type enum value
unsigned Monster :: get_type(const shared_ptr<Meta>& config) {
    string name = config->at<string>("name", "");
    
    if (name.empty())
        return 0;

    // Determines if the name is in the Monster Typenames vector
    auto itr = std::find(ENTIRE(s_TypeNames), name);
    if (itr == s_TypeNames.end())
        return 0;
    
    return std::distance(s_TypeNames.begin(), itr);
}


void Monster :: logic_self(Freq::Time t) {
    clear_snapshots();
    snapshot();
    
    // why is this here instead of a signal/slot in stun?
    if (m_StunTimer.elapsed()) {
        m_pSprite->set_state("unhit");
        m_StunTimer.reset();
    }

    // if a player is within range of monster, set active
    auto players = m_pGame->players();
    float min_dist = 10000.0f;

    Player* closest_player = nullptr;
    for(auto&& player: players)
    {
        auto dist = glm::length(
            player->position(Space::WORLD) - position(Space::WORLD)
        );
        if(dist < min_dist){
            min_dist = dist;
            closest_player = player.get();
        }
    }

    bool old_active = m_bActive;
    m_bActive = (min_dist < 100.0f);
    if(old_active != m_bActive)
    {
        if(m_bActive)
            activate(closest_player);
        else
            deactivate(closest_player);
    }

    if(m_bActive) {
        m_LazyTimeline.logic(t);
        
        //LOG("active");
        if (m_MonsterID == Monster::WIZARD) {
            if(m_ShootTimer.elapsed() || not m_ShootTimer.started()) {
                shoot(DEFAULT_BULLET_SPEED, vec3(kit::sign(velocity().x)*10.0f, 0.0f, 0.0f), 10);
                m_ShootTimer.set(Freq::Time::seconds(2.0f));
            }
        }
        if (m_MonsterID == Monster::MOUSE) {
            if(m_ShootTimer.elapsed() || not m_ShootTimer.started()) {
                shoot(DEFAULT_BULLET_SPEED, vec3(kit::sign(velocity().x)*10.0f, 0.0f, 0.0f));
                m_ShootTimer.set(Freq::Time::seconds(0.5f));
            }
        }

    }

    auto layer = (TileLayer*)parent();
    auto vel = velocity();
    if(vel.x < -K_EPSILON && layer->tile(
        (position().x / layer->size().x - 1), // -
        (position().y / layer->size().y + 1)
    )) {
        velocity(-vel.x, vel.y, vel.z);
    }
    else if(vel.x > K_EPSILON && layer->tile(
        position().x / layer->size().x + 1, // +
        position().y / layer->size().y + 1
    )) {
        velocity(-vel.x, vel.y, vel.z);
    }


    // Why not in damage?
    if (not is_alive())
        detach();
}


void Monster :: initialize() {
    assert(m_pPartitioner);

    m_Box = m_pPlaceholder->box();

    //m_pPartitioner->register_object(shared_from_this(), Game::MONSTER);

    TRY(m_pConfig->merge(make_shared<Meta>(
        m_pResources->transform(m_Identity + ".json")
    )));
    
    auto mask = m_pConfig->meta("mask");
    m_Box = Box(
        vec3(mask->at<double>(0), mask->at<double>(1), -0.5f),
        vec3(mask->at<double>(2), mask->at<double>(3), 0.5f)
    );
    
    //m_Box = Box(
    //    vec3(
    //        m_pPlaceholder->world_box().min().x * mask->at<double>(0),
    //        m_pPlaceholder->box().min().y * mask->at<double>(1),
    //        -0.5f
    //    ),
    //    vec3(
    //        m_pPlaceholder->box().max().x * mask->at<double>(2),
    //        m_pPlaceholder->box().max().y * mask->at<double>(3),
    //        0.5f
    //    )
    //);
    //LOG(m_Box.string());
    
    // make sensor boxes for detecting ground to the left and right of the monster
    //m_pLeft = make_shared<Mesh>();
    //m_pRight = make_shared<Mesh>();
    
    //auto lbox = Box();
    //auto rbox = Box();
    //lbox = Box(
    //    vec3(-4.0f, 0.0f, -0.5f),
    //    vec3(0.0f, 8.0f, 0.5f)
    //);
    //rbox = Box(
    //    vec3(0.0f, 0.0f, -0.5f),
    //    vec3(4.0f, 8.0f, 0.5f)
    //);
    //m_pLeft->set_box(lbox);
    //m_pRight->set_box(rbox);
    
    //add(m_pLeft);
    //add(Mesh::line(vec3(-8.0f, 0.0f, 1.0f), vec3(8.0f, 0.0f, 1.0f),
    //    m_pResources->cache_as<Texture>("white.png")
    //));
    //add(m_pRight);

    //LOG(m_pLeft->world_box().string());
    
    //m_pLeft->config()->set<Monster*>("monster", this);
    //m_pRight->config()->set<Monster*>("monster", this);
    //m_pPartitioner->register_object(m_pLeft, Game::SENSOR);
    //m_pPartitioner->register_object(m_pRight, Game::SENSOR);

    m_HP = m_pConfig->at<int>("hp", 5);
    m_MaxHP = m_pConfig->at<int>("hp", 5);
    m_StartSpeed = m_pConfig->at<double>("speed", 10.0);
    m_Speed = m_StartSpeed;

    m_pSprite = make_shared<Sprite>(
        m_pResources->transform(m_Identity + ".json"),
        m_pResources
    );

    add(m_pSprite);

    m_pSprite->set_states({"unhit", "left"});
    if (m_pPlaceholder->tile_layer()->depth() || m_pConfig->has("depth"))
        m_pSprite->mesh()->set_geometry(m_pMap->tilted_tile_geometry());

    collapse(); // detach from placeholder
    rescale(vec3(1.0f));
    position(m_pPlaceholder->position(Space::WORLD)); 

    // inherit placeholder pos
    // adding a sprite will spawn its center on 0,0...
    // so we offset
    move(vec3(
        m_pSprite->origin().x * m_pSprite->size().x,
        m_pSprite->origin().y * m_pSprite->size().y,
        0.0f
    ));

    //m_pPlaceholder->detach(); // don't want to invalidate iterator
    m_pPlaceholder->mesh()->visible(false); // remove placeholder
    m_pSprite->mesh()->config()->set<string>("id", m_Identity);
    m_pSprite->mesh()->config()->set<Monster*>("monster", this);
    m_pSprite->mesh()->set_box(m_Box);
    m_pPartitioner->register_object(m_pSprite->mesh(), Game::MONSTER);

    velocity(vec3(-m_Speed, 0.0f, 0.0f));
}


void Monster :: activate(Player* closest_player) {
    // Triggered bro

    // Trigger Monster activated behavior

    // face towards player
    auto vel = velocity();
    velocity(
        kit::sign((
            closest_player->position(Space::WORLD) -
            position(Space::WORLD)
        ).x) *
        abs(vel.x),
        vel.y,
        vel.z
    );
    if(velocity().x < -K_EPSILON)
        sprite()->set_state("left");
    else if(velocity().x > K_EPSILON)
        sprite()->set_state("right");
}


void Monster :: deactivate(Player* closest_player) {
    // Trigger Monster deactivated behavior

}


void Monster :: damage(int dmg) {
    if (m_HP > 0 and dmg > 0) {
        m_HP = std::max(m_HP - dmg, 0);

        if (m_HP == 0) {
            m_Dying = true;
            velocity(vec3(0.0f));
        }
    }
}


void Monster :: shoot(float bullet_speed, glm::vec3 offset, int life) {

    if (m_MonsterID == Monster::WIZARD) {

        // Load fire sprite
        auto fire = make_shared<Sprite>(m_pResources->transform("fire.json"), m_pResources);
        fire->set_state("animated");

        auto firebox = fire->mesh()->box();
        fire->mesh()->set_box(Box(
            vec3(firebox.min().x, firebox.min().y, -5.0),
            vec3(firebox.max().x, firebox.max().y, 5.0)
        ));
        
        add(fire);
        fire->collapse();

        m_pPartitioner->register_object(fire->mesh(), Game::FATAL);
        
        offset.x += kit::sign(velocity().x) * float(fire->size().x);
        fire->position(fire->position() + offset);
        
        auto spawn_timer = make_shared<Freq::Alarm>(m_pTimeline);
        spawn_timer->set(Freq::Time::seconds(0.05f));
        auto death_timer = make_shared<Freq::Alarm>(m_pTimeline);
        death_timer->set(Freq::Time::seconds(0.5f));
        
        Sound::play(m_pSprite.get(), "fire.wav", m_pResources);

        auto _this = this;
        auto fireptr = fire.get();
        auto origin = m_pSprite.get();
        //fireptr->on_tick.connect_extended([
        auto part = m_pPartitioner;
        fireptr->on_tick.connect([
            _this, spawn_timer, death_timer, bullet_speed, offset, fireptr,
            origin, life, part
        //](boost::signals2::connection con, Freq::Time t){
        ](Freq::Time t){
            //auto fire = firew.lock();
            //if(not fire){
            //    con.disconnect();
            //    return;
            //}
        
            if(spawn_timer->elapsed()){
                if(life > 0)
                    _this->shoot(bullet_speed, offset, life-1);
                spawn_timer->reset();
            }
            if(death_timer->elapsed()){
                fireptr->safe_detach();
                //part->after([fireptr]{
                //});
                //con.disconnect();
            }
        });
        //fire->on_tick.connect([fireptr, death_timer](Freq::Time){
        //    if(death_timer->elapsed()){
        //        fireptr->safe_detach();
        //    }
        //});
    }

    else {
        // Create a bullet mesh
        auto shot = make_shared<Mesh>(
            make_shared<MeshGeometry>(Prefab::quad(glm::vec2(8.0f, 2.0f))),
            vector<shared_ptr<IMeshModifier>>{make_shared<Wrap>(Prefab::quad_wrap(vec2(0.0f, 1.0f), vec2(1.0f, 0.0f)))},
            make_shared<MeshMaterial>("laser.png", m_pResources)
        );

        m_pPartitioner->register_object(shot, Game::BULLET);
        shot->config()->set<Monster*>("monster",this);

        // Creates a box around the bullet (With increased z width)
        auto shotbox = shot->box();
        shot->set_box(Box(
            vec3(shotbox.min().x, shotbox.min().y, -5.0),
            vec3(shotbox.max().x, shotbox.max().y, 5.0)
        ));

        // Add the bullet to the parent
        //auto par = m_pSprite->parent();
        //par->add(shot);
        stick(shot);
        shot->move(vec3(0.0f, -m_pSprite->mesh()->world_box().size().y / 2.0f, 0.0f));

        // Add a random angle to the bullet
        shot->rotate(((rand() % 10) - 5) / 360.0f, vec3(0.0f, 0.0f, 1.0f));
        shot->velocity(shot->orient_to_world(
            vec3((m_pSprite->check_state("left") ? -1.0f : 1.0f) * bullet_speed, 0.0f, 0.0f)
        ));

        // Sets timer for bullets before disappearing
        auto timer = make_shared<Freq::Alarm>(m_pTimeline);
        timer->set(Freq::Time::seconds(0.5f));

        Sound::play(m_pSprite.get(), "shoot.wav", m_pResources);

        // Connects shot to a game tick signal
        auto shotptr = shot.get();
        shot->on_tick.connect([timer, shotptr](Freq::Time t){
            if (timer->elapsed())
                shotptr->detach();
        });
    }
}


void Monster :: stun(int stun_time=DEFAULT_STUN_TIME) {
    m_pSprite->set_state("hit");
    m_StunTimer.set(Freq::Time::ms(stun_time));
}


void Monster :: gib() {
    auto gib = make_shared<Sprite>(m_pResources->transform("blood.json"), m_pResources);
    gib->set_state("animated");
    
    // Randomizes direction gib moves
    auto dir = Angle::degrees(1.0f * (rand() % 360)).vector();
    stick(gib);

    // Sets gib size and movement
    gib->move(vec3(rand() % 16 - 8.0f, rand() % 32 - 16.0f, 2.0f));
    gib->velocity(vec3(dir, 0.0f) * 100.0f);
    gib->acceleration(vec3(0.0f, 500.0f, 0.0f));
    gib->scale(rand() % 100 / 100.0f * 0.5f);

    // Creates random gib lifetime
    auto lifetime = make_shared<float>(0.5f * (rand() % 4));
    auto gibptr = gib.get();

    // Connexts gib to game tick signal
    gib->on_tick.connect([gibptr, lifetime](Freq::Time t){
        *lifetime -= t.s();

        if(*lifetime < 0.0f)
            gibptr->detach();
    });
}


void Monster :: sound(const std::string& fn) {
    Sound::play(this, fn, m_pResources);
}


// Callbacks
void Monster :: cb_to_bullet(Node* monster_node, Node* bullet) {
    auto monster = monster_node->config()->at<Monster*>("monster", nullptr);

    if (not monster)
        return;

    // bullet owner is a monster, ignore
    if(bullet->config()->has("monster"))
        return;

    if (monster->is_alive() and not bullet->detaching()) {
        monster->sound("damage.wav");

        auto hp_before = monster->m_HP;
        monster->damage(bullet->config()->at("damage", 1));
        auto hp_after = monster->m_HP;

        if (hp_before > hp_after) {

            // Generate blood splatter when hit
            auto gibs = monster->m_Dying ? 5 : 20;
            for (int i = 0; i < gibs; ++i)
                monster->gib();

            // Knockback and stun
            auto vel = monster->velocity();
            monster->move(vec3(-kit::sign(vel.x) * 5.0f, 0.0f, 0.0f));
            monster->stun();
            
            // Change direction based on bullet velocity
            if (bullet->velocity().x > K_EPSILON) {
                monster->velocity(-abs(monster->velocity()));
                monster->sprite()->set_state("left");
            } else if (bullet->velocity().x < -K_EPSILON) {
                monster->velocity(abs(monster->velocity()));
                monster->sprite()->set_state("right");
            }
            
            // Schedule detachment and activate monster
            bullet->safe_detach();
            //monster->activate();
        }

        // Change color of monster based on health
        monster->m_pSprite->material()->ambient(kit::mix(
            Color::red(), Color::white(), float(monster->hp()) / float(monster->max_hp())
        ));
    }
}


void Monster :: cb_to_static(Node* monster_node, Node* static_node) {
    auto monster = monster_node->config()->at<Monster*>("monster",nullptr);

    if (not monster)
        return;

    if (monster->num_snapshots()) {
        if (static_node->world_box().center().x > monster->world_box().center().x) {
            monster->velocity(-abs(monster->velocity()));
            monster->sprite()->set_state("left");
        } else if (static_node->world_box().center().x < monster->world_box().center().x) {
            monster->velocity(abs(monster->velocity()));
            monster->sprite()->set_state("right");
        }
    }
}


void Monster :: cb_to_player(Node* player_node, Node* monster_node) {
    auto monster = monster_node->config()->at<Monster*>("monster", nullptr);

    if (not monster)
        return;

    if (monster->is_alive())
        monster->m_pGame->reset();
}


//void Monster :: cb_sensor_to_no_static(Node* sensor_node, Node* static_node) {
//    auto monster = sensor_node->config()->at<Monster*>("monster", nullptr);
//    LOG("???")

//    if (not monster)
//        return;
    
//    LOG("sensor to no static");
//    monster->velocity(
//        -monster->velocity().x,
//        monster->velocity().y,
//        monster->velocity().z
//    );
//}

