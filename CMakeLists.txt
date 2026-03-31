include(/opt/devkitpro/cmake/WiiU.cmake)
cmake_minimum_required(VERSION 3.16)

project(themiify C CXX)

add_executable(themiify
    src/main.cpp
)
wut_create_rpx(themiify)
wut_create_wuhb(themiify
    CONTENT content
    NAME Themiify
    SHORTNAME Themiify
    AUTHOR Fangal_Airbag, Alpha-Craft, dkosmari & more!
    ICON meta/icon.png
    TVSPLASH meta/tv-splash.png
    DRCSPLASH meta/drc-splash.png
)