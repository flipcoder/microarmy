#ifndef THING_CPP_USLBKY9A
#define THING_CPP_USLBKY9A

#include <memory>
#include "Qor/TileMap.h"
#include "Qor/BasicPartitioner.h"
class Game;
class Sprite;

class Thing : public Node {
    public:
        enum Type {
            INVALID_THING = 0,

            MONSTERS,
                MOUSE = MONSTERS,
                SNAIL,
                WIZARD,
                ROBOT,
                DUCK,
            MONSTERS_END,

            ITEMS = MONSTERS_END,
                BATTERY = ITEMS,
                HEART,
                STAR,
                KEY,
            ITEMS_END,

            OBJECTS = ITEMS_END,
                SPRING = OBJECTS,
                DOOR,
            OBJECTS_END,
            
            MARKERS = OBJECTS_END,
                LIGHT = MARKERS,
            MARKERS_END
        };


        // Constructor
        Thing(
            const std::shared_ptr<Meta>& config,
            MapTile* placeholder,
            Game* game,
            TileMap* map,
            BasicPartitioner* partitioner,
            Freq::Timeline* timeline,
            Cache<Resource, std::string>* resources
        );

        // Destructor
        virtual ~Thing() {}


        // Static Variables
        const static std::vector<std::string> s_TypeNames;


        // Getters
        unsigned id() const { return m_ThingID; }
        Game* game() { return m_pGame; }
        MapTile* placeholder() { return m_pPlaceholder; }
        Sprite* sprite() { return m_pSprite.get(); }
        static unsigned get_id(const std::shared_ptr<Meta>& config);
        float hp_fraction() { return m_HP * 1.0f / m_MaxHP; }


        // Properties
        static bool is_thing(std::string name);
        bool solid() const { return m_Solid; }
        bool alive() const { return not m_Dead and not m_Dying; }
        bool is_monster() const {
            return m_ThingID >= MONSTERS && m_ThingID < MONSTERS_END; }
        bool is_item() const {
            return m_ThingID >= ITEMS && m_ThingID < ITEMS_END; }
        bool is_object() const {
            return m_ThingID >= OBJECTS && m_ThingID < OBJECTS_END; }
        bool is_marker() const {
            return m_ThingID >= MARKERS && m_ThingID < MARKERS_END; }
        // bool is_weapon() const {
        //    return m_ThingID >= WEAPONS && m_ThingID < WEAPONS_END; }


        // Methods
        void init_thing();
        
        void register_player(Sprite* p);
        void setup_player(const std::shared_ptr<Sprite>& player);
        void setup_map(const std::shared_ptr<TileMap>& map);
        void setup_other(const std::shared_ptr<Thing>& thing);

        void stun();
        void origin();
        void shoot(Sprite* origin);
        void activate();
        void gib(Node* bullet);
        void sound(const std::string& fn);
        bool damage(int dmg);
        
        static std::shared_ptr<Thing> find_thing(Node* n);


        // Virtual Functions
        virtual void lazy_logic_self(Freq::Time t) override;
        virtual void logic_self(Freq::Time t) override;


        // Callbacks
        static void cb_to_bullet(Node* thing_node, Node* bullet);
        static void cb_to_static(Node* thing_node, Node* static_node);
        static void cb_to_player(Node* player_node, Node* thing_node);


    private:
        // Member Properties
        unsigned m_ThingID = 0;
        int m_HP = 1;
        int m_MaxHP = 1;
        float m_StartSpeed = 0.0f;
        float m_Speed = 0.0f;

        bool m_Dying = false;
        bool m_Dead = false;
        bool m_bActive = false;
        bool m_Solid = false;

        std::string m_Identity;
        glm::vec3 m_Impulse;
        std::vector<Sprite*> m_Players;
        boost::signals2::scoped_connection m_ResetCon;

        // Pointers
        MapTile* m_pPlaceholder = nullptr;
        BasicPartitioner* m_pPartitioner = nullptr;
        Game* m_pGame = nullptr;
        TileMap* m_pMap = nullptr;
        
        // Pointers - Ground Detection (Monsters)
        std::shared_ptr<Mesh> m_pLeft;
        std::shared_ptr<Mesh> m_pRight;

        // sprite is optional for thing type, not attached
        std::shared_ptr<Sprite> m_pSprite;

        Freq::Alarm m_StunTimer;
        Freq::Timeline* m_pTimeline;

        // Cache
        Cache<Resource, std::string>* m_pResources = nullptr;
};

#endif
