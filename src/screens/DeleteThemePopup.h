#pragma once

#include <filesystem>

#include "../installer.h"

namespace DeleteThemePopup {
    void show(Installer::installed_theme_data installed_theme_data, std::filesystem::path theme_json_path);

    void process_ui();
}
