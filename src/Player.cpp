#include "Player.h"
#include "Qor/Sound.h"
using namespace std;
using namespace glm;
#include "Game.h"
#include "Monster.h"

Player :: Player(
    std::string fn,
    Cache<Resource, std::string>* resources,
    Camera* camera,
    Controller* ctrl,
    Freq::Timeline* timeline,
    IPartitioner* part,
    Game* game
):
    Sprite(fn, resources),
    m_pTimeline(timeline),
    m_pResources(resources),
    m_JumpTimer(timeline),
    m_ShootTimer(timeline),
    m_pCamera(camera),
    m_pController(ctrl),
    m_pPartitioner(part),
    m_pGame(game)
{
    set_states({"stand", "right", "forward"});
    //m_pCamera->position(glm::vec3(-64.0f, -64.0f, 0.0f));
    position(glm::vec3(0.0f, 0.0f, 1.0f));

    m_pCharFocusRight = make_shared<Node>();
    m_pCharFocusRight->position(glm::vec3(32.0f, 0.0f, 0.0f));
    m_pCharFocusLeft = make_shared<Node>();
    m_pCharFocusLeft->position(glm::vec3(-32.0f, 0.0f, 0.0f));
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
    Sprite::logic_self(t);

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
        set_state("walljump");
    else if (in_air)
        set_state("jump");
    
    glm::vec3 move(0.0f);

    if (velocity().x > -K_EPSILON && velocity().x < K_EPSILON) {
        if (m_pController->button("left")) {
            m_pCamera->track(focus_left());
            move += glm::vec3(-1.0f, 0.0f, 0.0f);
        }
        if (m_pController->button("right")) {
            m_pCamera->track(focus_right());
            move += glm::vec3(1.0f, 0.0f, 0.0f);
        }
    }

    if (m_pController->button("shoot") && m_ShootTimer.elapsed())
        shoot();
        
    bool block_jump = false;
    if (m_pController->button("jump")) {
        
        if (walljump || not in_air || not m_JumpTimer.elapsed()) {
            float x = 0.0f;

            if (walljump) {
                auto last_dir = m_LastWallJumpDir;
                auto tile_loc = wall_colliders.at(0)->world_box().center().x;

                if (tile_loc < position(Space::WORLD).x + 4.0f) {
                    m_LastWallJumpDir = -1;
                } else {
                    m_LastWallJumpDir = 1;
                }

                // prevent "climbing" the wall by checking to make sure last wall jump was a diff direction (or floor)
                // 0 means floors, -1 and 1 are directions
                if (m_LastWallJumpDir != 0 && last_dir != 0 && m_LastWallJumpDir == last_dir)
                    block_jump = true;
            }
        
            // jumping from floor or from a different wall
            if (not block_jump) {
                if (not in_air || walljump || not m_JumpTimer.elapsed()) {
                    velocity(glm::vec3(x, -125.0f, 0.0f));

                    if (not in_air || walljump) {
                        auto sounds = m_pCamera->find_type<Sound>();

                        if(sounds.empty())
                            Sound::play(m_pCamera, "jump.wav", m_pResources);
                        m_JumpTimer.set(Freq::Time::ms(200));
                    }
                }
            } else {
                // jumping on same wall? only allow this if jump timer is still running
                if (not m_JumpTimer.elapsed())
                    velocity(glm::vec3(x, -125.0f, 0.0f));
            }
        }
    } else {
        m_JumpTimer.set(Freq::Time::ms(0));
    }

    if (not in_air && m_WasInAir)
        Sound::play(m_pCamera, "touch.wav", m_pResources);

    m_WasInAir = in_air;
    
    if (not in_air)
        m_LastWallJumpDir = 0;

    if (glm::length(move) > K_EPSILON) {
        if (not in_air)
            set_state("walk");

        move = glm::normalize(move);

        if (move.x < -K_EPSILON){
            set_state("left");
        }
        else if (move.x > K_EPSILON){
            set_state("right");
        }

        move *= 100.0f * t.s();
        clear_snapshots();
        snapshot();
        this->move(move);
    }
    else {
        if (not in_air)
            set_state("stand");

        clear_snapshots();
        snapshot();
    }

    if(m_pController->button("left") || m_pController->button("right")) {
        if(m_pController->button("up"))
            set_state("upward");
        else if(m_pController->button("down"))
            set_state("downward");
        else
            set_state("forward");
    }else{
        if(m_pController->button("up"))
            set_state("up");
        else if(m_pController->button("down"))
            set_state("down");
        else
            set_state("forward");
    }
}


void Player :: shoot() {
    auto shot = make_shared<Mesh>(
        make_shared<MeshGeometry>(Prefab::quad(glm::vec2(8.0f, 2.0f))),
        vector<shared_ptr<IMeshModifier>>{
            make_shared<Wrap>(Prefab::quad_wrap(
                glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 0.0f)
            ))
        },
        make_shared<MeshMaterial>("laser.png", m_pResources)
    );
    shot->config()->set<Player*>("player",this);

    shot->material()->emissive(Color::white());
    root()->add(shot);

    shot->position(glm::vec3(
        position().x +
        -origin().x*size().x +
            mesh()->world_box().size().x / 2.0f,
        position().y +
            -origin().y*size().y +
            mesh()->world_box().size().y / 2.0f + -2.0f,
        position().z
    ));

    vec3 aimdir = vec3(0.0f, 0.0f, 0.0f);
    if(not check_state("up") && not check_state("down"))
        aimdir += check_state("left") ? vec3(-1.0f, 0.0f, 0.0f) : vec3(1.0f, 0.0f, 0.0f);
    if(check_state("up") || check_state("upward"))
        aimdir += vec3(0.0f, -1.0f, 0.0f);
    if(check_state("down") || check_state("downward"))
        aimdir += vec3(0.0f, 1.0f, 0.0f);
    aimdir = normalize(aimdir);
    
    auto ang = atan2(aimdir.y, aimdir.x) / K_TAU;
    shot->rotate(ang, glm::vec3(0.0f, 0.0f, 1.0f));
    shot->velocity(aimdir * 256.0f);

    auto timer = make_shared<Freq::Alarm>(m_pTimeline);
    timer->set(Freq::Time::seconds(0.5f));

    auto shotptr = shot.get();
    shot->on_tick.connect([timer, shotptr](Freq::Time t){
        if (timer->elapsed())
            shotptr->detach();
    });
    
    m_pPartitioner->register_object(shot, Game::BULLET);
    
    Sound::play(m_pCamera, "shoot.wav", m_pResources);

    m_ShootTimer.set(Freq::Time::ms(m_Power == 0 ? 200 : 100));
    
    // increase box Z width
    auto shotbox = shot->box();

    shot->set_box(Box(
        vec3(shotbox.min().x, shotbox.min().y, -5.0),
        vec3(shotbox.max().x, shotbox.max().y, 5.0)
    ));
}

void Player :: cb_to_bullet(Node* player_node, Node* bullet)
{
    auto player = player_node->config()->at<Player*>("player", nullptr);

    if (not player)
        return;

    // bullet owner is a player, ignore
    if(bullet->config()->has("player"))
        return;

    player->reset();
}

void Player :: reset()
{
    m_pGame->reset();
}

void Player :: reset_walljump()
{
    m_LastWallJumpDir = 0;
}

