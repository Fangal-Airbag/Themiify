#include <cmath>
#include <iostream>

#include <imgui.h>
#include <imgui_raii.h>

#include <cafe_glyphs.h>

#include "ThemePreviewPopup.h"
#include "../ImageLoader.h"
#include "../IconsFontAwesome4.h"

using std::cout;
using std::endl;
using namespace std::literals;

namespace ThemePreviewPopup {
    enum class State {
        hidden,
        shown,
    };

    State state;

    bool popup_queued;
    const std::string popup_id = "ThemePreviewPopup"s;

    std::string launcher_url;
    std::string waraWara_url;

    int image_idx;

    bool hide_ui;

    namespace {

        bool overlay_button(const std::string& label, const ImVec2 size) {
            using ImGui::RAII::StyleColor;
            StyleColor button         {ImGuiCol_Button,        {0.0f, 0.0f, 0.0f, 0.5f}};
            StyleColor button_hovered {ImGuiCol_ButtonHovered, {0.0f, 0.0f, 0.0f, 0.7f}};
            StyleColor button_active  {ImGuiCol_ButtonActive,  {0.0f, 0.0f, 0.0f, 0.9f}};
            return ImGui::Button(label, size);
        }

    } // namespace

    void show(const std::string& launcherUrl, const std::string& waraWaraUrl) {
        state = State::shown;
        popup_queued = true;
        image_idx = 0;
        hide_ui = false;

        launcher_url = launcherUrl;
        waraWara_url = waraWaraUrl;
    }

    void process_ui() {
        using namespace ImGui::RAII;
        if (state == State::hidden)
            return;

        if (popup_queued) {
            ImGui::OpenPopup(popup_id);
            popup_queued = false;
        }

        auto& style = ImGui::GetStyle();
        auto center = ImGui::GetMainViewport()->GetCenter();
        auto viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowSize(viewport->Size, ImGuiCond_Always);
        ImGui::SetNextWindowPos(center, ImGuiCond_Always, {0.5f, 0.5f});
        Popup popup{popup_id, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize |
                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                    ImGuiWindowFlags_NoNavInputs};

        if (!popup) {
            state = State::hidden;
            return;
        }

        // TODO: we can have the combined image too, so the last will be 2
        int image_idx_last = 1;

        if (image_idx < 0)
            image_idx = 0;

        if (image_idx > image_idx_last)
            image_idx = image_idx_last;

        if (Child images_box{"images_box",
                             viewport->Size,
                             ImGuiChildFlags_None | ImGuiChildFlags_NavFlattened,
                             ImGuiWindowFlags_NoScrollbar}) {
            StyleVar no_border   {ImGuiStyleVar_ChildBorderSize, 0};
            StyleVar no_rounding {ImGuiStyleVar_ChildRounding, 0};
            StyleVar no_padding  {ImGuiStyleVar_FramePadding, {0, 0}};

            auto *launcher_tex = ImageLoader::get(launcher_url);
            auto *waraWara_tex = ImageLoader::get(waraWara_url);
            ImGui::SetNextItemAllowOverlap();
            ImGui::Image((ImTextureID)launcher_tex, viewport->Size);
            ImGui::SameLine();
            ImGui::SetNextItemAllowOverlap();
            ImGui::Image((ImTextureID)waraWara_tex, viewport->Size);

            // When not dragging, make sure the scroll pos is always lined up with an image.
            float image_width = viewport->Size.x;
            float image_spacing = style.ItemSpacing.x;
            float scroll_step = image_width + image_spacing;
            float scroll_x = ImGui::GetScrollX();
            // TODO: should only snap when both dragging and gliding stops.
            bool should_snap = !ImGui::IsMouseDragging(ImGuiMouseButton_Left);
            if (should_snap) {
                float new_scroll_x = image_idx * scroll_step;
                float diff = new_scroll_x - scroll_x;
                if (std::abs(diff) < 16) { // arbitrary threshold of 16 pixels
                    // Snap to the new_scroll_pos
                    ImGui::SetScrollX(new_scroll_x);
                } else {
                    // Smooth out the snapping
                    float smooth_scroll_x = std::lerp(scroll_x, new_scroll_x, 0.5f);
                    ImGui::SetScrollX(smooth_scroll_x);
                }
            } else {
                image_idx = std::lround(scroll_x / scroll_step);
            }

            const bool is_first_image = image_idx <= 0;
            const bool is_last_image = image_idx >= image_idx_last;

            // Bug with ImGui, I'm assuming cause they never bothered to implement
            // A Nintendo button layout :P
            if (ImGui::IsKeyPressed(ImGuiKey_GamepadFaceLeft))
                hide_ui = !hide_ui;

            if (ImGui::IsKeyPressed(ImGuiKey_GamepadL1) && !is_first_image)
                image_idx--;

            if (ImGui::IsKeyPressed(ImGuiKey_GamepadR1) && !is_last_image)
                image_idx++;

            // TODO: should detect a flick and change index too.

            if (!hide_ui) {
                ImVec2 arrow_button_size{60.0f, 60.0f};
                float middle_y = viewport->Pos.y + (viewport->Size.y - arrow_button_size.y) * 0.5f;
                float padding = 30.0f;

                ImVec2 button_size{160.0f, 60.0f};
                ImVec2 text_box_size{220.0f, 60.0f};

                float spacing = 20.0f;

                float total_width = button_size.x + spacing + text_box_size.x + spacing + button_size.x;

                float start_x = viewport->Pos.x + (viewport->Size.x - total_width) * 0.5f;

                float y = viewport->Pos.y + viewport->Size.y - padding - button_size.y;

                ImGui::SetCursorScreenPos({start_x, y});

                if (overlay_button(CAFE_GLYPH_BTN_B " Close", button_size)) {
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SetCursorScreenPos({ start_x + button_size.x + spacing, y });

                overlay_button("##info_panel", text_box_size);

                std::string label = std::string(ICON_FA_CHEVRON_LEFT) + " " + std::string(CAFE_GLYPH_BTN_L) + "/" + std::string(CAFE_GLYPH_BTN_R) + " " + std::string(ICON_FA_CHEVRON_RIGHT);

                ImVec2 text_size = ImGui::CalcTextSize(label.c_str());

                ImGui::SetCursorScreenPos({
                    start_x + button_size.x + spacing +
                        (text_box_size.x - text_size.x) * 0.5f,
                    y + (text_box_size.y - text_size.y) * 0.5f
                });

                ImGui::TextUnformatted(label);

                ImGui::SetCursorScreenPos({
                    start_x + button_size.x + spacing +
                    text_box_size.x + spacing,
                    y
                });
                if (overlay_button(CAFE_GLYPH_BTN_X " Hide", button_size)) {
                    hide_ui = true;
                }

                {
                    Disabled disable_left{is_first_image};
                    ImGui::SetCursorScreenPos({
                        viewport->Pos.x + padding,
                        middle_y
                    });
                    if (overlay_button(ICON_FA_CHEVRON_LEFT, arrow_button_size)) {
                        image_idx--;
                    }
                }

                {
                    Disabled disable_right{is_last_image};
                    ImGui::SetCursorScreenPos({
                        viewport->Pos.x + viewport->Size.x - padding - arrow_button_size.x,
                        middle_y
                    });
                    if (overlay_button(ICON_FA_CHEVRON_RIGHT, arrow_button_size)) {
                        image_idx++;
                    }
                }
            }
            else {
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                    hide_ui = false;
            }
        }
    }
}
