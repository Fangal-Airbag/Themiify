/*
 * Themiify - A theme manager for the Nintendo Wii U
 * Copyright (C) 2026 Fangal-Airbag
 * Copyright (C) 2026 AlphaCraft9658
 * Copyright (C) 2026  Daniel K. O. <dkosmari>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <iostream>
#include <filesystem>
#include <string>

#include <imgui.h>
#include <imgui_raii.h>

#include "DownloadThemePopup.h"
#include "InstallThemePopup.h"
#include "../utils.h"
#include "../DownloadManager.h"
#include "../humanize.hpp"
#include "../installer.h"

using std::cout;
using std::endl;
using namespace std::literals;

namespace DownloadThemePopup {
    enum class State {
        hidden,
        confirmation,
        downloading,
        error,
        success,
    };

    State state;

    bool popup_queued;
    const std::string popup_id = "Download Theme";
    std::filesystem::path utheme_path;

    ThemezerAPI::WiiuThemeSmall theme;

    bool set_current = true;

    void show(const ThemezerAPI::WiiuThemeSmall &theme_data) {
        state = State::confirmation;
        popup_queued = true;
        theme = theme_data;
        utheme_path = "";
    }

    void process_ui() {
        using namespace ImGui::RAII;
        if (state == State::hidden)
            return;

        if (popup_queued) {
            ImGui::OpenPopup(popup_id);
            popup_queued = false;
        }

        auto viewport = ImGui::GetMainViewport();
        // WORKAROUND: setting an initial size helps with the initial position not jumping around.
        ImGui::SetNextWindowSize(viewport->Size * 0.75f, ImGuiCond_Appearing);
        auto center = viewport->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Always, {0.5f, 0.5f});
        PopupModal popup{popup_id, nullptr,
                         ImGuiWindowFlags_NoSavedSettings |
                         ImGuiWindowFlags_AlwaysAutoResize |
                         ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoScrollWithMouse |
                         ImGuiWindowFlags_NoCollapse |
                         // ImGuiWindowFlags_NoTitleBar |
                         ImGuiWindowFlags_None
        };

        if (!popup) {
            state = State::hidden;
            return;
        }

        const auto &style = ImGui::GetStyle();

        switch (state) {
            case State::confirmation: {
                {
                    Font title_font{nullptr, 35};
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Download Confirmation");
                }

                ImGui::TextWrapped("Would you like to download the theme:\n%s ?", theme.name.c_str());

                ImGui::Spacing();

                ImVec2 button_size{180.0f, 60.0f};

                float spacing = style.ItemSpacing.x;
                float total_width = button_size.x * 2.0f + spacing;

                float start_x = (ImGui::GetContentRegionAvail().x - total_width) * 0.5f;

                if (start_x > 0.0f)
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + start_x);

                if (ImGui::Button("Download", button_size)) {
                    if (DownloadManager::add("Theme: " + theme.name,
                                             theme.downloadUrl,
                                             theme.collagePreview.thumbUrl,
                                             THEMES_ROOT / (theme.slug + ".utheme"),
                                             THEMIIFY_THUMBNAILS / ("Themezer" + theme.hexId + ".webp"),
                                             {},
                                             {})) {
                    }

                    state = State::downloading;
                }
                ImGui::SetItemDefaultFocus();

                ImGui::SameLine();

                if (ImGui::Button("Cancel", button_size)) {
                    ImGui::CloseCurrentPopup();
                    state = State::hidden;
                }

                ImGui::Spacing();

                break;
            }
            case State::downloading: {
                auto& infos = DownloadManager::get_infos();
                // Don't really wanna do a multiple downloads approach
                // I think this is cleaner for actually installing themes
                // Afterwards
                auto& info = infos.at(0);

                utheme_path = info->utheme_output;

                {
                    Font title_font{nullptr, 35};
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Downloading Theme...");
                }

                ImGui::Text(info->label);

                ImGui::Text(info->utheme_url);

                ImGui::TextWrapped("Saving to: %s", info->utheme_output.filename().c_str());

                //auto speed = humanize::value_bin(info->speed) + "B/s";
                //ImGui::Text("DL speed: %s", speed.data());

                ImGui::ProgressBar(info->progress);

                if (info->progress >= 1.0f) {
                    DownloadManager::clear_finished();
                    state = State::success;
                }

                break;
            }
            case State::error: {
                break;
            }
            case State::success: {
                {
                    Font title_font{nullptr, 50};
                    ImGui::AlignTextToFramePadding();
                    ImGui::Text("Download successful!");
                }

                ImGui::TextWrapped("Would you like to install this theme for StyleMiiU plugin?");

                ImGui::Checkbox("Set as current theme", &set_current);

                ImGui::Spacing();

                ImVec2 button_size{180.0f, 60.0f};

                float spacing = style.ItemSpacing.x;
                float total_width = button_size.x * 2.0f + spacing;

                float start_x = (ImGui::GetContentRegionAvail().x - total_width) * 0.5f;

                if (start_x > 0.0f)
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + start_x);

                if (ImGui::Button("Install", button_size)) {
                    Installer::theme_data theme_data;
                    Installer::GetThemeMetadata(utheme_path, &theme_data);

                    ImGui::CloseCurrentPopup();
                    state = State::hidden;

                    InstallThemePopup::show(utheme_path, theme_data, true, set_current);
                }
                ImGui::SetItemDefaultFocus();

                ImGui::SameLine();

                if (ImGui::Button("Cancel", button_size)) {
                    ImGui::CloseCurrentPopup();
                    state = State::hidden;
                }

                ImGui::Spacing();

                break;
            }
            default:
                break;
        }

    }
}
