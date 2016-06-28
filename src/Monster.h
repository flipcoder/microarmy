// Add definitions
// #ifndef THING_CPP_USLBKY9A
// #define THING_CPP_USLBKY9A

#include <memory>
#include "Qor/TileMap.h" 
#include "Qor/BasicPartitioner.h"


class Game;
class Sprite;

class Monster: public Node {
    public:
        enum Type {
            NONE = 0,

            DUCK,
            MOUSE,
            ROBOT,
            SNAIL,
            WIZARD,
        };
        static constexpr float DEFAULT_BULLET_SPEED = 256.0f;
        static const int DEFAULT_STUN_TIME = 200;

        // Constructor
        Monster(
            const std::shared_ptr<Meta>& config,
            MapTile* placeholder,
            Game* game,
            TileMap* map,
            BasicPartitioner* partitioner,
            Freq::Timeline* timeline,
            Cache<Resource, std::string>* resources
        );


        // Destructor
        virtual ~Monster() {}


        // Abstract Methods
        // virtual void lazy_logic_self(Freq::Time t) override;
        virtual void logic_self(Freq::Time t) override;


        // Setters (NOT CURRENTLY USED)
        void player(const std::shared_ptr<Sprite>& player);
        void map(const std::shared_ptr<TileMap>& map);
        void other(const std::shared_ptr<Monster>& thing);

        // Getters
        static unsigned get_type(const std::shared_ptr<Meta>& config);
        bool is_alive() const { return not m_Dead and not m_Dying; }
        int hp() { return m_HP; }
        int max_hp() { return m_MaxHP; }
        Game* game() { return m_pGame; }
        Sprite* sprite() { return m_pSprite.get(); }
        MapTile* placeholder() { return m_pPlaceholder; }


        // Methods
        void initialize();
        void activate();
        void deactivate();
        void damage(int dmg);
        void shoot(Sprite* origin, float bullet_speed=DEFAULT_BULLET_SPEED);
        void stun(int m_StunTime);
        void gib();
        void sound(const std::string& fn);


        // Callbacks
        static void cb_to_bullet(Node* monster_node, Node* bullet);
        static void cb_to_static(Node* monster_node, Node* static_node);
        static void cb_to_player(Node* player_node, Node* monster_node);
        static void cb_sensor_to_no_static(Node* sensor_node, Node* static_node);

    private:
        const static std::vector<std::string> s_TypeNames;
        
        unsigned m_MonsterID = 0;
        int m_HP = 1;
        int m_MaxHP = 1;
        int m_StunTime = 0;
        float m_StartSpeed = 0.0f;
        float m_Speed = 0.0f;
        float m_BulletSpeed = 0;
        bool m_Dying = false;
        bool m_Dead = false;
        bool m_Solid = false;
        bool m_bActive = false;


        std::string m_Identity; // String version of Type
        glm::vec3 m_Impulse;
        Freq::Alarm m_StunTimer;
        boost::signals2::scoped_connection m_ResetCon;


        Cache<Resource, std::string>* m_pResources = nullptr;
        MapTile* m_pPlaceholder = nullptr;
        BasicPartitioner* m_pPartitioner = nullptr;
        Game* m_pGame = nullptr;
        TileMap* m_pMap = nullptr;
        Freq::Timeline* m_pTimeline;

        // sprite is optional for thing type, not attached
        std::shared_ptr<Sprite> m_pSprite;

        // ground detection for monsters
        std::shared_ptr<Mesh> m_pLeft;
        std::shared_ptr<Mesh> m_pRight;

        // List of players Monster knows about
        std::vector<Sprite*> m_Players;
};
