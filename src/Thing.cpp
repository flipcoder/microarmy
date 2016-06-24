#include "Thing.h"
#include "Game.h"
#include "Qor/Sprite.h"

using namespace std;
using namespace glm;


const std::vector<std::string> Thing :: s_TypeNames({
    "",
    
    // monsters
    "mouse",
    "snail",
    "wizard",
    "robot",
    "duck",
    
    // items
    "battery",
    "heart",
    "star",
    "key",
    
    // objects
    "spring",
    "door",

    "light",
});


Thing :: Thing( // Parameters
    const std::shared_ptr<Meta>& config,
    MapTile* placeholder,
    Game* game,
    TileMap* map,
    BasicPartitioner* partitioner,
    Freq::Timeline* timeline,
    Cache<Resource, std::string>* resources
): // Initialize Variables
    Node(config),
    m_pPlaceholder(placeholder),
    m_pPartitioner(partitioner),
    m_pGame(game),
    m_pMap(map),
    m_pResources(resources),
    m_Identity(config->at<string>("name", "")),
    m_ThingID(get_id(config)),
    m_StunTimer(timeline),
    m_pTimeline(timeline)
{}


std::shared_ptr<Thing> Thing :: find_thing(Node* n) {
    shared_ptr<Thing> thing;
    thing = dynamic_pointer_cast<Thing>(n->as_node());
    
    if (not thing) {
        thing = dynamic_pointer_cast<Thing>(n->parent()->as_node());
        if (not thing)
            thing = dynamic_pointer_cast<Thing>(n->parent()->parent()->as_node());
    }
    return thing;
}


void Thing :: init_thing() {
    assert(m_pPartitioner);

    m_Box = m_pPlaceholder->box();

    m_pPartitioner->register_object(shared_from_this(), Game::THING);
    
    const float item_dist = 200.0f;
    const float glow = 1.0f;

    if (is_monster()) {
        TRY(m_pConfig->merge(make_shared<Meta>(
            m_pResources->transform(m_Identity + ".json")
        )));
        
        auto mask = m_pConfig->meta("mask");
        m_Box = Box(
            vec3(mask->at<double>(0), mask->at<double>(1), -0.5f),
            vec3(mask->at<double>(2), mask->at<double>(3), 0.5f)
        );
        
        m_pLeft = make_shared<Mesh>();
        auto lbox = m_Box;
        lbox.min().x -= m_Box.size().x;
        lbox.max().x -= m_Box.size().x;
        lbox.min().y += m_Box.size().y;
        lbox.max().y += m_Box.size().y;
        m_pLeft->set_box(lbox);
        add(m_pLeft);
        
        m_pRight = make_shared<Mesh>();
        auto rbox = m_Box;
        rbox.min().x += m_Box.size().x;
        rbox.max().x += m_Box.size().x;
        rbox.min().y += m_Box.size().y;
        rbox.max().y += m_Box.size().y;
        m_pRight->set_box(rbox);
        add(m_pRight);

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
        rescale(glm::vec3(1.0f));
        position(m_pPlaceholder->position(Space::WORLD)); // inherit placeholder pos
        // adding a sprite will spawn its center on 0,0...
        // so we offset
        move(glm::vec3(
            m_pSprite->origin().x * m_pSprite->size().x,
            m_pSprite->origin().y * m_pSprite->size().y,
            0.0f
        ));

        //m_pPlaceholder->detach(); // don't want to invalidate iterator
        m_pPlaceholder->mesh()->visible(false); // remove placeholder
        m_pSprite->mesh()->config()->set<string>("id", m_Identity);
        m_pSprite->mesh()->config()->set<Thing*>("thing", this);
        m_pSprite->mesh()->set_box(m_Box);
        m_pPartitioner->register_object(m_pSprite->mesh(), Game::THING);

        velocity(vec3(-m_Speed, 0.0f, 0.0f));
        
    } else if (m_ThingID == Thing::STAR) {
        auto l = make_shared<Light>();
        string type = config()->at<string>("type");

        l->ambient(Color::white() * glow);
        l->diffuse(Color::white() * glow);

        l->specular(Color::white() * glow);
        l->dist(item_dist);
        l->move(glm::vec3(glm::vec3(0.5f, 0.5f, 0.0f)));
        add(l);
        collapse();

    }
    else if (m_ThingID == Thing::BATTERY) {
        auto l = make_shared<Light>();
        l->ambient(Color::green() * glow);
        l->diffuse(Color::green() * glow);
        l->specular(Color::white() * glow);
        l->dist(item_dist);
        l->move(glm::vec3(glm::vec3(0.5f, 0.5f, 0.0f)));
        add(l);
        collapse();

    }
    else if (m_ThingID == Thing::HEART) {
        auto l = make_shared<Light>();
        l->ambient(Color::red() * glow);
        l->diffuse(Color::red() * glow);
        l->specular(Color::white() * glow);
        l->dist(item_dist);
        l->move(glm::vec3(glm::vec3(0.5f, 0.5f, 0.0f)));
        add(l);
        collapse();

    } else if (m_ThingID == Thing::KEY) {
        auto l = make_shared<Light>();
        string typ = config()->at<string>("type");
        auto col = typ == "red" ? Color::red() : Color::blue();

        l->ambient(col * glow);
        l->diffuse(col * glow);
        l->specular(Color::white() * glow);
        l->dist(item_dist);
        l->move(glm::vec3(glm::vec3(0.5f, 0.5f, 0.0f)));
        add(l);
        collapse();
    } else if (m_ThingID == Thing::DOOR) {
        m_Solid = true;
    }
}


void Thing :: sound(const std::string& fn) {
    Sound::play(this, fn, m_pResources);
}


void Thing :: setup_player(const std::shared_ptr<Sprite>& player) {}
void Thing :: setup_map(const std::shared_ptr<TileMap>& map) {}
void Thing :: setup_other(const std::shared_ptr<Thing>& thing) {}


unsigned Thing :: get_id(const std::shared_ptr<Meta>& config) {
    string name = config->at<string>("name", "");
    
    if (name.empty())
        return INVALID_THING;

    auto itr = std::find(ENTIRE(s_TypeNames),name);
    if (itr == s_TypeNames.end())
        return INVALID_THING;
    
    return std::distance(s_TypeNames.begin(), itr);
}


void Thing :: cb_to_player(Node* player_node, Node* thing_node) {
    auto thing = (Thing*)thing_node;

    if (thing->id() == Thing::STAR) {
        if(thing->placeholder()->visible()){
            thing->sound("pickup2.wav");

            // TODO: Replace with animation
            // thing->visible(false);

            // thing->placeholder()->visible(false);
            
            thing->m_ResetCon = thing->game()->on_reset.connect([thing]{
                // TODO: Replace with outlined or shadow picture
                thing->visible(true);
                thing->placeholder()->visible(true);
            });

            auto meta = make_shared<Meta>();
            meta->set<string>("type", thing->config()->at<string>("type"));
            player_node->parent()->event("star", meta);
        }
    } else if (thing->id() == Thing::HEART) {
        if (thing->placeholder()->visible()) {
            thing->sound("pickup.wav");
            thing->visible(false);
            thing->placeholder()->visible(false);
            thing->m_ResetCon = thing->game()->on_reset.connect([thing]{
                thing->visible(true);
                thing->placeholder()->visible(true);
            });
        }
    } else if(thing->id() == Thing::BATTERY) {
        if (thing->placeholder()->visible()) {
            thing->sound("pickup.wav");
            thing->visible(false);
            thing->placeholder()->visible(false);
            thing->m_ResetCon = thing->game()->on_reset.connect([thing]{
                thing->visible(true);
                thing->placeholder()->visible(true);
            });
            player_node->parent()->event("battery");
        }
    } else if (thing->id() == Thing::SPRING) {
        if (thing->hook_type<Sound>().empty())
            thing->sound("spring.wav");

        auto player = player_node->parent();// mask -> mesh -> sprite
        auto vel = player->velocity();

        player->velocity(glm::vec3(0.0f,
            vel.y > 250.0f ? -vel.y : -250.0f,
            0.0f)
        );
    } else if (thing->id() == Thing::KEY) {
        if (thing->placeholder()->visible()) {
            thing->sound("pickup.wav");
            thing->placeholder()->visible(false);

            auto layer = thing->m_pPlaceholder->tile_layer();
            auto keycol = thing->config()->at<string>("type");

            for (auto&& tile: thing->m_pPlaceholder->tile_layer()->tiles()) {
                if (not tile)
                    continue;

                for (auto&& ch: *tile) {
                    if (ch->name() == "door") {
                        auto col = ch->config()->at<string>("type","");

                        if (col == keycol) {
                            ch->parent()->visible(false);
                        }
                    }
                }
            }
        }
    } else if (thing->id() == Thing::DOOR) {
        if (thing->placeholder()->visible()) {
            thing->m_pGame->cb_to_tile(player_node, thing_node);

            if (thing->config()->at<string>("type") == "star") {
                thing->m_pGame->event("stardoor");
            }
        }
    } else if (thing->is_monster()) {
        if (thing->alive())
            thing->m_pGame->reset();
    }
}


void Thing :: cb_to_static(Node* thing_node, Node* static_node) {
    auto thing = thing_node->config()->at<Thing*>("thing",nullptr);

    if (not thing)
        return;

    if (thing->is_monster()) {
        if (thing->num_snapshots()) {
            if (static_node->world_box().center().x > thing->world_box().center().x) {
                thing->velocity(-abs(thing->velocity()));
                thing->sprite()->set_state("left");
            } else if (static_node->world_box().center().x < thing->world_box().center().x) {
                thing->velocity(abs(thing->velocity()));
                thing->sprite()->set_state("right");
            }
        }
    }
}


void Thing :: cb_to_bullet(Node* thing_node, Node* bullet) {
    auto thing = thing_node->config()->at<Thing*>("thing", nullptr);

    if (not thing)
        return;

    if (thing->is_monster() && thing->alive() && not bullet->detaching()) {
        thing->sound("damage.wav");

        if (thing->damage(bullet->config()->at("damage", 1))) {

            // Generate blood splatter when hit
            auto gibs = thing->m_Dying ? 5 : 20;
            for(int i = 0; i < gibs; ++i)
                thing->gib(bullet);

            // Knockback and stun
            auto vel = thing->velocity();
            thing->move(glm::vec3(-kit::sign(vel.x) * 5.0f, 0.0f, 0.0f));
            thing->stun();
            
            // Change direction based on bullet velocity
            if (bullet->velocity().x > K_EPSILON) {
                thing->velocity(-abs(thing->velocity()));
                thing->sprite()->set_state("left");
            } else if (bullet->velocity().x < -K_EPSILON) {
                thing->velocity(abs(thing->velocity()));
                thing->sprite()->set_state("right");
            }
            
            // Schedule detachment and activate thing
            bullet->safe_detach();
            thing->activate();
        }

        // Change color of thing based on health
        thing->m_pSprite->material()->ambient(kit::mix(
            Color::red(), Color::white(), thing->hp_fraction()
        ));
    }
}


void Thing :: stun() {
    m_pSprite->set_state("hit");
    m_StunTimer.set(Freq::Time::ms(200));
}


// Returns true if thing took damage. Applies damage if thing accepts damage.
bool Thing :: damage(int dmg) {
    if(m_HP <= 0 || dmg < 0)
        return false;

    m_HP = std::max(m_HP - dmg, 0);
    
    if (m_HP <= 0) {
        m_Dying = true;
        velocity(glm::vec3(0.0f));
    }
    
    return true;
}


void Thing :: logic_self(Freq::Time t) {
    clear_snapshots();
    snapshot();
    
    if (m_StunTimer.elapsed()) {
        m_pSprite->set_state("unhit");
        m_StunTimer.reset();
    }

    if (not alive())
        detach();
}


void Thing :: lazy_logic_self(Freq::Time t) {}


void Thing :: gib(Node* bullet) {
    auto gib = make_shared<Sprite>(m_pResources->transform("blood.json"), m_pResources);
    gib->set_state("animated");
    
    auto dir = Angle::degrees(1.0f * (std::rand() % 360)).vector();
    stick(gib);

    gib->move(glm::vec3(std::rand() % 16 - 8.0f, std::rand() % 32 - 16.0f, 2.0f));
    gib->velocity(glm::vec3(dir, 0.0f) * 100.0f);
    gib->acceleration(glm::vec3(0.0f, 500.0f, 0.0f));
    gib->scale(std::rand() % 100 / 100.0 * 0.5f);

    auto life = make_shared<float>(0.5f * (std::rand() % 4));
    auto gibptr = gib.get();

    gib->on_tick.connect([gibptr, life](Freq::Time t){
        *life -= t.s();

        if(*life < 0.0f)
            gibptr->detach();
    });
}


void Thing :: shoot(Sprite* origin) {
    auto shot = make_shared<Mesh>(
        make_shared<MeshGeometry>(Prefab::quad(glm::vec2(8.0f, 2.0f))),
        vector<shared_ptr<IMeshModifier>>{
            make_shared<Wrap>(Prefab::quad_wrap(
                glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 0.0f)
            ))
        },
        make_shared<MeshMaterial>("laser.png", m_pResources)
    );

    auto par = origin->parent();
    par->add(shot);

    shot->rotate(((std::rand() % 10)-5) / 360.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    shot->velocity(shot->orient_to_world(glm::vec3(
        (origin->check_state("left")?-1.0f:1.0f) * 256.0f,
        0.0f, 0.0f
    )));
    auto timer = make_shared<Freq::Alarm>(m_pTimeline);
    timer->set(Freq::Time::seconds(0.5f));
    auto shotptr = shot.get();
    shot->on_tick.connect([timer,shotptr](Freq::Time t){
        if(timer->elapsed())
            shotptr->detach();
    });
    
    
    Sound::play(origin, "shoot.wav", m_pResources);

    // increase box Z width
    auto shotbox = shot->box();
    shot->set_box(Box(
        vec3(shotbox.min().x, shotbox.min().y, -5.0),
        vec3(shotbox.max().x, shotbox.max().y, 5.0)
    ));
}


void Thing :: activate() {
    if (is_monster()) {
        if (m_ThingID == MOUSE) {
            shoot(m_pSprite.get());
        }
    }
}


void Thing :: register_player(Sprite* p) {
    m_Players.push_back(p);
}
