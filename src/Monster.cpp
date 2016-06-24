#include "Thing.h"
#include "Qor/TileMap.h" 
#include "Qor/Sprite.h"

using namespace std;


// Monster Constructor
Monster :: Monster(
    const shared_ptr<Meta>& config, 			// Parameters
    MapTile* placeholder,
    Game* game,
    TileMap* map,
    BasicPartitioner* partitioner,
    Freq::Timeline* timeline,
    Cache<Resource, string>* resources
:													/* Initializers */
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


// Static Member Variable Definitions
const vector<string> Monster :: s_TypeNames({
	// Match this to Monster Type enum
	"",
	"duck",
	"mouse",
	"robot",
	"snail",
	"wizard",
});



// Find the Monster Type enum value
unsigned Monster :: get_type(const shared_ptr<Meta>& config) {
	string name = config->at<string>("name", "");
    
    if (name.empty())
        return INVALID_MONSTER;

    // Determines if the name is in the Monster Typenames vector
    auto itr = find(ENTIRE(s_TypeNames), name);
    if (itr == s_TypeNames.end())
        return INVALID_MONSTER;
    
    return distance(s_TypeNames.begin(), itr);
}


// Methods
void Monster :: activate() {
	// Trigger Monster activated behavior

	// Activated behavior can be modeled with Qor states for Nodes
}
void Monster :: deactivate() {}
void Monster :: damage(int dmg) {}
void Monster :: shoot() {}
void Monster :: stun() {}
void Monster :: gib(Node* bullet) {}
void Monster :: sound() {}


// Callbacks
void Monster :: cb_to_bullet(Node* thing_node, Node* bullet) {}
void Monster :: cb_to_static(Node* thing_node, Node* static_node) {}
void Monster :: cb_to_player(Node* player_node, Node* thing_node) {}
