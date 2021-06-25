#pragma once
#include <core/globals.h>
#include <sdk/player/NetAvatar.h>

// clang-format off


//logging options
namespace logging {
		bool	enabled		= true; //master switch for all logging
	
		//logging in console
		int		console		= callfunction;

		//logging in menu (not yet implemented)
		int		max_count	= 500;  //max events in menu, for performance concerns.
		int		menu		= sendpacket | sendpacketraw | processtank | callfunction;
}

//options
namespace opt {

	//enhanchements 
	float		fps_limit	= 144.0f;	//fps limit gets forced to this
	std::string	gt_version	= "3.64";	//gt version gets spoofed to this
	std::string	gt_supported= "3.61";	//the gt version for which INZERNAL was built for. Probably wont work for other versions.

	bool		spoof_login	= true;		//whether or not to spoof mac, hash2, wk, rid, etc on login. GT version spoofed even when off.
	bool		spoof_name	= true;		//Also set the requestedName field to random. Might be unwanted for non grow-id accounts.
	WinSpoof	spoof_win	= ::wsMimic;	//Mode of zf/fz spoofing mode. See the WinSpoof enum for more details.
	FlagMode	flag_mode	= ::fmRandom; //See FlagMode enum for details.
	std::string	flag		= "us";		//set if flag_mode is set to fmCustom

	bool		custom_server_on	= false;	//when on it uses 
	std::string	custom_server_val	= "127.0.0.1"; //localhost works with proxy. custom servers and growtopia2/1 work too.

	//cheats
	namespace cheat {	
		bool	mod_zoom			= true;		//unlimited zoom. Cant ban you. cant punch further, only view.
		bool	dev_zoom			= false;	//	mod zoom with build/punch too. can ban when using punch/build far.
		bool	block_sendpacketraw	= false;	//	blocks any and all sendpacketraw requests. Acts as full ghost.
		bool	antighost			= true;		//	ignores ghost effect.
		bool	tp_click			= true;		//when on, holding CTRL allows you to tp to mouse location.
		bool	see_ghosts			= true;		//allows you to see ghosts so you can avoid constant slime
		bool	dash				= false;	//allows you to perform dashing when double clicking A/D
		bool	jump_charge			= false;	//switches jumping mode to charging
		bool	jump_cancel			= false;	//tapping W in air instantly stops the ascent and instantly starts bringing you down
		bool	antipunch			= true;		//basically (personal) punch jammer even when there is none
		bool	see_fruits			= false;	//See how many fruits a tree will have, before it has grown. Visually sets them to be fully grown though.
		bool	dialog_cheat		= false;	//Allows you to move when dialogs are opened, also punch and do other things.

		bool	punch_cooldown_on	= true;		//	custom punch cooldown
		float	punch_cooldown_val	= 0.15f;
		
		bool	gravity_on			= false;	//	custom gravity
		float	gravity_val			= 1000.0f;	

		bool	speed_on			= false;	//	custom movement speed
		float	speed_val			= 250.f;
		
		bool	accel_on			= false;	//	custom acceleration
		float	accel_val			= 1200.0f;

		bool	waterspeed_on		= false;	//	custom water speed
		float	waterspeed_val		= 125.f;
	}
	
}

//other variables
namespace global {
		HWND		hwnd	= NULL;		//	pointer to hwnd of Growtopia window
		App*		app		= nullptr;	//	pointer to App instance
		bool		unload	= false;	//	proces terminates when set to true
		bool		d9init	= false;	//	has s_renderD3D9 yet been init? if not, then check in App::Update for news
		uintptr_t	gt		= 0;		//	growtopia base address
		bool		load	= false;	//	menu/imgui initialization status
		bool		draw	= false;	//	menu draw status. set to true for menu to show up when you inject
		std::string	version	= "V0.6 public";	//	INZERNAL version
		float		fade	= 0.f;		//	does not matter, will get automatically updated
		charstate_t state{};			//	state info of player that may be modified so that we can restore it later.
}

// clang-format on

void charstate_t::copy_from_packet(GameUpdatePacket* packet) {
    //int8s
    buildrange = packet->build_range - 128;
    punchrange = packet->punch_range - 128;

    //floats
    gravity = packet->gravity_out;
    speed = packet->speed_out;
    accel = packet->accel;
    punch_strength = packet->punch_strength;
    water_speed = packet->water_speed;

    //ints
    punchid = packet->punchid;
    pupil_color = packet->pupil_color;
    eye_color = packet->eyecolor;
    eye_shade_color = packet->eye_shade_color;
    eff_flags1 = packet->effect_flags;
    eff_flags2 = packet->flags;
    if (opt::cheat::antighost) {//remove mindcontrol mod
        packet->flags &= ~(1 << 11);
        eff_flags2 &= ~(1 << 11); 
    }
}

void charstate_t::copy_inject(NetAvatar* player, bool is_local) {
    gravity = player->gravity.get(is_local);
    speed = player->speed.get(is_local);
    accel = player->accel.get(is_local);
    punch_strength = player->punch_strength.get(is_local);
    water_speed = player->water_speed;

    //cant copy more than this because we dont know all the values in NetAvatar ATM
}
