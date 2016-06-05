#include "Thing.h"
#include "Game.h"
using namespace std;

const std::vector<std::string> Thing :: s_TypeNames({
    "",
    
    // monsters
    "mouse",
    
    // items
    "battery",
    "heart",
    "star",
    
    // objects
    "spring",
});

Thing :: Thing(
    const std::shared_ptr<Meta>& config,
    MapTile* placeholder,
    Game* game,
    TileMap* map,
    BasicPartitioner* partitioner,
    Cache<Resource, std::string>* resources
):
    Node(config),
    m_pPlaceholder(placeholder),
    m_pPartitioner(partitioner),
    m_pGame(game),
    m_pMap(map),
    m_pResources(resources),
    m_Identity(config->at<string>("name","")),
    m_ThingID(get_id(config))
{
}

std::shared_ptr<Thing> Thing :: find_thing(Node* n)
{
    shared_ptr<Thing> thing;
    thing = dynamic_pointer_cast<Thing>(n->as_node());
    if(not thing){
        thing = dynamic_pointer_cast<Thing>(n->parent()->as_node());
        if(not thing)
            thing = dynamic_pointer_cast<Thing>(n->parent()->parent()->as_node());
    }
    return thing;
}

void Thing :: init_thing()
{
    assert(m_pPartitioner);
    m_Box = m_pPlaceholder->box();

    m_pPartitioner->register_object(shared_from_this(), Game::THING);

    if(m_ThingID == Thing::STAR) {
        auto l = make_shared<Light>();
        string type = config()->at<string>("type");
        //if(type == "gold"){
            l->ambient(Color::white());
            l->diffuse(Color::white());
        //}else if(type == "silver"){
        //    l->ambient(Color::gray());
        //    l->diffuse(Color::gray());
        //}else if(type == "bronze"){
        //    l->ambient(Color("8c7853"));
        //    l->diffuse(Color("8c7853"));
        //}
        //l->diffuse(Color::yellow());
        l->specular(Color::black());
        l->dist(50.0f);
        l->move(glm::vec3(glm::vec3(0.5f, 0.5f, 0.0f)));
        stick(l);
    }
    else if(m_ThingID == Thing::BATTERY) {
        auto l = make_shared<Light>();
        l->ambient(Color::green());
        l->diffuse(Color::green());
        l->specular(Color::black());
        l->dist(50.0f);
        l->move(glm::vec3(glm::vec3(0.5f, 0.5f, 0.0f)));
        stick(l);
    }
    else if(m_ThingID == Thing::HEART) {
        auto l = make_shared<Light>();
        l->ambient(Color::red());
        l->diffuse(Color::red());
        l->specular(Color::black());
        l->dist(50.0f);
        l->move(glm::vec3(glm::vec3(0.5f, 0.5f, 0.0f)));
        stick(l);
    }
}

void Thing :: sound(const std::string& fn)
{
    Sound::play(this, fn, m_pResources);
}

void Thing :: setup_player(const std::shared_ptr<Sprite>& player)
{
}

void Thing :: setup_map(const std::shared_ptr<TileMap>& map)
{
}

void Thing :: setup_other(const std::shared_ptr<Thing>& thing)
{
}

unsigned Thing :: get_id(const std::shared_ptr<Meta>& config)
{
    string name = config->at<string>("name","");
    
    if(name.empty())
        return INVALID_THING;
    auto itr = std::find(ENTIRE(s_TypeNames),name);
    if(itr == s_TypeNames.end())
        return INVALID_THING;
    
    return std::distance(s_TypeNames.begin(), itr);
}

void Thing :: cb_to_player(Node* player_node, Node* thing_node)
{
    auto thing = (Thing*)thing_node;
    if(thing->id() == Thing::STAR){
        if(thing->placeholder()->visible()){
            thing->sound("pickup2.wav");
            thing->placeholder()->visible(false);
            thing->m_ResetCon = thing->game()->on_reset.connect([thing]{
                thing->placeholder()->visible(true);
            });
        }
    }else if(thing->id() == Thing::HEART){
        if(thing->placeholder()->visible()){
            thing->sound("pickup.wav");
            thing->placeholder()->visible(false);
            thing->m_ResetCon = thing->game()->on_reset.connect([thing]{
                thing->placeholder()->visible(true);
            });
        }
    }else if(thing->id() == Thing::BATTERY){
        if(thing->placeholder()->visible()){
            thing->sound("pickup.wav");
            thing->placeholder()->visible(false);
            thing->m_ResetCon = thing->game()->on_reset.connect([thing]{
                thing->placeholder()->visible(true);
            });
        }
    }else if(thing->id() == Thing::SPRING){
        if(thing->hook_type<Sound>().empty())
            thing->sound("spring.wav");
        auto player = player_node->parent();// mask -> mesh -> sprite
        auto vel = player->velocity();
        player->velocity(glm::vec3(0.0f,
            vel.y > 250.0f ? -vel.y : -250.0f,
            0.0f)
        );
    }
}

void Thing :: cb_to_static(Node* thing_node, Node* static_node)
{
}

void Thing :: cb_to_bullet(Node* thing_node, Node* bullet_node)
{
    Thing* thing = (Thing*)thing_node->parent()->parent();
    Node* bullet = bullet_node->parent();
    if(thing->is_monster() && thing->alive())
    {
        if(thing->damage(bullet->config()->at("damage",1)))
        {
            bullet->on_tick.connect([bullet](Freq::Time){
                bullet->detach();
            });
        }
    }
}

bool Thing :: damage(int dmg)
{
    if(m_HP <= 0 || dmg < 0)
        return false;
    m_HP = std::max(m_HP-dmg, 0);
    if(m_HP <= 0){
        m_Dying = true;
        velocity(glm::vec3(0.0f));
    }
    return true;
}

