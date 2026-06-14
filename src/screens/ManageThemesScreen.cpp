/*
 * Themiify - A theme manager for the Nintendo Wii U
 * Copyright (C) 2026 Fangal-Airbag  
 * Copyright (C) 2026 AlphaCraft9658
 * Copyright (C) 2026  Daniel K. O. <dkosmari>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ManageThemesScreen.h"
#include "InstallThemePopup.h"
#include "ThemeDetailsPopup.h"
#include "DeleteThemePopup.h"
#include "../installer.h"
#include "../utils.h"
#include "../IconsFontAwesome4.h"

#include <iostream>
#include <unordered_map>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <imgui.h>
#include <imgui_raii.h>

using std::cout;
using std::endl;
using namespace std::literals;

namespace ManageThemesScreen {
    enum class Tab {
        manage_installed,
        install_local,
    };

    Tab current_tab = Tab::manage_installed;

    std::vector<std::filesystem::path> local_themes;
    std::vector<std::filesystem::path> json_files;

    std::vector<Installer::installed_theme_data> installed_themes;

    bool local_themes_refresh = true;
    bool is_current_theme = false;

    SDL_Renderer *manage_renderer;
    
    SDL_Texture *thumbnail;
    std::unordered_map<std::string, SDL_Texture*> thumbnail_cache;
    SDL_Texture* placeholder_thumbnail = nullptr;    

    std::string search;
    std::string current_theme;

    void scan_local_themes() {
        local_themes.clear();

        for (auto& entry : std::filesystem::directory_iterator(THEMES_ROOT)) {
            if (entry.is_regular_file() && entry.path().extension() == ".utheme") {
                local_themes.push_back(entry.path());
            }
        }
    }  
    
    void scan_installed_themes() {
        json_files.clear();
        installed_themes.clear();

        for (auto& entry : std::filesystem::directory_iterator(THEMIIFY_INSTALLED_THEMES)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                json_files.push_back(entry.path());
            }
        }
    }

    void initialize(SDL_Renderer *renderer) {
        cout << "Hello from InstalledScreen init!" << endl;
        std::filesystem::create_directories(THEMES_ROOT);
        std::filesystem::create_directories(THEMIIFY_INSTALLED_THEMES);

        manage_renderer = renderer;

        placeholder_thumbnail = IMG_LoadTexture(manage_renderer, "fs:/vol/content/ui/theme-placeholder-icon.png");
    }

    void finalize() {
        cout << "Hello from InstalledScreen finalize!" << endl;

        for (auto& [path, tex] : thumbnail_cache) {
            if (tex)
                SDL_DestroyTexture(tex);
        }

        thumbnail_cache.clear();

        if (placeholder_thumbnail) {
            SDL_DestroyTexture(placeholder_thumbnail);
            placeholder_thumbnail = nullptr;
        }
    }

    void force_refresh() {
        local_themes_refresh = true;
    }

    SDL_Texture *get_thumbnail(const std::filesystem::path& path) {
        std::string key = path.string();

        auto it = thumbnail_cache.find(key);
        if (it != thumbnail_cache.end())
            return it->second;

        SDL_Texture* tex = IMG_LoadTexture(manage_renderer, key.c_str());

        if (!tex)
            return placeholder_thumbnail;

        thumbnail_cache[key] = tex;
        return tex;
    }    

    void process_ui() {
        using namespace ImGui::RAII;

        Installer::installed_theme_data theme_data;

        Child content{
            "ManageThemesContent",
            {0, 0},
            ImGuiChildFlags_None
        };

        if (!content)
            return;

        float tab_width =
            (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.50f;

        constexpr float tab_height = 60.0f;

        if (ImGui::Selectable(
                "Manage Installed Themes",
                current_tab == Tab::manage_installed,
                0,
                {tab_width, tab_height}))
        {
            current_tab = Tab::manage_installed;
            force_refresh();
        }

        ImGui::SameLine();

        if (ImGui::Selectable(
                "Install Local Themes",
                current_tab == Tab::install_local,
                0,
                {tab_width, tab_height}))
        {
            current_tab = Tab::install_local;
            force_refresh();
        }

        ImGui::Separator();
        ImGui::Spacing();

        switch (current_tab) {
            case Tab::manage_installed: {
                ImGui::Text("Manage your installed themes here.");

                ImGui::SameLine();

                SDL_WiiUSetSWKBDHintText("Input the name of a theme to search for it...");
                SDL_WiiUSetSWKBDOKLabel("Search");
                SDL_WiiUSetSWKBDShowWordSuggestions(SDL_TRUE);
                SDL_WiiUSetSWKBDHighlightInitialText(SDL_TRUE);
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::InputTextWithHint("##local_search"s, "Search..."s, search);
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    cout << "Searching: " << search << endl;
                    //fetch_page(1);
                }                

                ImGui::Spacing();

                if (local_themes_refresh) {
                    scan_installed_themes();
                    current_theme = Installer::GetCurrentTheme();

                    auto current_it = std::find_if(
                        json_files.begin(),
                        json_files.end(),
                        [&](const auto& path)
                        {
                            Installer::installed_theme_data data;
                            Installer::GetInstalledThemeMetadata(path, &data);

                            return data.themeName + " (" + data.themeIDPath + ")" == current_theme;
                        }
                    );

                    if (current_it != json_files.end())
                        std::rotate(json_files.begin(), current_it, current_it + 1);

                    local_themes_refresh = false;
                }

                for (auto json = json_files.begin(); json != json_files.end(); ) {
                    Installer::GetInstalledThemeMetadata(*json, &theme_data);

                    std::filesystem::path current_json_path = *json;

                    if (!std::filesystem::exists(theme_data.installedThemePath)) {
                        // Modpack doesn't exist so we should delete the json because the user "uninstalled" the theme themselves
                        DeletePath(*json);
                        json = json_files.erase(json);
                        force_refresh();
                        continue;
                    }

                    ++json;

                    if (std::string(theme_data.themeName + " (" + theme_data.themeIDPath + ")") == current_theme) {
                        is_current_theme = true;
                    }
                    else {
                        is_current_theme = false;
                    }

                    Child theme_frame{
                        theme_data.themeIDPath,
                        {0, 320},
                        ImGuiChildFlags_NavFlattened |
                        ImGuiChildFlags_FrameStyle,
                        ImGuiWindowFlags_NoSavedSettings
                    };

                    if (!theme_frame)
                        continue;

                    if (is_current_theme) {
                        auto* draw_list = ImGui::GetWindowDrawList();

                        ImVec2 min = ImGui::GetWindowPos();
                        ImVec2 max = {
                            min.x + ImGui::GetWindowSize().x,
                            min.y + ImGui::GetWindowSize().y
                        };

                        constexpr float rounding = 16.0f;

                        draw_list->AddRect(
                            min,
                            max,
                            IM_COL32(50, 220, 50, 255),
                            rounding,
                            ImDrawFlags_RoundCornersAll,
                            6.0f
                        );

                        constexpr float radius = 18.0f;

                        ImVec2 badge_center{
                            max.x - radius - 12.0f,
                            min.y + radius + 12.0f
                        };

                        draw_list->AddCircleFilled(
                            badge_center,
                            radius,
                            IM_COL32(50, 220, 50, 255)
                        );

                        draw_list->AddCircle(
                            badge_center,
                            radius,
                            IM_COL32(255, 255, 255, 255),
                            0,
                            2.0f
                        );

                        const char* star = ICON_FA_STAR;

                        ImVec2 star_size = ImGui::CalcTextSize(star);

                        draw_list->AddText(
                            {
                                badge_center.x - star_size.x * 0.5f,
                                badge_center.y - star_size.y * 0.5f
                            },
                            IM_COL32(255, 255, 255, 255),
                            star
                        );
                    }

                    std::filesystem::path thumbnailPath = std::string(std::string(THEMIIFY_ROOT) + "/cache/thumbnails/" + theme_data.themeIDPath + ".webp");
                    SDL_Texture* thumbnail = placeholder_thumbnail;

                    if (std::filesystem::exists(thumbnailPath)) {
                        thumbnail = get_thumbnail(thumbnailPath);
                    }

                    ImGui::Image((ImTextureID)thumbnail, {426, 240});

                    ImGui::SameLine();


                    {
                        Group right_group;
                        ImGui::TextWrapped("Name: %s", theme_data.themeName.c_str());
                        ImGui::TextWrapped("Author: %s", theme_data.themeAuthor.c_str());

                        if (ImGui::Button(ICON_FA_INFO_CIRCLE " Details")) {
                            ThemeDetailsPopup::show_local(theme_data, thumbnail, is_current_theme);
                        }
                        
                        ImGui::SameLine();
                        
                        {
                            Disabled disable_when{is_current_theme};
                            if (ImGui::Button(ICON_FA_STAR " Make Default")) {
                                Installer::SetCurrentTheme(theme_data.themeName, theme_data.themeIDPath);
                                force_refresh();
                            }
                        }

                        ImGui::Spacing();

                        if (ImGui::Button(ICON_FA_TRASH " Delete")) {
                            DeleteThemePopup::show(theme_data, current_json_path);
                        }
                    }
                }
                break;
            }
            case Tab::install_local:
                ImGui::Text("Install .utheme files from sd:/wiiu/themes here.");

                ImGui::Spacing();
            
                if (local_themes_refresh) {
                    scan_local_themes();
                    local_themes_refresh = false;
                }
            
                for (const auto& utheme_path : local_themes) {
                    std::string id = utheme_path.string();

                    Child theme_frame{
                        id.c_str(),
                        {0, 80},
                        ImGuiChildFlags_NavFlattened |
                        ImGuiChildFlags_FrameStyle,
                        ImGuiWindowFlags_NoSavedSettings |
                        ImGuiWindowFlags_NoScrollbar |
                        ImGuiWindowFlags_NoScrollWithMouse
                    };

                    if (!theme_frame)
                        continue;

                    ImGui::TextWrapped(
                        "%s",
                        utheme_path.filename().string().c_str()
                    );

                    ImGui::SameLine();

                    ImVec2 install_button_size{150.0f, 50.0f};
                    ImVec2 trash_button_size{50.0f, 50.0f};

                    float spacing = ImGui::GetStyle().ItemSpacing.x;

                    float total_width =
                        install_button_size.x +
                        spacing +
                        trash_button_size.x;

                    float start_x =
                        ImGui::GetWindowWidth()
                        - total_width
                        - ImGui::GetStyle().WindowPadding.x;

                    ImGui::SetCursorPosX(start_x);

                    if (ImGui::Button(ICON_FA_DOWNLOAD " Install", install_button_size)) {
                        Installer::theme_data theme_data;
                        Installer::GetThemeMetadata(utheme_path, &theme_data);
                        InstallThemePopup::show(utheme_path, theme_data, false, true);
                    }

                    ImGui::SameLine();

                    if (ImGui::Button(ICON_FA_TRASH, trash_button_size)) {
                        DeletePath(utheme_path);
                        force_refresh();
                    }
                }
                break;
        }

        ThemeDetailsPopup::process_ui();
        InstallThemePopup::process_ui();
        DeleteThemePopup::process_ui();
    }

}
