/*
 * Themiify - A theme manager for the Nintendo Wii U
 * Copyright (C) 2026 Fangal-Airbag  
 * Copyright (C) 2026 AlphaCraft9658
 * Copyright (C) 2026  Daniel K. O. <dkosmari>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "HomeScreen.h"

#include <iostream>

#include <imgui.h>
#include <imgui_raii.h>

using std::cout;
using std::endl;

namespace HomeScreen {
    void initialize(SDL_Renderer *renderer) {
        cout << "Hello from HomeScreen init!" << endl;
    }

    void finalize() {
        cout << "Hello from HomeScreen finalize!" << endl;
    }

    void process_ui() {
        using namespace ImGui::RAII;

        Child home_content{"HomeContent", {0, 0}, ImGuiChildFlags_NavFlattened};
        if (!home_content)
            return;
        
        {
            Font font_guard{nullptr, 36};
            ImGui::Text("Themiify Home Screen!");
        }
        
    }
}
