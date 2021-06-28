#pragma once
#include <core/gt.h>
#include <hooks/ProcessTankUpdatePacket.h>
#include <hooks/SendPacket.h>
#include <hooks/SendPacketRaw.h>
#include <intrin.h>
#include <sdk/GameUpdatePacket.h>
#include <sdk/sdk.h>
#include <algorithm>

std::string gt::generate_mac(const std::string& prefix) {
    std::string x = prefix + ":";
    for (int i = 0; i < 5; i++) {
        x += utils::hex_str(utils::random(0, 255));
        if (i != 4)
            x += ":";
    }
    return x;
}

std::string gt::generate_rid() {
    std::string rid_str;

    for (int i = 0; i < 16; i++)
        rid_str += utils::hex_str(utils::random(0, 255));

    std::transform(rid_str.begin(), rid_str.end(), rid_str.begin(), std::toupper);

    return rid_str;
}

std::string gt::generate_meta() {
    return utils::rnd(utils::random(5, 10)) + ".com";
}

std::string gt::get_random_flag() {
    static bool done = false;
    static std::vector<string> candidates{};

    if (!done) {
        CHAR NPath[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, NPath);
        std::string pattern(string(NPath) + "\\interface\\flags\\*.rttex");
        _WIN32_FIND_DATAA data{};
        HANDLE hFind;
        if ((hFind = FindFirstFileA(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE) {
            do {
                auto temp = string(data.cFileName);
                if (utils::replace(temp, ".rttex", ""))
                    if (temp.length() == 2)
                        candidates.push_back(temp);

            } while (FindNextFileA(hFind, &data) != 0);
            FindClose(hFind);
        }
        done = true;
    }

    return candidates[utils::random(0, (int)candidates.size())];
}

std::string gt::get_type_string(uint8_t type) {
    static const char* types[]{ "PACKET_STATE", "PACKET_CALL_FUNCTION", "PACKET_UPDATE_STATUS", "PACKET_TILE_CHANGE_REQUEST", "PACKET_SEND_MAP_DATA",
        "PACKET_SEND_TILE_UPDATE_DATA", "PACKET_SEND_TILE_UPDATE_DATA_MULTIPLE", "PACKET_TILE_ACTIVATE_REQUEST", "PACKET_TILE_APPLY_DAMAGE",
        "PACKET_SEND_INVENTORY_STATE", "PACKET_ITEM_ACTIVATE_REQUEST", "PACKET_ITEM_ACTIVATE_OBJECT_REQUEST", "PACKET_SEND_TILE_TREE_STATE",
        "PACKET_MODIFY_ITEM_INVENTORY", "PACKET_ITEM_CHANGE_OBJECT", "PACKET_SEND_LOCK", "PACKET_SEND_ITEM_DATABASE_DATA", "PACKET_SEND_PARTICLE_EFFECT",
        "PACKET_SET_ICON_STATE", "PACKET_ITEM_EFFECT", "PACKET_SET_CHARACTER_STATE", "PACKET_PING_REPLY", "PACKET_PING_REQUEST", "PACKET_GOT_PUNCHED",
        "PACKET_APP_CHECK_RESPONSE", "PACKET_APP_INTEGRITY_FAIL", "PACKET_DISCONNECT", "PACKET_BATTLE_JOIN", "PACKET_BATTLE_EVENT", "PACKET_USE_DOOR",
        "PACKET_SEND_PARENTAL", "PACKET_GONE_FISHIN", "PACKET_STEAM", "PACKET_PET_BATTLE", "PACKET_NPC", "PACKET_SPECIAL", "PACKET_SEND_PARTICLE_EFFECT_V2",
        "PACKET_ACTIVE_ARROW_TO_ITEM", "PACKET_SELECT_TILE_INDEX", "PACKET_SEND_PLAYER_TRIBUTE_DATA", "PACKET_PVE_UNK1", "PACKET_PVE_UNK2", "PACKET_PVE_UNK3",
        "PACKET_PVE_UNK4", "PACKET_PVE_UNK5", "PACKET_SET_EXTRA_MODS", "PACKET_ON_STEP_ON_TILE_MOD", "PACKET_ERRORTYPE" };

    constexpr auto size = sizeof(types) / sizeof(const char*) - 1;
    if (type >= PACKET_MAXVAL || type >= size)
        type = PACKET_MAXVAL - 1; //will set any unknown type as errortype and keep us from crashing

    return types[type];
}
#define WORDn(x, n) (*((WORD*)&(x) + n))
int16_t gt::get_cpuid() {
    int32_t regs[4];
    __cpuid((int*)regs, 0);

    return WORDn(regs[0], 1) + WORDn(regs[1], 1) + WORDn(regs[2], 1) + WORDn(regs[3], 1) + regs[0] + regs[1] + regs[2] + regs[3];
}

int gt::decrypt_piece(uint8_t* data, uint32_t size, int seed) {
    auto seed_mod = seed;
    char unk = -2 - seed;

    int smth = 0;
    int smth2 = 0;
    do {
        int temp1 = smth;
        int temp2 = *data;
        smth2 = temp1 + temp2;
        *data = unk + temp2;
        --unk;
        ++data;
        smth = seed_mod + smth2;
        ++seed_mod;
        --size;
    } while (size);

    return seed_mod + smth2 - 1;
}

//TLDR gt stores badly encrypted cached versions of hash, wk, mac into registry and they take priority over real one
//this is why unban used to require u to delete the keys too.
//stealers getting the mac from here would be ideal to use for save decryption, since real mac used by gt cant be known
//and since gt uses mac to encrypt pass (see my save decrypter for decrypter without bruteforce)
void gt::decrypt_reg_vals() {
    //did this while bored and it was too easy
    uint32_t uint3 = (uint16_t)gt::get_cpuid() + 1;
    LPCSTR key = ("Software\\Microsoft\\" + std::to_string(uint3)).c_str();
    BYTE data[1024];
    DWORD data_len = 1024;
    memset(data, 0, 1024);
    if (!utils::read_reg_value(key, (std::to_string((uint3 >> 1))).c_str(), data, &data_len)) {
        printf("failed at reading reg value %s!\n", key);
        return;
    }

    auto hashc = gt::decrypt_piece(data, data_len - 1, 25532);
    auto hash = utils::format("%s", data);
    auto hash_int = atoi((char*)data);
    memset(data, 0, 1024);
    if (!utils::read_reg_value(key, (std::to_string((uint3 >> 1)) + "c").c_str(), data, &data_len)) {
        printf("failed at reading reg value %s!\n", key);
        return;
    }
    auto hashc2 = atol((const char*)data);
    if (!utils::read_reg_value(key, (std::to_string((uint3 >> 1)) + "w").c_str(), data, &data_len)) {
        printf("failed at reading reg value %s!\n", key);
        return;
    }
    auto wkc = gt::decrypt_piece(data, data_len - 1, 25532);
    auto wk = utils::format("%s", data);
    memset(data, 0, 1024);
    if (!utils::read_reg_value(key, (std::to_string((uint3 >> 1)) + "wc").c_str(), data, &data_len)) {
        printf("failed at reading reg value %s!\n", key);
        return;
    }
    auto wkc2 = atol((const char*)data);
    memset(data, 0, 1024);
    auto keymac = std::to_string(abs(hash_int / 3));
    auto valmac = std::to_string(abs(hash_int / 4));

    if (!utils::read_reg_value(keymac.c_str(), valmac.c_str(), data, &data_len)) {
        printf("failed at reading reg value %s\n", keymac.c_str());
        return;
    }

    auto macc = gt::decrypt_piece(data, data_len - 1, 25532);
    auto mac = utils::format("%s", data);

    printf("hash: %s\nwk: %s\nmac: %s\n", hash.c_str(), wk.c_str(), mac.c_str());
    //got bored at this point and couldnt be bothered to read the c for mac too
}

void gt::set_extra_character_mods(NetAvatar* player, uint8_t flags) {
    static auto func = types::SetCharacterExtraMods(sigs::get(sig::setextracharactermods));

    func(player, &flags);
}

void gt::send(int type, std::string message, bool hook_send) {
    static auto func = types::SendPacket(sigs::get(sig::sendpacket));
    if (hook_send) //if we want to apply our own code or just log shit
        SendPacketHook::Execute(type, message, sdk::GetPeer());
    else
        func(type, message, sdk::GetPeer());
}

void gt::send(GameUpdatePacket* packet, int extra_size, bool hook_send) {
    static auto func = types::SendPacketRaw(sigs::get(sig::sendpacketraw));
    void* PacketSender = nullptr;
    if (hook_send) //if we want to apply our own code or just log shit
        SendPacketRawHook::Execute(NET_MESSAGE_GAME_PACKET, packet, 56 + extra_size, PacketSender, sdk::GetPeer(), ENET_PACKET_FLAG_RELIABLE);
    else
        func(NET_MESSAGE_GAME_PACKET, packet, 56 + extra_size, PacketSender, sdk::GetPeer(), ENET_PACKET_FLAG_RELIABLE);
}

void gt::send_self(GameUpdatePacket* packet, bool hook_send) {
    static auto func = types::ProcessTankUpdatePacket(sigs::get(sig::processtankupdatepacket));
    if (hook_send) //if we want to apply our own code or just log shit
        ProcessTankUpdatePacketHook::Execute(sdk::GetGameLogic(), packet);
    else
        func(sdk::GetGameLogic(), packet);
}

void gt::send_varlist_self(variantlist_t variantlist, int netid, int delay, bool hook_send) {
    uint32_t data_size = 0;
    void* data = variantlist.serialize_to_mem(&data_size, nullptr);

    auto packet = (GameUpdatePacket*)calloc(sizeof(GameUpdatePacket) + data_size, 1);
    packet->type = PACKET_CALL_FUNCTION;
    packet->netid = netid;
    packet->flags |= 8;
    packet->int_data = delay;
    packet->data_size = data_size;
    memcpy(&packet->data, data, data_size);

    static auto func = types::ProcessTankUpdatePacket(sigs::get(sig::processtankupdatepacket));
    if (hook_send) //if we want to apply our own code or just log shit
        ProcessTankUpdatePacketHook::Execute(sdk::GetGameLogic(), packet);
    else
        func(sdk::GetGameLogic(), packet);
    free(packet);
    free(data);
}

bool gt::patch_banbypass() {
    try {
        static auto banbypass = sigs::get(sig::banbypass);
        if (!banbypass) //did not find ban bypass
            throw std::runtime_error("could not find ban bypass");

        auto bypassed = utils::patch_bytes<2>(banbypass, "\x90\x90");
        if (!bypassed)
            throw std::runtime_error("could not patch ban bypass");
        else {
            printf("patched ban bypass\n");
            return true;
        }
    } catch (std::exception exception) {
        printf("exception thrown: %s\n", exception.what());
        utils::read_key();
        return false;
    }
    return false;
}
//#include <memory>
//#include <winternl.h>
//
//#pragma comment(lib, "ntdll")
//
//#define NT_SUCCESS(status) (status >= 0)
//
//#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)
//
//
//typedef struct _PROCESS_HANDLE_TABLE_ENTRY_INFO {
//    HANDLE HandleValue;
//    ULONG_PTR HandleCount;
//    ULONG_PTR PointerCount;
//    ULONG GrantedAccess;
//    ULONG ObjectTypeIndex;
//    ULONG HandleAttributes;
//    ULONG Reserved;
//} PROCESS_HANDLE_TABLE_ENTRY_INFO, *PPROCESS_HANDLE_TABLE_ENTRY_INFO;
//
//// private
//typedef struct _PROCESS_HANDLE_SNAPSHOT_INFORMATION {
//    ULONG_PTR NumberOfHandles;
//    ULONG_PTR Reserved;
//    PROCESS_HANDLE_TABLE_ENTRY_INFO Handles[1];
//} PROCESS_HANDLE_SNAPSHOT_INFORMATION, *PPROCESS_HANDLE_SNAPSHOT_INFORMATION;
//
//extern "C" NTSTATUS NTAPI NtQueryInformationProcess(_In_ HANDLE ProcessHandle, _In_ PROCESSINFOCLASS ProcessInformationClass,
//    _Out_writes_bytes_(ProcessInformationLength) PVOID ProcessInformation, _In_ ULONG ProcessInformationLength, _Out_opt_ PULONG ReturnLength);

bool gt::patch_mutex() {
    static auto mutex1 = sigs::get(sig::mutexbypass1);
    static auto mutex2 = sigs::get(sig::mutexbypass2);

    if (!mutex2 || !mutex1) {
        printf(
            "If you're using a modified GT executable that supports multiboxing (not meaning patcher that comes with INZERNAL)\n"
            "Then just ignore this warning, although some things might be broken if the file size is different from original, otherwise should be fine.\n"
            "If that's not the case, you are probably using wrong gt version, or something went horribly wrong.\n");
        return true;
    }

    mutex2 -= 9; //Didnt bother fixing negative offsets in sig manager cuz I'm lazy, not a big deal (for now) at least

    //this checks for presence of gt's mutex 247, and will freeze if you delete them
    auto patched1 = utils::patch_bytes<6>(mutex1, "\x48\x83\xc4\x50\x5b\xc3"); //basic frame stuff and ret

    if (!patched1)
        return false;

    //the length will probably change with each GT version so I have to remember to change this
    //(no way im calculating the offset by adding another sig for where to jump etcetc) well prob will actually if I get bored
    utils::patch_bytes<2>(mutex2, "\xEB\x53");

    //fow now mutex bypass only works with patcher, since I was too lazy to add proper handle closing thats required for injector
    //will add it prob by V0.6

    printf("Patched mutex checks\n");
    return true;
}

void gt::hit_tile(CL_Vec2i where) {
    auto local = sdk::GetGameLogic()->GetLocalPlayer();
    if (!local)
        return;

    GameUpdatePacket packet{ 0 };
    packet.type = PACKET_TILE_CHANGE_REQUEST;
    packet.item_id = 18;
    packet.int_x = where.x;
    packet.int_y = where.y;
    auto pos = local->GetPos();
    packet.vec_x = pos.x;
    packet.vec_y = pos.y;
    gt::send(&packet);
}

void gt::place_tile(int id, CL_Vec2i where) {
    auto local = sdk::GetGameLogic()->GetLocalPlayer();
    if (!local)
        return;

    GameUpdatePacket packet{ 0 };
    packet.type = PACKET_TILE_CHANGE_REQUEST;
    packet.item_id = id;
    packet.int_x = where.x;
    packet.int_y = where.y;
    auto pos = local->GetPos();
    packet.vec_x = pos.x;
    packet.vec_y = pos.y;
    gt::send(&packet);
}

void gt::wrench_tile(CL_Vec2i where) {
     auto local = sdk::GetGameLogic()->GetLocalPlayer();
    if (!local)
        return;

    GameUpdatePacket packet{ 0 };
    packet.type = PACKET_TILE_CHANGE_REQUEST;
    packet.item = 32;
    packet.int_x = where.x;
    packet.int_y = where.y;
    auto pos = local->GetPos();
    packet.pos_x = pos.x;
    packet.pos_y = pos.y;
    gt::send(&packet);
}
void gt::water_tile(CL_Vec2i where) {
    auto local = sdk::GetGameLogic()->GetLocalPlayer();
    if (!local)
        return;

    GameUpdatePacket packet{ 0 };
    packet.type = PACKET_STATE;
    packet.item = 822;
    packet.int_x = where.x;
    packet.int_y = where.y;
    packet.flags = (1 << 5) | (1 << 10) | (1 << 11); //no enum rn so using raw values

    auto pos = local->GetPos();
    packet.pos_x = pos.x;
    packet.pos_y = pos.y;
    gt::send(&packet);
}

void gt::solve_captcha(std::string text) {
    utils::replace(text,
        "set_default_color|`o\nadd_label_with_icon|big|`wAre you Human?``|left|206|\nadd_spacer|small|\nadd_textbox|What will be the sum of the following "
        "numbers|left|\nadd_textbox|",
        "");
    utils::replace(text, "|left|\nadd_text_input|captcha_answer|Answer:||32|\nend_dialog|captcha_submit||Submit|", "");
    auto number1 = text.substr(0, text.find(" +"));
    auto number2 = text.substr(number1.length() + 3, text.length());
    int result = atoi(number1.c_str()) + atoi(number2.c_str());
    gt::log("Solved captcha as `2" + std::to_string(result) + "``");
    gt::send(2, "action|dialog_return\ndialog_name|captcha_submit\ncaptcha_answer|" + std::to_string(result), sdk::GetPeer());
}

void gt::log(std::string text) {
    variantlist_t var{ "OnConsoleMessage" };
    var[1] = "`0[`#INZERNAL`0] " + text;
    gt::send_varlist_self(var, -1, 0, true);
}
