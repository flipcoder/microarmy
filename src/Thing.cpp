#include "Thing.h"
#include "Game.h"
#include "Qor/Sprite.h"

using namespace std;
using namespace glm;


const std::vector<std::string> Thing :: s_TypeNames({
    "",
    
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


void Thing :: initialize() {
    assert(m_pPartitioner);

    m_Box = m_pPlaceholder->box();

    m_pPartitioner->register_object(shared_from_this(), Game::THING);
    
    const float item_dist = 200.0f;
    const float glow = 1.0f;

    if (m_ThingID == Thing::STAR) {
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
    }
}


void Thing :: cb_to_static(Node* thing_node, Node* static_node) {
    auto thing = thing_node->config()->at<Thing*>("thing",nullptr);

    if (not thing)
        return;
}


void Thing :: cb_to_bullet(Node* thing_node, Node* bullet) {
    auto thing = thing_node->config()->at<Thing*>("thing", nullptr);

    if (not thing)
        return;
}


//void Thing :: logic_self(Freq::Time t) {}
//void Thing :: lazy_logic_self(Freq::Time t) {}


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

