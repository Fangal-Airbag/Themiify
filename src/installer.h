/*
 * Themiify - A theme manager for the Nintendo Wii U
 * Copyright (C) 2026 Fangal-Airbag
 * Copyright (C) 2026 AlphaCraft9658
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

#include <exception>
#include <filesystem>
#include <functional>
#include <string>
#include <stop_token>

namespace Installer {
    struct theme_data {
        std::string themeID;
        std::string themeIDPath;
        std::string themeName;
        std::string themeAuthor;
        std::string themeVersion;
    };

    struct installed_theme_data {
        std::string themeID;
        std::string themeIDPath;
        std::string themeName;
        std::string themeAuthor;
        std::string themeVersion;
        std::filesystem::path installedThemePath;
    };

    int GetThemeMetadata(const std::filesystem::path &themePath, theme_data *themeData);
    int GetInstalledThemeMetadata(const std::filesystem::path &installedThemeJsonPath, installed_theme_data *themeData);

    using progress_function_sig = void (const std::string &msg);
    using progress_function_t = std::function<progress_function_sig>;

    using success_function_sig = void ();
    using success_function_t = std::function<success_function_sig>;

    using error_function_sig = void (const std::exception &e);
    using error_function_t = std::function<error_function_sig>;

    void InstallTheme(std::stop_token &stopper,
                      const std::filesystem::path &themePath,
                      theme_data themeData,
                      progress_function_t progressCallback,
                      success_function_t successCallback,
                      error_function_t errorCallback);
    bool DeleteTheme(const std::filesystem::path &modpackPath, const std::filesystem::path &installPath);
    bool SetCurrentTheme(const std::string &themeName, const std::string &themeIDPath);
    std::string GetCurrentTheme();
} // namespace Installer
