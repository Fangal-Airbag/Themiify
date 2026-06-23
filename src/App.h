/*
 * Themiify - A theme manager for the Nintendo Wii U
 * Copyright (C) 2026 Fangal-Airbag  
 * Copyright (C) 2026 AlphaCraft9658
 * Copyright (C) 2026  Daniel K. O. <dkosmari>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <string>
#include "utils.h"

namespace App {
    const std::string user_agent = "Themiify/" + std::string(THEMIIFY_VERSION) + " (Wii U)";

    void initialize();

    void finalize();

    bool run();

    void quit();
}
