/*
 * Themiify - A theme manager for the Nintendo Wii U
 * Copyright (C) 2026 Fangal-Airbag
 * Copyright (C) 2026 AlphaCraft9658
 * Copyright (C) 2026  Daniel K. O. <dkosmari>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "App.h"
#include "NavBar.h"
#include "ContentPanel.h"
#include "ThemezerAPI.h"
#include "ImageLoader.h"
#include "DownloadManager.h"
#include "Camera.h"
#include "utils.h"

#include <fstream>
#include <iostream>
#include <span>
#include <vector>

#include <coreinit/memory.h>
#include <coreinit/debug.h>
#include <proc_ui/procui.h>

#include <mocha/mocha.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_raii.h>
#include <imgui_stdlib.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#ifdef IMGUI_ENABLE_FREETYPE
#include <imgui_freetype.h>
#endif

#include <curl/curl.h>

// Define this to help seeing the padding and spacing values for windows.
// #define DEBUG_BG_COLOR

using std::cout;
using std::cerr;
using std::endl;

namespace App {
    SDL_Window *window;
    SDL_Renderer *renderer;

    std::vector<SDL_GameController *> controllers;
    std::vector<char> bgMusicData;

    Mix_Music *bgMusic;

    bool isRunning;

    static uint32_t procCallbackAcquire(void *content) {
        if (Camera::is_initialized())
            Camera::open();

        return 0;
    }

    void initialize_imgui() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

#if 1
        ImGuiContext& g = *ImGui::GetCurrentContext();
        g.ConfigNavWindowingWithGamepad = false;
        g.ConfigNavWindowingKeyNext = 0;
        g.ConfigNavWindowingKeyPrev = 0;
#endif

        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

#ifdef IMGUI_ENABLE_FREETYPE
        io.Fonts->FontLoaderFlags |= ImGuiFreeTypeLoaderFlags_LoadColor;
        io.Fonts->FontLoaderFlags |= ImGuiFreeTypeLoaderFlags_Bitmap;
#endif

        io.ConfigDragScroll = true;
        io.ConfigWindowsMoveFromTitleBarOnly = true;
        io.MouseDragThreshold = 25;

        io.ConfigInputTrickleEventQueue = false;

        auto &style = ImGui::GetStyle();
        style.ScaleAllSizes(3);
        // style.WindowBorderSize = 0.0f;
        // style.WindowPadding = {6.0f, 6.0f};

        style.FrameRounding = 12.0f;
        style.ChildRounding = 0.0f;
        style.GrabRounding = 12.0f;
        style.PopupRounding = 12.0f;

        auto &colors = style.Colors;
#ifdef DEBUG_BG_COLOR
        colors[ImGuiCol_WindowBg] = {0.5, 0.0f, 0.0f, 1.0f};
#else
        colors[ImGuiCol_WindowBg] = {0.0, 0.0f, 0.0f, 1.0f};
#endif

        style.FontSizeBase = 30;

        ImFontConfig fontConfig;
        fontConfig.Flags |= ImFontFlags_NoLoadError;
        fontConfig.EllipsisChar = U'…';
#ifdef IMGUI_ENABLE_FREETYPE
        // WORKAROUND: the freetype backend seems to misalign fonts merged with FontAwesome
        fontConfig.GlyphOffset.y = -style.FontSizeBase * (1.0f / 8.0f);
#endif
        void *fontData = nullptr;
        uint32_t fontSize = 0;
        OSGetSharedData(OS_SHAREDDATATYPE_FONT_STANDARD, 0, &fontData, &fontSize);

        io.Fonts->AddFontFromMemoryTTF(fontData, fontSize, style.FontSizeBase, &fontConfig);
        fontConfig.MergeMode = true;
        io.Fonts->AddFontFromFileTTF("fs:/vol/content/fonts/fontawesome-webfont.ttf", style.FontSizeBase, &fontConfig);
        io.Fonts->AddFontFromFileTTF("fs:/vol/content/fonts/InterVariable.ttf", style.FontSizeBase, &fontConfig);
        io.Fonts->AddFontFromFileTTF("fs:/vol/content/fonts/Symbola.ttf", style.FontSizeBase, &fontConfig);

        ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer2_Init(renderer);
    }

    void initialize() {
        std::filesystem::create_directories(THEMIIFY_ROOT);

        MochaUtilsStatus res;
        if ((res = Mocha_InitLibrary()) != MOCHA_RESULT_SUCCESS) {
            OSFatal("FATAL ERROR:\nCould not initialize Mocha.\n\nPlease make sure you are running on the latest version of Aroma");
        }

        if ((res = Mocha_MountFS("storage_mlc", nullptr, "/vol/storage_mlc01")) != MOCHA_RESULT_SUCCESS) {
            OSFatal("FATAL ERROR:\nCould not mount storage_mlc.\n\nPlease make sure you are running on the latest version of Aroma");
        }

        curl_global_init(CURL_GLOBAL_DEFAULT);

        ThemezerAPI::initialize(user_agent);

        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER);
        IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_WEBP);
        Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG);

        Mix_OpenAudioDevice(48000, MIX_DEFAULT_FORMAT, 2, 1024, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);

        SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

        window = SDL_CreateWindow("Themiify", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

        cout << "Hello world from Themiify!" << endl;

        initialize_imgui();
#if 0
        for (int i = 0; i < SDL_NumJoysticks(); i++) {
            SDL_GameController *controller = nullptr;
            if (SDL_IsGameController(i)) {
                controller = SDL_GameControllerOpen(i);
                if (controller) {
                    controllers.push_back(controller);
                }
            }
        }
#endif
        Camera::initialize(renderer);
        Camera::open();

        DownloadManager::initialize(user_agent);
        ImageLoader::initialize(renderer);
        NavBar::initialize(renderer);
        ContentPanel::initialize(renderer);

        // Register proc_ui callback for camera
        ProcUIRegisterCallback(PROCUI_CALLBACK_ACQUIRE, &procCallbackAcquire, nullptr, 1);

        // Start playing bg music, AFTER settings are initialized (through the ContentPanel).
        {
            std::filebuf music_filebuf;
            if (music_filebuf.open("fs:/vol/content/sound/bgm.mp3",
                                   std::ios::in | std::ios::binary)) {
                char buf[4096];
                std::streamsize read;
                while ((read = music_filebuf.sgetn(buf, sizeof buf)) > 0)
                    bgMusicData.append_range(std::span(buf, read));
                auto rwops = SDL_RWFromConstMem(bgMusicData.data(), bgMusicData.size());
                bgMusic = Mix_LoadMUS_RW(rwops, 1);
                if (bgMusic)
                    Mix_PlayMusic(bgMusic, -1);
                else
                    cerr << "Failed to load bgm: " << SDL_GetError() << endl;
            } else {
                cerr << "Failed to open bgm.mp3" << endl;
            }
        }
    }

    void finalize() {

        Mix_PlayMusic(nullptr, 0);
        if (bgMusic) {
            Mix_FreeMusic(bgMusic);
            bgMusic = nullptr;
        }

        NavBar::finalize();
        ContentPanel::finalize();
        ImageLoader::finalize();
        DownloadManager::finalize();

        Camera::close();
        Camera::shutdown();

        Mix_Quit();

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);

        SDL_Quit();

        ThemezerAPI::finalize();

        curl_global_cleanup();

        Mocha_UnmountFS("storage_mlc");
        Mocha_DeInitLibrary();
    }

    bool run() {
        isRunning = true;

        while (isRunning) {
            try {
                ThemezerAPI::process();
            }
            catch (std::exception& e) {
                cerr << "ERROR in ThemezerAPI::process(): " << e.what() << endl;
            }

            try {
                DownloadManager::process();
            }
            catch (std::exception& e) {
                cerr << "ERROR in DownloadManager::process(): " << e.what() << endl;
            }

            SDL_Event e;
            while(SDL_PollEvent(&e)) {
                ImGui_ImplSDL2_ProcessEvent(&e);
                switch (e.type) {
                    case SDL_QUIT: {
                        cout << "Quitting Themiify!" << endl;
                        isRunning = false;
                        break;
                    }
                    case SDL_CONTROLLERDEVICEADDED: {
                        SDL_GameController *controller = SDL_GameControllerOpen(e.cdevice.which);
                        if (controller) {
                            controllers.push_back(controller);
                        }
                        break;
                    }
                    case SDL_CONTROLLERDEVICEREMOVED: {
                        SDL_GameController *controller = SDL_GameControllerFromInstanceID(e.cdevice.which);
                        for (auto it = controllers.begin(); it != controllers.end(); ++it) {
                            if (*it == controller) {
                                SDL_GameControllerClose(*it);
                                controllers.erase(it);
                                break;
                            }
                        }
                        break;
                    }
                    /*case SDL_SYSWMEVENT: {
                        auto *msg = e.syswm.msg;
                        if (!msg)
                            break;

                        switch (msg->msg.wiiu.event) {
                            case SDL_WIIU_SYSWM_SWKBD_OK_FINISH_EVENT: {
                                swkbd_ok_selected = true;
                                break;
                            }

                            default:
                                break;
                        }


                    }*/

                    default:
                        break;
                }
            }

            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame();

            ImGui::NewFrame();

            {
                using namespace ImGui::RAII;
                auto viewport = ImGui::GetMainViewport();
                ImGui::SetNextWindowPos(viewport->Pos, ImGuiCond_Always);
                ImGui::SetNextWindowSize(viewport->Size, ImGuiCond_Always);
                // NOTE: on the toplevel window, we disable padding, rounding, border,
                // but we don't want this to propagate to the rest of the UI code.
                const auto &style = ImGui::GetStyle();
                auto orig_padding  = style.WindowPadding;
                auto orig_border   = style.WindowBorderSize;
                auto orig_rounding = style.WindowRounding;
                StyleVar no_padding{ImGuiStyleVar_WindowPadding, {0, 0}};
                StyleVar no_border{ImGuiStyleVar_WindowBorderSize, 0};
                StyleVar no_rounding{ImGuiStyleVar_WindowRounding, 0};
                if (Window main_window{"Themiify",
                                       nullptr,
                                       ImGuiWindowFlags_NoTitleBar |
                                       ImGuiWindowFlags_NoMove |
                                       ImGuiWindowFlags_NoSavedSettings |
                                       ImGuiWindowFlags_NoResize}) {
                    // Restore the window properties.
                    StyleVar restore_padding{ImGuiStyleVar_WindowPadding, orig_padding};
                    StyleVar restore_border{ImGuiStyleVar_WindowBorderSize, orig_border};
                    StyleVar restore_rounding{ImGuiStyleVar_WindowRounding, orig_rounding};
                    NavBar::process_ui();
                    ImGui::SameLine(0, 0); // NOTE: ignore ItemSpacing
                    ContentPanel::process_ui(NavBar::get_current_tab());
                }
            }

            // ImGui::ShowDemoWindow();
            // ImGui::ShowStyleEditor();

            ImGui::EndFrame();

            ImGui::Render();

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);

            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

            // Wii U clip fix
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
            SDL_RenderDrawPoint(renderer, 0, 0);

            SDL_RenderPresent(renderer);
        }

        return isRunning;
    }

    void quit() {
        isRunning = false;
    }
}
