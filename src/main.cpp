/*
 * Themiify - A theme manager for the Nintendo Wii U
 * Copyright (C) 2026 Fangal-Airbag  
 * Copyright (C) 2026 AlphaCraft9658
 * Copyright (C) 2026  Daniel K. O. <dkosmari>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

// Template for now cuz I'll actually start coding when I get home today lol
#include <whb/proc.h>
#include <whb/log.h>
#include <whb/log_console.h>

int main(int arc, char **argv)
{
    WHBProcInit();
    WHBLogConsoleInit();

    WHBLogPrint("Hello World!");

    while (WHBProcIsRunning()) {
        WHBLogConsoleDraw();
    }

    WHBLogConsoleFree();
    WHBProcShutdown();

    return 0;
}