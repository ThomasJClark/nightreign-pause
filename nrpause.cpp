#include <elden-x/menu/menu_man.hpp>
#include <elden-x/singletons.hpp>
#include <elden-x/task.hpp>
#include <elden-x/utils/modutils.hpp>

#include <mini/ini.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <filesystem>
#include <thread>

using namespace std;

class nrpause : public er::CS::CSEzTask {
    bool hotkey_pressed{false};

    // Settings from the nrpause.ini file
    bool map_menu{true};
    bool equipment_menu{false};
    bool status_menu{false};
    bool system_menu{true};
    bool shops{false};
    bool dialog{false};
    bool dormant_power{false};
    bool inspecting_item{false};
    bool hotkey{true};

public:
    nrpause(filesystem::path ini_path) {
        mINI::INIFile ini_file(ini_path.string());
        mINI::INIStructure ini;
        if (ini_file.read(ini)) {
            auto &config = ini["nrpause"];
            map_menu = config["map_menu"] != "false";
            equipment_menu = config["equipment_menu"] != "false";
            status_menu = config["status_menu"] != "false";
            system_menu = config["system_menu"] != "false";
            shops = config["shops"] != "false";
            dialog = config["dialog"] != "false";
            dormant_power = config["dormant_power"] != "false";
            inspecting_item = config["inspecting_item"] != "false";
            hotkey = config["hotkey"] != "false";
        }
    }

    void execute(er::FD4::task_data *data,
                 er::FD4::task_group group,
                 er::FD4::task_runner runner) override {
        auto menu_man = er::CS::CSMenuMan::instance();
        if (!menu_man) {
            return;
        }

        auto &tutorial_lightbox_state = menu_man->flags[er::CS::menu_flag::tutorial_lightbox];
        auto prev_tutorial_lightbox_state = tutorial_lightbox_state;

        auto is_visible = [&](er::CS::menu_flag flag) {
            auto it = menu_man->flags.find(flag);
            return it != menu_man->flags.end() && it->second.visible;
        };

        // If a menu that is configured to pause the game is currently open, set the flag that
        // indicates a tutorial is open. This causes various game systems to temporarily stop,
        // since tutorials normally pause the game.
        tutorial_lightbox_state.enabled =
            (map_menu && (is_visible(er::CS::menu_flag::map_menu) ||
                          is_visible(er::CS::menu_flag::roundtable_map))) ||
            (equipment_menu && is_visible(er::CS::menu_flag::equipment_menu)) ||
            (status_menu && is_visible(er::CS::menu_flag::status_menu)) ||
            (system_menu && (is_visible(er::CS::menu_flag::system_menu) ||
                             is_visible(er::CS::menu_flag::system_menu_controls))) ||
            (shops && is_visible(er::CS::menu_flag::shop)) ||
            (dialog && is_visible(er::CS::menu_flag::talk_list)) ||
            (dormant_power && is_visible(er::CS::menu_flag::dormant_power)) ||
            (inspecting_item && is_visible(er::CS::menu_flag::pick_up_item));

        if (hotkey) {
            // When F9 is pressed, flip the current pause state from unpaused to paused or vice
            // versa
            if (GetAsyncKeyState(VK_F9) & 1) {
                hotkey_pressed = !hotkey_pressed;
            } else if (tutorial_lightbox_state.enabled == prev_tutorial_lightbox_state.enabled) {
                hotkey_pressed = false;
            }
            if (hotkey_pressed) {
                tutorial_lightbox_state.enabled = !tutorial_lightbox_state.enabled;
            }
        }
    }
};

bool WINAPI DllMain(HINSTANCE hinst, unsigned int reason, void *reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        wchar_t dll_filename[MAX_PATH] = {0};
        GetModuleFileNameW(hinst, dll_filename, MAX_PATH);

        static auto mod_task =
            nrpause{filesystem::path(dll_filename).parent_path() / "nrpause.ini"};

        static auto mod_thread = thread{[]() {
            modutils::initialize();
            er::FD4::find_singletons();
            er::CS::CSTask::instance()->register_task(er::FD4::task_group::FrameBegin, mod_task);
        }};
    }

    return true;
}
