#pragma once
#include <menu/menu.h>
#include <sdk/Consts.h>
#include <sdk/ResourceManager.h>
#include <sdk/sdk.h>

void menu::cheats_tab() {
    auto local = sdk::GetGameLogic()->GetLocalPlayer();



    //commented for now - added too many things and things are pretty messy ATM
    //TODO: move to enhancements or smth
  
    //commented for now - added too many things and things are pretty messy ATM
    /*   static SurfaceAnim* surfaceanim = nullptr;
    if (!surfaceanim && global::app) {
        ResourceManager* mgr = (ResourceManager*)((uintptr_t)global::app + 2408);
        if (mgr)
            surfaceanim = mgr->GetSurfaceAnim("game/tiles_page4.rttex");
    }*/


   
    //commented for now - added too many things and things are pretty messy ATM
    //tldr texture drawing 
    //if (surfaceanim) {
    //    //shoutout to atipls for helping me find s_renderD3D9 offset initially,
    //    //and helping me with this in general, couldnt have done it without him
    //    constexpr static auto m_textures_offset = 0x67800;    //bgfx internal, very very small chance of changing
    //    static auto s_renderD3D9 = sigs::get(sig::s_renderd3d9);
    //    static auto m_textures = (TextureD3D9*)(s_renderD3D9 + m_textures_offset);

    //    static auto& texture = m_textures[surfaceanim->texture_id];

    //    int largest = 0;
    //    for (auto i = 0; m_textures[i].m_ptr; ++i)
    //        largest = i;

    //    const auto size = ImVec2(64.f, 64.f);
    //    //proof of concept image drawing
    //    ImGui::Image(ImTextureID(texture.m_ptr), size, ImVec2((size.x + size.x) / texture.m_width, 1 - (size.x / texture.m_width)),
    //        ImVec2(1 - ((size.y + size.y) / texture.m_height), size.y / texture.m_height));
    //}

    bool dash, charge, cancel;

    imwrap::prep_columns(6);
    imwrap::checkbox("TP on click", opt::cheat::tp_click, "Teleports to cursor position when you press ctrl + left click");
    dash = imwrap::checkbox("Dashing", opt::cheat::dash, "Allows you to double tap to dash in any direction");

    ImGui::NextColumn();
    imwrap::checkbox("Block SPR", opt::cheat::block_sendpacketraw, "Blocks Sendpacketraw. Basically full-on ghost, but a bit more crude than actual ghost");
    charge = imwrap::checkbox("Jump charge", opt::cheat::jump_charge, "Switches your jumping mode to being charging, holding means higher jump");

    ImGui::NextColumn();
    imwrap::checkbox("Mod zoom", opt::cheat::mod_zoom, "Allows you to zoom as far out as you want to, like mods");
    cancel = imwrap::checkbox("Jump cancel", opt::cheat::jump_cancel, "You can tap W in air to instantly change your ascent to starting descent");

    ImGui::NextColumn();
    imwrap::checkbox("Dev zoom", opt::cheat::dev_zoom, "Same as mod zoom but allows you to place and build far away too.\nWhich can ban btw.");
    imwrap::checkbox("Free dialogs", opt::cheat::dialog_cheat,
        "When enabled, you can move, punch and so on while a dialog is opened.\nIf there is text in the dialog, then you cant move, but can still punch, etc.");

    ImGui::NextColumn();
    imwrap::checkbox("Antighost", opt::cheat::antighost,
        "Ignores ghost slimed effect.\nAlso now completely ignores any packets relating to these annoying fuckers.\nWell, that makes see ghosts useless with this, I "
        "guess?");
    imwrap::checkbox("Antipunch", opt::cheat::antipunch, "Simple: Same as having a punch jammer in worlds that don't have one.");

    ImGui::NextColumn();
    imwrap::checkbox("See ghosts", opt::cheat::see_ghosts, "Allows you to see ghosts as the name says\nDoesn't work with antighost.");
    imwrap::checkbox("See fruits", opt::cheat::see_fruits, "See how many fruits a tree will have before it has grown fully.");
    imwrap::checkbox("Local building", sdk::GetGameLogic()->local_building,
        "Allows you to build locally.\nYou will not send packets when placing things, and can place consumables, etc");

    ImGui::Columns(1, nullptr, false);
    ImGui::PopStyleVar();

    if (local && (dash || charge || cancel))
        local->SetCharacterMods(opt::cheat::dash, opt::cheat::jump_charge, opt::cheat::jump_cancel);

    imwrap::prep_columns(2);
    ImGui::Spacing();
    static int ix = C_PUNCH_RELOAD_TIME;
    if (ImGui::BeginChild("###cooldownchanger", AUTOSIZEC(2), true, ImGuiWindowFlags_MenuBar)) {
        ImGui::BeginMenuBar();
        ImGui::Text("Punch speed changer");
        ImGui::EndMenuBar();
        if (ImGui::Checkbox("Enabled###punch", &opt::cheat::punch_cooldown_on)) {
            if (!opt::cheat::punch_cooldown_on)
                consts::set_float(ix, consts::defs[ix]);
            else
                consts::set_float(ix, opt::cheat::punch_cooldown_val);
        }
        ImGui::SameLine();
        if (ImGui::SliderFloat("###punchc", &opt::cheat::punch_cooldown_val, 0.0f, 0.4f, "%0.2f") && opt::cheat::punch_cooldown_on)
            consts::set_float(ix, opt::cheat::punch_cooldown_val);

        ImGui::EndChild();
        ImGui::Spacing();
    }
    if (imwrap::fchanger("Gravity changer", opt::cheat::gravity_val, opt::cheat::gravity_on, -500.f, 2000.f) && local)
        local->gravity.set(global::state.gravity);

    static bool isConstOn = false;

    if (ImGui::BeginChild("###Constants", isConstOn ? AUTOSIZEC(10) : AUTOSIZEC(2), true, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar)) {
        ImGui::BeginMenuBar();
        ImGui::Text("Game constants");
        ImGui::EndMenuBar();

        if (!isConstOn) {
            if (ImGui::Button("Expand"))
                isConstOn = true;
            ImGui::SameLine();
            ImGui::Text("Contains 21 different protected constants");
        }
        else {
            if (ImGui::Button("Close"))
                isConstOn = false;
            ImGui::SameLine();
            int i = 0;
            if (ImGui::Button("Reset all")) {
                consts::reset_all();

                if (opt::cheat::punch_cooldown_on) //dont reset this if we have the other option on for this
                    consts::set_float(C_PUNCH_RELOAD_TIME, opt::cheat::punch_cooldown_val);
            }
            ImGui::Spacing();
            ImGui::Spacing();

            imwrap::const_slider("Max fall speed", 0.f, 2000.f, "%.0f", i++, "Pretty simple");
            imwrap::const_slider("Float power", 0.f, 10000.f, "%.0f", i++, "Higher makes damage flying curve smaller");
            imwrap::const_slider("Wind speed", 0.f, 2000.0f, "%.0f", i++, "The speed winter wind and others push you");
            imwrap::const_slider("Wind speed (against)", 0.f, 200.0f, "%.0f", i++, "The resistance of going against winter wind");
            imwrap::const_slider("Coll. max divergence", 0.f, 128.0f, "%.0f", i++, "No clue what this does, no references, no effect");
            imwrap::const_slider("Player drag", -500.f, 2400.0f, "%.0f", i++, "Drag for walking. Low values allow 'moonwalk'.");
            imwrap::const_slider("Initial jump boost", -100.f, -1000.0f, "%.0f", i++, "Affects how high you can jump in a different way from gravity");
            imwrap::const_slider(
                "Control hurt mod", 0.0f, 1.5f, "%0.2f", i++, "Affects how well you are able to control your char when hurt. 1 = normal, 0 = no control.");
            imwrap::const_slider("Lava bounce [x]", -100.f, 1000.0f, "%.0f", i++, "Just how much you bounce from lava/etc horizontally");
            imwrap::const_slider("Lava bounce [y]", -100.f, 1400.0f, "%.0f", i++, "Just how much you bounce from lava/etc vertically");
            imwrap::const_slider("Trampoline bounce", -100.f, 1600.0f, "%.0f", i++, "Bounciness for mushroom/etc");
            imwrap::const_slider("Regular bounce", -100.f, 1600.0f, "%.0f", i++, "Bounciness for pinballs/etc");
            imwrap::const_slider("Def. punch knockback", -500.f, 500.0f, "%.0f", i++, "Does not seem to have much effect except for anims (clientside)");
            imwrap::const_slider("Parasol gravity mod", 0.f, 1.0f, "%0.2f", i++, "How much parasol and similar affect gravity");
            imwrap::const_slider("Water gravity mod", 0.f, 2.0f, "%0.2f", i++, "How much water affects gravity");
            imwrap::const_slider("Mars gravity mod", 0.f, 2.0f, "%0.2f", i++, "How much mars affects gravity");
            imwrap::const_slider("Engine speedhack", 0.f, 4000.0f, "%.0f", i++, "Game engine speedhack. Affects various things, some need re-enter to apply fully.");
            imwrap::const_slider("Ghost speed", 0.f, 1000.0f, "%.0f", i++, "When you have the /ghost playmod, not related to ghosts that slime you.");
            imwrap::const_slider(
                "Water lev. time", 0.f, 6000.0f, "%.0f", i++, "Probably the time that the clothes that keep you floating on water have any effect. Didn't test.");
            imwrap::const_slider("Punch cooldown", 0.f, 0.4f, "%0.2f", i++, "Changes punch cooldown, fast punch when small value");
            imwrap::const_slider("Hack report delay (ms)", 100.f, 30000.0f, "%.0f", i++, "For example cheat engine open report, etc. Shouldn't have much of any use");
        }
    }
    ImGui::EndChild();

    ImGui::NextColumn();
    ImGui::Spacing();
    if (imwrap::fchanger("Acceleration changer", opt::cheat::accel_val, opt::cheat::accel_on, -500.f, 10000.f) && local)
        local->accel.set(global::state.accel);
    if (imwrap::fchanger("Speed changer", opt::cheat::speed_val, opt::cheat::speed_on, 100.f, 1400.f) && local)
        local->speed.set(global::state.speed);
    if (imwrap::fchanger("Water Speed changer", opt::cheat::waterspeed_val, opt::cheat::waterspeed_on, 0.f, 1400.f) && local)
        local->water_speed = global::state.water_speed;

    ImGui::Columns(1, nullptr, false);
    
    if (ImGui::BeginChild("###Warp", AUTOSIZEC(2), true, ImGuiWindowFlags_MenuBar)) {
        ImGui::BeginMenuBar();
        ImGui::Text("Warp");
        ImGui::EndMenuBar();
        static char worldname[25];
        size_t size = std::strlen(worldname);
        ImGui::Text("World Name:");
        ImGui::SameLine();
        ImGui::InputText("", worldname, IM_ARRAYSIZE(worldname));
        ImGui::SameLine();
        if (ImGui::Button("Warp to World")) {
            gt::join_world(worldname);
        }
        ImGui::SameLine();
        ImGui::Text(" %u / 24", size);
        ImGui::EndChild();
        ImGui::Spacing();
    }
    
    ImGui::PopStyleVar();
    //dont worry bringing back instant stop and start later on without the hooks.
}
