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

    // ...
}

void Thing :: sound(const std::string& fn)
{
    Sound::play(this, fn, m_pResources);
}

//void Thing :: setup_player(const std::shared_ptr<Character>& player)
//{
//}

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

//void Thing :: cb_to_player(Node* player_node, Node* thing_node)
//{
    
//}

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

