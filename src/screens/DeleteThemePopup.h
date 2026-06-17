/*
 * Themiify - A theme manager for the Nintendo Wii U
 * Copyright (C) 2026 Fangal-Airbag  
 * Copyright (C) 2026 AlphaCraft9658
 * Copyright (C) 2026  Daniel K. O. <dkosmari>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <filesystem>

#include "../installer.h"

namespace DeleteThemePopup {
    void show(Installer::installed_theme_data installed_theme_data, std::filesystem::path theme_json_path);

    void process_ui();
}
