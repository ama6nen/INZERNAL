#pragma once
#include <core/utils.h>
#include <proton/clanlib/vec2.h>
#include <string>

namespace gt {
    //i mean tell me a simpler way to do this
    std::string generate_mac(const std::string& prefix = "02");
    std::string generate_rid();
    std::string generate_meta();
    std::string get_random_flag();
    std::string get_type_string(uint8_t type);
    int16_t get_cpuid();
    int decrypt_piece(uint8_t* data, uint32_t size, int seed);
    void decrypt_reg_vals();

    void set_extra_character_mods(NetAvatar* player, uint8_t flags);

    void send(int type, std::string message, bool hook_send = false);
    void send(GameUpdatePacket* packet, int extra_size = 0, bool hook_send = false);
    void send_self(GameUpdatePacket* packet, bool hook_send = true);
    void send_varlist_self(variantlist_t variantlist, int netid = -1, int delay = 0, bool hook_send = false);

    void ghetto_fix();
    bool patch_banbypass();
    bool patch_mutex();
    void join_world(std::string world);
    void show_message(std::string message, int time, int delay);
    void log(std::string msg);
    void solve_captcha(std::string text);

    //void screen_size(CL_Vec2f& rect);
    void hit_tile(CL_Vec2i where);
    void place_tile(int id, CL_Vec2i where);
    void wrench_tile(CL_Vec2i where);
    void water_tile(CL_Vec2i where);
    //void tutorial_bypass();
    //void calc_checksum(iteminfo_t& item);
} // namespace gt
