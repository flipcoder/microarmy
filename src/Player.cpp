#include "Player.h"
#include "Qor/Sound.h"
#include "Game.h"
#include "Monster.h"
#include "Qor/Qor.h"

using namespace std;
using namespace glm;


Player :: Player(
    std::string fn,
    Cache<Resource, std::string>* resources,
    Camera* camera,
    Controller* ctrl,
    Freq::Timeline* timeline,
    IPartitioner* part,
    Game* game
):
    //Sprite(fn, resources),
    m_pTimeline(timeline),
    m_pResources(resources),
    m_JumpTimer(timeline),
    m_ShootTimer(timeline),
    m_InvincibleTime(timeline),
    m_BlinkTime(timeline),
    m_pCamera(camera),
    m_pController(ctrl),
    m_pPartitioner(part),
    m_pGame(game)
{
    //m_pCamera->position(glm::vec3(-64.0f, -64.0f, 0.0f));
    position(glm::vec3(0.0f, 0.0f, 1.0f));

    m_pChar = make_shared<Sprite>(fn,resources);
    m_pChar->config()->set<Player*>("player", this);
    m_pChar->set_states({"stand", "right", "forward"});
    
    m_pProne = make_shared<Sprite>(resources->transform("guy-prone.json"),resources);
    m_pProne->set_states({"right"});
    m_pProne->visible(false);
    
    m_pCharFocusRight = make_shared<Node>();
    m_pCharFocusRight->position(glm::vec3(32.0f, 0.0f, 0.0f));
    m_pCharFocusLeft = make_shared<Node>();
    m_pCharFocusLeft->position(glm::vec3(-32.0f, 0.0f, 0.0f));
    add(m_pChar);
    add(m_pProne);
    add(m_pCharFocusRight);
    add(m_pCharFocusLeft);
    m_JumpTimer.set(Freq::Time::ms(0));
    m_ShootTimer.set(Freq::Time::ms(0));
}


Player :: ~Player() {
    
}


void Player :: enter() {
    acceleration(glm::vec3(0.0f, 500.0f, 0.0f));
}


void Player :: logic_self(Freq::Time t) {
    Node::logic_self(t);

    if (m_InvincibleTime.elapsed()){
        blinking(false);
        m_pProne->visible(m_Prone);
        m_pChar->visible(not m_Prone);
        m_InvincibleTime.reset();
        m_BlinkTime.reset();
    }
    else if (m_BlinkTime.elapsed()){
        if (m_Prone)
            m_pProne->visible(!m_pProne->visible());
        else
        m_pChar->visible(!m_pChar->visible());
        m_BlinkTime.set(Freq::Time::ms(BLINK_TIME));
    }

    auto feet_colliders = m_pGame->get_static_collisions(
        Node::find("feetmask").at(0)
    );

    //auto wall_colliders = m_pPartitioner->get_collisions_for(
    //    Node::find("sidemask").at(0), STATIC
    //);

    auto wall_colliders = m_pGame->get_static_collisions(
        Node::find("sidemask").at(0)
    );

    if (not feet_colliders.empty() && wall_colliders.empty()) {
        auto v = velocity();
        velocity(0.0f, v.y, v.z);
    }
    
    auto vel = velocity();
    bool in_air = feet_colliders.empty();
    bool walljump = feet_colliders.empty() && not wall_colliders.empty();

    if (walljump)
        m_pChar->set_state("walljump");
    else if (in_air)
        m_pChar->set_state("jump");
    
    glm::vec3 move(0.0f);

    if (velocity().x > -K_EPSILON && velocity().x < K_EPSILON) {
        if (m_pController->button("left")) {
            m_pCamera->track(focus_left());
            move += glm::vec3(-1.0f, 0.0f, 0.0f);
            //m_pChar->set_state("left");
        }
        if (m_pController->button("right")) {
            m_pCamera->track(focus_right());
            move += glm::vec3(1.0f, 0.0f, 0.0f);
            //m_pChar->set_state("right");
        }
    }
    // 29 July 2016 - KG: Added buttons for God Mode. See "default.json" in profiles to change buttons
    if (m_pController->button("NoEnemyDamage").pressed_now())
        m_NoEnemyDamage = !m_NoEnemyDamage;
    
    if (m_pController->button("NoFatalObjects").pressed_now())
        m_NoFatalObjects = !m_NoFatalObjects;
    
    if (m_pController->button("God").pressed_now())
        god(!m_GodMode);

    if (m_pController->button("NextLevel").pressed_now())
        next_level();

    if (m_pController->button("PreviousLevel").pressed_now())
        previous_level();

    // aim+shoot logic
    float xpres = m_pController->button("aimright").pressure() -
        m_pController->button("aimleft").pressure();
    float ypres = m_pController->button("aimdown").pressure() -
        m_pController->button("aimup").pressure();
    if(std::abs(xpres) > 0.25f || std::abs(ypres) > 0.25f){
        vec2 dir = glm::normalize(vec2(xpres,ypres));
        face(dir);
        if(m_ShootTimer.elapsed())
            shoot(dir);
    }

    const float min_pressure = 0.25f;
    
    if(xpres < -min_pressure){
        m_pChar->set_state("left");
        if(std::abs(ypres) < min_pressure)
            m_pChar->set_state("forward");
    }
    if(xpres > min_pressure){
        m_pChar->set_state("right");
        if(std::abs(ypres) < min_pressure)
            m_pChar->set_state("forward");
    }
    
    if(xpres < -min_pressure){
        m_pChar->set_state("left");
        if(std::abs(ypres) < min_pressure)
            m_pChar->set_state("forward");
    }
    if(xpres > min_pressure){
         m_pChar->set_state("right");
         if(std::abs(ypres) < min_pressure)
             m_pChar->set_state("forward");
    }

    bool block_jump = false;
    if (m_pController->button("up").pressure() > 0.8f || m_pController->button("jump")) {

        if (!m_Prone) {

        if (walljump || not in_air || not m_JumpTimer.elapsed()) {
            float x = 0.0f;

            if (walljump) {
                auto last_dir = m_LastWallJumpDir;
                auto tile_loc = wall_colliders.at(0)->world_box().center().x;

                if (tile_loc < position(Space::WORLD).x + 4.0f) {
                    m_LastWallJumpDir = -1;
                }
                else {
                    m_LastWallJumpDir = 1;
                }

                // prevent "climbing" the wall by checking to make sure last wall jump was a diff direction (or floor)
                // 0 means floors, -1 and 1 are directions
                if (m_LastWallJumpDir != 0 and last_dir != 0 and m_LastWallJumpDir == last_dir)
                    block_jump = true;
            }

            // jumping from floor or from a different wall
            if (not block_jump) {
                if (not in_air || walljump || not m_JumpTimer.elapsed()) {
                    velocity(glm::vec3(x, -125.0f, 0.0f));

                    if (not in_air || walljump) {
                        auto sounds = m_pCamera->find_type<Sound>();

                        if (sounds.empty())
                            Sound::play(m_pCamera, "jump.wav", m_pResources);
                        m_JumpTimer.set(Freq::Time::ms(200));
                    }
                }
            }
            else {
                // jumping on same wall? only allow this if jump timer is still running
                if (not m_JumpTimer.elapsed())
                    velocity(glm::vec3(x, -125.0f, 0.0f));
            }
        }
    }
        }
        else {
            m_JumpTimer.set(Freq::Time::ms(0));
        }

        if (not in_air and m_WasInAir)
            Sound::play(m_pCamera, "touch.wav", m_pResources);

        m_WasInAir = in_air;

        if (not in_air)
            m_LastWallJumpDir = 0;

        if (glm::length(move) > K_EPSILON) {
            if (not in_air)
                m_pChar->set_state("walk");

            move = glm::normalize(move);
            //wrp in if, if abs val x pressure < k_epsilon
            if (std::abs(xpres) < min_pressure) {

                if (std::abs(xpres) < min_pressure) {
                    if (move.x < -min_pressure) {
                        m_pChar->set_state("left");
                    }
                    else if (move.x > min_pressure) {
                        m_pChar->set_state("right");
                    }

                }
            }

            move *= 100.0f * t.s();
            clear_snapshots();
            snapshot();
            this->move(move);
        }
        else {
            if (not in_air)
                m_pChar->set_state("stand");

            clear_snapshots();
            snapshot();
        }

		if (not in_air) {
			prone(m_pController->button("down").pressure() > 0.8f);
		}
        //if (m_pController->button("left") or m_pController->button("right")) {
        //    if (m_pController->button("up"))
        //        m_pChar->set_state("upward");
        //    else if(m_pController->button("down"))
        //        m_pChar->set_state("downward");
        //    else
        //        m_pChar->set_state("forward");
        //} else {
        //    if (m_pController->button("up"))
        //        m_pChar->set_state("up");
        //    else if (m_pController->button("down"))
        //        m_pChar->set_state("down");
        //    else
        //        m_pChar->set_state("forward");
        //}


        //////// RAY CASTING CODE //////////

        // Get player vision origin
        auto orig = m_pChar->origin();
        auto vorig = m_pChar->vorigin();
        auto s = vec2(this->size());

        auto vorig_object = (vorig - orig) * s;
        auto vorig_world = this->to_world(vec3(vorig_object, 0.0f));

        // LOGf("Player Origin: %s", Vector::to_string(orig));
        // LOGf("Player Vision Origin: %s", Vector::to_string(vorig));
        // LOGf("Player Size: %s", Vector::to_string(s));

        // LOGf("Player Vision Origin [Percentage]: %s", Vector::to_string(vorig));
        // LOGf("Player Vision Origin [Object Space]: %s", Vector::to_string(vorig_object));
        // LOGf("Player Vision Origin [World Space]: %s", Vector::to_string(vorig_world));

        vec3 ray[2] = { vorig_world, vec3(0.0f, 0.0f, 0.0f) };

        // Computing pixel size difference between object space and world space for width
        auto world_vec = this->to_world(vec3(1.0f, 1.0f, 0.0f));

        //LOGf("Player Origin: %s", Vector::to_string(world_vec));

        auto line = Mesh::line(
            ray[0], // start
            ray[1], // end
            m_pResources->cache_as<Texture>("white.png"), // tex
            1.0f// width
        );

        //if (m_pController->button("left") || m_pController->button("right")) {
        //    if (m_pController->button("up"))
        //        m_pChar->set_state("upward");
        //    else if(m_pController->button("down"))
        //        m_pChar->set_state("downward");
        //    else
        //        m_pChar->set_state("forward");
        //} else {
        //    if (m_pController->button("up"))
        //        m_pChar->set_state("up");
        //    else if (m_pController->button("down"))
        //        m_pChar->set_state("down");
        //    else
        //        m_pChar->set_state("forward");
        //}

        ////////// RAY CASTING TEST //////////

        //// Get player vision origin
        //auto orig = this->origin();
        //auto vorig = this->vorigin();
        //auto s = vec2(this->size());

        //auto vorig_object = (vorig - orig) * s;
        //auto vorig_world = this->to_world(vec3(vorig_object, 0.0f));

        //// LOGf("Player Origin: %s", Vector::to_string(orig));
        //// LOGf("Player Vision Origin: %s", Vector::to_string(vorig));
        //// LOGf("Player Size: %s", Vector::to_string(s));

        //// LOGf("Player Vision Origin [Percentage]: %s", Vector::to_string(vorig));
        //// LOGf("Player Vision Origin [Object Space]: %s", Vector::to_string(vorig_object));
        //// LOGf("Player Vision Origin [World Space]: %s", Vector::to_string(vorig_world));

        //vec3 ray[2] = {vorig_world, vec3(0.0f, 0.0f, 0.0f)};

        //auto line = Mesh::line(
        //   ray[0], // start
        //   ray[1], // end
        //   m_pResources->cache_as<Texture>("white.png"), // tex
        //   1.0f // width
        //);

        //// Line from vision origin to world origin
        //auto lineptr = line.get();
        //line->on_tick.connect([lineptr](Freq::Time t){
        //    lineptr->detach();
        //});

        //m_pGame->root()->add(line);

        //// Gather list of all nodes
        //// auto nodes = m_pGame->root()->descendants();
        //// vector<shared_ptr<Node>> visible_nodes;

        //// int counter = 0;
        //// for (auto&& node: nodes) {
        ////     if (node->visible()) {
        ////         visible_nodes.push_back(node->as_node());
        ////         counter++;
        ////     }
        //// }

        //// LOG(to_string(counter));
    
}

// face a direction
void Player :: face(glm::vec2 dir) {

    auto ang = atan2(dir.y, dir.x) / K_TAU;

    // I don't want negatives
    if (ang <= 0.0f)
        ang += 1.0f;
    
    if (ang >= -1.0f/16.0f && ang <= 1.0f/16.0f) {
        m_pChar->set_state("right");
        m_pChar->set_state("forward");
    } else if (ang >= 1.0f/8.0f - 1.0f/16.0f && ang <= 1.0f/8.0f + 1.0f/16.0f) {
        m_pChar->set_state("downward");
        m_pChar->set_state("right");
    } else if (ang >= 2.0f/8.0f - 1.0f/16.0f && ang <= 2.0f/8.0f + 1.0f/16.0f) {
        m_pChar->set_state("down");
    } else if (ang >= 3.0f/8.0f - 1.0f/16.0f && ang <= 3.0f/8.0f + 1.0f/16.0f) {
        m_pChar->set_state("downward");
        m_pChar->set_state("left");
    } else if (ang >= 4.0f/8.0f - 1.0f/16.0f && ang <= 4.0f/8.0f + 1.0f/16.0f) {
        m_pChar->set_state("left");
        m_pChar->set_state("forward");
    } else if (ang >= 5.0f/8.0f - 1.0f/16.0f && ang <= 5.0f/8.0f + 1.0f/16.0f) {
        m_pChar->set_state("upward");
        m_pChar->set_state("left");
    } else if (ang >= 6.0f/8.0f - 1.0f/16.0f && ang <= 6.0f/8.0f + 1.0f/16.0f) {
        m_pChar->set_state("up");
    } else if (ang >= 7.0f/8.0f - 1.0f/16.0f && ang <= 7.0f/8.0f + 1.0f/16.0f) {
        m_pChar->set_state("upward");
        m_pChar->set_state("right");
    }
}

void Player :: shoot(glm::vec2 dir) {
    
    /* ***** Weapon Upgrades *****
    * big bullet, wave(shotgun?), flamethrower
    * check condition for speed
    */
    string gfx;
    //base
    if (m_Battery == 0)
        gfx = "laser";
    //upgrade 2: big bullet (laser_nrm placehold)
    if (m_Battery == 1)
        gfx = "laser_NRM";
    //upgrade 3: wave/shotgun
    if (m_Battery == 2)
        gfx = "blood";
    //upgrade 4: flamethrower
    if (m_Battery == 3)
        gfx = "fireball";
    if (m_Battery > 3)
        gfx = "stickyhand";
    auto shot = make_shared<Mesh>(
        make_shared<MeshGeometry>(Prefab::quad(glm::vec2(8.0f, 2.0f))),
        vector<shared_ptr<IMeshModifier>>{
            make_shared<Wrap>(Prefab::quad_wrap(
                glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 0.0f)
            ))
        },
        make_shared<MeshMaterial>(gfx + ".png", m_pResources)
    );
    shot->config()->set<Player*>("player",this);

    shot->material()->emissive(Color::white());
    root()->add(shot);

    if(m_Prone)
        shot->position(glm::vec3(
            position().x +
            -m_pProne->origin().x * m_pProne->size().x +
                m_pProne->mesh()->world_box().size().x / 2.0f,
            position().y +
                -m_pProne->origin().y * m_pProne->size().y +
                m_pProne->mesh()->world_box().size().y / 2.0f + -2.0f,
            position().z
        ));
    else
        shot->position(glm::vec3(
            position().x +
            -m_pChar->origin().x * m_pChar->size().x +
                m_pChar->mesh()->world_box().size().x / 2.0f,
            position().y +
                -m_pChar->origin().y * m_pChar->size().y +
                m_pChar->mesh()->world_box().size().y / 2.0f + -2.0f,
            position().z
        ));

    vec3 aimdir = vec3(dir, 0.0f);

    //if(not check_state("up") && not check_state("down"))
    //    aimdir += check_state("left") ? vec3(-1.0f, 0.0f, 0.0f) : vec3(1.0f, 0.0f, 0.0f);

    //if(check_state("up") || check_state("upward"))
    //    aimdir += vec3(0.0f, -1.0f, 0.0f);

    //if(check_state("down") || check_state("downward"))
    //    aimdir += vec3(0.0f, 1.0f, 0.0f);
    //aimdir = normalize(aimdir);
    auto ang = atan2(aimdir.y, aimdir.x) / K_TAU;
   /*
    if (m_Battery == 1) {
        auto rAng = kit::random<float>();
        //ang += (rAng - 0.5f) * 10.0f;
    }
    if (m_Battery == 2) {
        auto rAng = kit::random<float>();
        //ang += (rAng - 0.5f) * 10.0f;
    }
    if (m_Battery == 3) {
        auto rAng = kit::random<float>();
        ang += rAng - 0.5f;
    }
    if (m_Battery > 3) {
        auto rAng = kit::random<float>();
        ang += rAng - 0.5f;
    }
    aimdir = vec3(cos(ang), sin(ang), 0.0f);
    */
    shot->rotate(ang, glm::vec3(0.0f, 0.0f, 1.0f));
    shot->velocity(aimdir * 256.0f);

    shot->when_with(Freq::Time::seconds(0.5f), m_pTimeline, [](Node* shot){
        shot->detach();
    });
    
    m_pPartitioner->register_object(shot, Game::BULLET);
    
    Sound::play(m_pCamera, "shoot.wav", m_pResources);

    //upgrade 1: speed upgrade battery
    m_ShootTimer.set(Freq::Time::ms(m_Battery == 0 ? 200 : 100));
    
    // increase box Z width
    auto shotbox = shot->box();

    shot->set_box(Box(
        vec3(shotbox.min().x, shotbox.min().y, -5.0),
        vec3(shotbox.max().x, shotbox.max().y, 5.0)
    ));
}

void Player :: cb_to_bullet(Node* player_node, Node* bullet) {
    auto player = player_node->config()->at<Player*>("player", nullptr);

    if (not player)
        return;

    // bullet owner is a player, ignore
    if (bullet->config()->has("player"))
        return;
    
    // 29 July 2016 - KG: Added God Mode
    if (player->god() || player->no_enemy_damage())
        return;

    //player->reset();
    if (bullet->config()->has("damage")) {
        int damage = bullet->config()->at<int>("damage", 1);
        if(player->hurt(damage))
            bullet->safe_detach();
        auto gibs = player->m_Health==0 ? 20 : 5;
        for (int i = 0; i < gibs; ++i)
            player->gib();
    }

}

void Player :: blink(){
    blinking(true);
    m_BlinkTime.set(Freq::Time::ms(BLINK_TIME));
    m_InvincibleTime.set(Freq::Time::ms(INVINCIBLE_TIME));
}

void Player :: reset() {
    if (m_Lives <= 0) {
        m_pGame->end();
        return;
    }
    m_pGame->reset();
    m_Health = 100;
    --m_Lives;
    on_health_change(m_Health);
}

void Player :: reset_walljump() {
    m_LastWallJumpDir = 0;
}

void Player :: prone(bool b) {
    if(m_Prone == b)
        return;

    if(b && std::abs(velocity().x) > K_EPSILON)
        return;
    
    m_Prone = b;
    
    if(m_pChar->check_state("left"))
        m_pProne->set_state("left");
    else
        m_pProne->set_state("right");
    
    m_pProne->visible(b);
    m_pChar->visible(not b);
}
bool Player :: hurt(int damage) {
    //check for vulnerability
    if(god() || blinking()) //if god we do not want to accept damage
        return false;
    if(m_Health < 1)
        return false; // if dead, don't accept damage
    m_Health = m_Health - damage;
    on_health_change(m_Health);
    if (m_Health < 1)
        reset();

    blink();

    return true; // player accepts damage
}
void Player::heal(int health) {
    m_Health = std::min(m_Health + health, 100);
    on_health_change(m_Health);
}

void Player::gib() {
    auto gib = make_shared<Sprite>(m_pResources->transform("blood.json"), m_pResources);
    gib->set_state("animated");

    // Randomizes direction gib moves
    auto dir = Angle::degrees(1.0f * (rand() % 360)).vector();
    stick(gib);

    // Sets gib size and movement
    gib->move(vec3(rand() % 100 - 50.0f, rand() % 200 - 100.0f, 2.0f));
    gib->velocity(vec3(dir, 0.0f) * 100.0f);
    gib->acceleration(vec3(0.0f, 500.0f, 0.0f));
    gib->scale((rand() % 100 + 1) / 100.0f * 0.5f);

    // Creates random gib lifetime
    auto lifetime = make_shared<float>(0.5f * (rand() % 4));
    auto gibptr = gib.get();

    // Connexts gib to game tick signal
    gib->on_tick.connect([gibptr, lifetime](Freq::Time t) {
        *lifetime -= t.s();

        if (*lifetime < 0.0f)
            gibptr->detach();
    });
}

void Player :: next_level() {
    auto mapname = m_pGame->qor()->args().value_or("map", "1");
    auto nextmap = to_string(boost::lexical_cast<int>(mapname) + 1);

    m_pGame->qor()->args().set("map", nextmap);
    m_pGame->qor()->change_state("pregame");
}

void Player :: previous_level() {
    auto mapname = m_pGame->qor()->args().value_or("map", "1");
    auto nextmap = to_string(boost::lexical_cast<int>(mapname) - 1);

    m_pGame->qor()->args().set("map", nextmap);
    m_pGame->qor()->change_state("pregame");
}
