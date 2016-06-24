#ifndef THING_CPP_USLBKY9A
#define THING_CPP_USLBKY9A

#include <memory>
#include "Qor/TileMap.h" 
#include "Qor/BasicPartitioner.h"

class Monster: public Node {
	public:
		enum Type {
			NONE = 0,

			DUCK,
			MOUSE,
			ROBOT,
			SNAIL,
			WIZARD,
		}

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
        virtual ~Monster();


        // Setters (NOT CURRENTLY USED)
        void set_player(const std::shared_ptr<Sprite>& player);
        void set_map(const std::shared_ptr<TileMap>& map);
        void set_other(const std::shared_ptr<Thing>& thing);


        // Getters
        static unsigned get_type(const std::shared_ptr<Meta>& config);
		Game* get_game() { return m_pGame; }
		Sprite* get_sprite() { return m_pSprite.get(); }
		Maptile* get_placeholder() { return m_pPlaceholder; }


        // Methods
		void activate();
        void deactivate();
		void damage(int dmg);
		void shoot();
		void stun();
		void gib(Node* bullet);
		void sound();


		// Callbacks
        static void cb_to_bullet(Node* thing_node, Node* bullet);
        static void cb_to_static(Node* thing_node, Node* static_node);
        static void cb_to_player(Node* player_node, Node* thing_node);

	private:
        const static std::vector<std::string> s_TypeNames;
        
		unsigned m_ThingID = 0;
        int m_HP = 1;
        int m_MaxHP = 1;
        float m_StartSpeed = 0.0f;
        float m_Speed = 0.0f;
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


		// ground detection for monsters
        std::shared_ptr<Mesh> m_pLeft;
        std::shared_ptr<Mesh> m_pRight;


		// List of players thing knows about
        std::vector<Sprite*> m_Players;
}