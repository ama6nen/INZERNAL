#pragma once
#include <core/globals.h>
#include <core/gt.h>
#include <proton\MiscUtils.h>
#include <sdk/sdk.h>
#include <proton/RTParam.hpp>
#include <string>

class SendPacketHook {
   public:
    static void Execute(int type, std::string& packet, ENetPeer* peer) {
        static auto orig = decltype(&hooks::SendPacket)(hookmgr->orig(sig::sendpacket));
        if (type > NET_MESSAGE_GAME_PACKET) //dont send any tracking packets, crash logs, or other shit to gt.
            return;

        if (packet.find("game_version|") != -1 && opt::spoof_login) {
            auto var = RTVar::parse(packet);
            auto mac = gt::generate_mac();
            auto hash_str = mac + "RT";
            auto hash2 = HashString(hash_str.c_str(), hash_str.length());

            /*   static bool once = false;
            if (!once) {
                var.get("game_version") != 
                    gt::send_varlist_self({ "OnAddNotification",  })
                once = true;
            }*/

            if (var.get("game_version") != opt::gt_version)
                var.set("game_version", opt::gt_version);

            var.set("mac", mac);
            var.set("wk", gt::generate_rid());
            var.set("rid", gt::generate_rid());
            var.set("hash", std::to_string(utils::random(INT_MIN, INT_MAX)));
            var.set("hash2", std::to_string(hash2));
            var.set("meta", gt::generate_meta());

            if (opt::spoof_win == WinSpoof::wsRandom) {
                var.set("fz", std::to_string(utils::random(INT_MIN, INT_MAX)));
                var.set("zf", std::to_string(utils::random(INT_MIN, INT_MAX)));
            }
            else if (opt::spoof_win == WinSpoof::wsMimic) { //win-specific file values, mismatching is logged as fake windows.
                var.set("fz", "7134376");                   //last update: 2.952
                var.set("zf", "-1331849031");
            }

            if (opt::flag_mode == FlagMode::fmRandom)
                var.set("country", gt::get_random_flag());
            else if (opt::flag_mode == FlagMode::fmCustom)
                var.set("country", opt::flag);

            //never really paid attention to this before but this indeed could be used to track too
            var.set("player_age", std::to_string(utils::random(21, 60)));

            if (opt::spoof_name)
                var.set("requestedName", utils::rnd(utils::random(4, 10)));

            packet = var.serialize();
        }
        auto warp = packet.find("/warp ");
        if (warp != -1) {
           gt::send(3, "action|join_request\nname|" + packet.substr(warp + 6));
           return; //dont send the /warp chat message
        }

        if (logging::enabled && logging::console & logging::sendpacket)
            printf("[sendpacket] type: %d\n%s\n", type, packet.c_str());

        orig(type, packet, peer);
    }
};
