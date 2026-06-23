/*
 * Themiify - A theme manager for the Nintendo Wii U
 * Copyright (C) 2026 Fangal-Airbag  
 * Copyright (C) 2026 AlphaCraft9658
 * Copyright (C) 2026  Daniel K. O. <dkosmari>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <SDL2/SDL.h>

namespace SettingsScreen {
    bool check_is_first_boot();
    void run_first_boot_check();
    void run_boot_integrity_check();
    
    void initialize(SDL_Renderer *renderer);

    void finalize();

    void process_ui();
}
