#ifndef THING_CPP_USLBKY9A
#define THING_CPP_USLBKY9A

#include <memory>
#include "Qor/TileMap.h" 
#include "Qor/BasicPartitioner.h"


class Game;
class Sprite;

class Thing: public Node {
    public:
        // Definitions
        enum Type {
            INVALID_THING = 0,

            ITEMS,
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

        
        // Static Methods
        static unsigned get_id(const std::shared_ptr<Meta>& config);
        static bool is_thing(std::string name);
        static std::shared_ptr<Thing> find_thing(Node* n);


        // Overriden virtual methods
        //virtual void lazy_logic_self(Freq::Time t) override;
        //virtual void logic_self(Freq::Time t) override;


        // Setters
        void initialize(); // TODO: Put this in the constructor? -- has to happen after object add()ed
        void setup_player(const std::shared_ptr<Sprite>& player);
        void setup_map(const std::shared_ptr<TileMap>& map);
        void setup_other(const std::shared_ptr<Thing>& thing);


        // Getters
        unsigned id() const { return m_ThingID; }
        bool is_item() const { return m_ThingID >= ITEMS && m_ThingID < ITEMS_END; }
        bool is_object() const { return m_ThingID >= OBJECTS && m_ThingID < OBJECTS_END; }
        bool is_marker() const { return m_ThingID >= MARKERS && m_ThingID < MARKERS_END; }
        bool solid() const { return m_Solid; }
        Freq::Timeline* timeline() const { return m_pTimeline; }
        Game* game() { return m_pGame; }
        Sprite* sprite() { return m_pSprite.get(); }
        MapTile* placeholder() { return m_pPlaceholder; }


        // Methods
        bool damage(int dmg);
        void sound(const std::string& fn);
        void origin();
        

        // Callbacks
        static void cb_to_bullet(Node* thing_node, Node* bullet);
        static void cb_to_static(Node* thing_node, Node* static_node);
        static void cb_to_player(Node* player_node, Node* thing_node);


    private:
        // Static variables
        const static std::vector<std::string> s_TypeNames;
        

        // Variables
        unsigned m_ThingID = 0;
        bool m_Collidable = true;
        bool m_Dying = false;
        bool m_Dead = false;
        bool m_Solid = false;
        bool m_Active = false;

        std::string m_Identity;
        glm::vec3 m_Impulse;
        Freq::Alarm m_StunTimer;
        boost::signals2::scoped_connection m_ResetCon;


        // Pointers
        Cache<Resource, std::string>* m_pResources = nullptr;
        MapTile* m_pPlaceholder = nullptr;
        BasicPartitioner* m_pPartitioner = nullptr;
        Game* m_pGame = nullptr;
        TileMap* m_pMap = nullptr;
        Freq::Timeline* m_pTimeline;

        std::shared_ptr<Sprite> m_pSprite;
};

#endif
