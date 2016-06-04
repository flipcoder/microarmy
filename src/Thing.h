#ifndef THING_CPP_USLBKY9A
#define THING_CPP_USLBKY9A

#include <memory>
#include "Qor/Sprite.h"
#include "Qor/TileMap.h" 
#include "Qor/BasicPartitioner.h"
class Game;

class Thing:
    public Node
{
    public:

        Thing(
            const std::shared_ptr<Meta>& config,
            MapTile* placeholder,
            Game* game,
            TileMap* map,
            BasicPartitioner* partitioner,
            Cache<Resource, std::string>* resources
        );
        virtual ~Thing() {}

        void init_thing();
        
        //void setup_player(const std::shared_ptr<Character>& player);
        void setup_map(const std::shared_ptr<TileMap>& map);
        void setup_other(const std::shared_ptr<Thing>& thing);

        static unsigned get_id(const std::shared_ptr<Meta>& config);
        static bool is_thing(std::string name);

        enum Type
        {
            INVALID_THING = 0,

            MONSTERS,
            MOUSE = MONSTERS,
            MONSTERS_END,

            ITEMS = MONSTERS_END,
            BATTERY = ITEMS,
            HEART,
            STAR,
            ITEMS_END
        };
        
        const static std::vector<std::string> s_TypeNames;

        bool is_monster() const {
            return m_ThingID >= MONSTERS && m_ThingID < MONSTERS_END;
        }
        bool is_item() const {
            return m_ThingID >= ITEMS && m_ThingID < ITEMS_END;
        }
        //bool is_weapon() const {
        //    return m_ThingID >= WEAPONS && m_ThingID < WEAPONS_END;
        //}

        bool alive() const { return not m_Dead; }

        bool damage(int dmg);

        bool solid() const { return m_Solid; }
        unsigned id() const { return m_ThingID; }
        
        static std::shared_ptr<Thing> find_thing(Node* n);

        Game* game() { return m_pGame; }
        
        void cb_to_bullet(Node* thing_node, Node* bullet_node);
        void cb_to_static(Node* thing_node, Node* static_node);
        //void cb_to_player(Node* player_node, Node* thing_node)
        
        void sound(const std::string& fn);
        
    private:
        
        int m_HP = 1;
        bool m_Dying = false;
        bool m_Dead = false;
        Cache<Resource, std::string>* m_pResources = nullptr;
        bool m_bActive = false;
        MapTile* m_pPlaceholder = nullptr;
        BasicPartitioner* m_pPartitioner = nullptr;
        Game* m_pGame = nullptr;
        TileMap* m_pMap = nullptr;
        bool m_Solid = false;
        std::shared_ptr<Sprite> m_pSprite; // optional for thing type
        std::string m_Identity;
        unsigned m_ThingID = 0;
};

#endif

