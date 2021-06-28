#pragma once
#include <core/globals.h>
#include <core/gt.h>
#include <sdk/sdk.h>
#include <string>

class ProcessTankUpdatePacketHook {
   public:
    static void Execute(GameLogic* logic, GameUpdatePacket* packet) {
        static auto orig = decltype(&hooks::ProcessTankUpdatePacket)(hookmgr->orig(sig::processtankupdatepacket));

        //just let the ghosts not even exist
        if (opt::cheat::antighost && packet->type == PACKET_NPC && (packet->npctype == 1 || packet->npctype == 7 || packet->npctype == 12))
            return;

        if (logging::enabled) {
            if (logging::console & logging::processtank)
                printf("got raw packet: %d [%s]\n", packet->type, gt::get_type_string(packet->type).c_str());
        }

auto local = sdk::GetGameLogic()->GetLocalPlayer();
        switch (packet->type) {
            case PACKET_CALL_FUNCTION: {
                if (logging::enabled && logging::console & logging::callfunction) {
                    variantlist_t varlist{};
                    auto extended = utils::get_extended(packet);
                    if (varlist.serialize_from_mem(extended)) {
                        auto content = varlist.print();
                        if (content.length() < 4096)
                            printf("%s\n", content.c_str());
                    }
                }
                variantlist_t varlist{};
                auto extended = utils::get_extended(packet);
                if (varlist.serialize_from_mem(extended)) {
                    auto head = varlist.get(0);
                    if (head.get_type() == variant_t::vartype_t::TYPE_STRING && head.get_string().find("onShowCaptcha") != -1) {
                        auto captcha = varlist.get(1).get_string();
                        gt::solve_captcha(captcha);
                        return;
                    }
                    if (head.get_type() == variant_t::vartype_t::TYPE_STRING && head.get_string().find("OnDialogRequest") != -1) {
                        auto content = varlist.get(1).get_string();
                        if (content.find("set_default_color|`o") != -1) {
                            auto content12 = varlist.get(1).get_string();
                            if (content.find("end_dialog|captcha_submit||Submit|") != -1) {
                                gt::solve_captcha(content12);
                                return;
                            }
                        }
                    }
                }
            } break;
            case PACKET_SET_CHARACTER_STATE: {
                if (!local)
                    break;
                if (packet->netid == local->netid) {
                    if (opt::cheat::antighost && global::state.gravity < packet->gravity_out && global::state.speed > packet->speed_out)
                        return;

                    global::state.copy_from_packet(packet);
                }
            } break;
        }

        orig(logic, packet);
    }
};
