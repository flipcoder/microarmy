#include "Thing.h"
#include "Qor/TileMap.h" 
#include "Qor/Sprite.h"


Monster(
    const std::shared_ptr<Meta>& config, 			// Parameters
    MapTile* placeholder,
    Game* game,
    TileMap* map,
    BasicPartitioner* partitioner,
    Freq::Timeline* timeline,
    Cache<Resource, std::string>* resources
:
    Node(config),									// Calls the Node Constructor
    m_pPlaceholder(placeholder),					// Set Monster Placeholder
    m_pPartitioner(partitioner),					// Set Monster Partitioner
    m_pGame(game),									// Set Monster Game
    m_pMap(map),									// Set Monster Map
    m_pResources(resources),						// Set Monster Resources
    m_ThingID(get_type(config)),					// Set Monster Type (int)
    m_Identity(config->at<string>("name", "")),		// Set Monster Type (String)
    m_StunTimer(timeline),							// Set Monster Stun Time (Alarm)
    m_pTimeline(timeline)							// Set Timeline
{}


// Methods
void activate() {}
void patrol() {}
void damage(int dmg) {}
void shoot() {}
void stun() {}
void gib(Node* bullet) {}
void sound() {}


// Callbacks
static void cb_to_bullet(Node* thing_node, Node* bullet) {}
static void cb_to_static(Node* thing_node, Node* static_node) {}
static void cb_to_player(Node* player_node, Node* thing_node) {}
