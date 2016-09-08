#ifndef PLAYER_H_E74SGBRG
#define PLAYER_H_E74SGBRG

#include "Qor/Sprite.h"
#include "Qor/Input.h"
#include "Qor/Camera.h"

class Game;

class Player: public Node {
    public:

        static const int INVINCIBLE_TIME = 1000;    //finish to hurt method
        static const int BLINK_TIME = 100;

        // Constructor
        Player(
            std::string fn,
            Cache<Resource, std::string>* resources,
            Camera* camera,
            Controller* ctrl,
            Freq::Timeline* timeline,
            IPartitioner* part,
            Game* game
        );


        // Destructor
        virtual ~Player();

        
        // Overidden virtual methods
        virtual void logic_self(Freq::Time t) override;


        // Getters
        std::shared_ptr<Node> focus_right() { return m_pCharFocusRight; }
        std::shared_ptr<Node> focus_left() { return m_pCharFocusLeft; }
        bool blinking() const { return m_Blinking; }
        bool god() const { return m_GodMode; }
        bool no_enemy_damage() const { return m_NoEnemyDamage; }
        bool no_fatal_objects() const { return m_NoFatalObjects; }
        bool prone() const { return m_Prone; }
        int get_health() { return m_Health; }

        // Methods
        void enter();
        void reset_walljump();
        void shoot(glm::vec2 dir);
        void face(glm::vec2 dir);
        void battery(int b) { m_Battery += b; }
        void god(bool b) { m_GodMode = b; }
        void blinking(bool b) { m_Blinking = b; }
        void blink();
        void reset();
        void prone(bool b);
        bool hurt(int damage);

        // Callbacks
        static void cb_to_bullet(Node* player_node, Node* bullet);
        
        Sprite* sprite() { return m_pChar.get(); }
        
    private:
        // Variables
        unsigned m_Battery = 0;
        int m_LastWallJumpDir = 0;
        bool m_WasInAir = false;
        // 28 July 2016 - KG: Added God Mode variables
        bool m_Blinking = false;
        bool m_GodMode = false; // NOTHING will kill player
        bool m_NoFatalObjects = false; // Only affects fatal objects (including Wizard's fire)
        bool m_NoEnemyDamage = false; // Only affects enemy overlap and bullets (but not Wizard's fire)
        bool m_Prone = false;
        int m_Health = 100;

        Freq::Alarm m_JumpTimer;
        Freq::Alarm m_ShootTimer;
        Freq::Alarm m_InvincibleTime;
        Freq::Alarm m_BlinkTime;
        
        // Pointers
        Game* m_pGame;
        Camera* m_pCamera;
        Controller* m_pController;
        IPartitioner* m_pPartitioner;
        Freq::Timeline* m_pTimeline;
        Cache<Resource, std::string>* m_pResources;

        std::shared_ptr<Sprite> m_pChar;
        std::shared_ptr<Sprite> m_pProne;
        std::shared_ptr<Node> m_pCharFocusLeft;
        std::shared_ptr<Node> m_pCharFocusRight;
};

#endif
