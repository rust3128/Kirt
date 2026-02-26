cmake_minimum_required(VERSION 3.19)

# Об'єднуємо назву, версію та мови в один виклик!
project(Kirt VERSION 0.1 LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(Kirt
    WIN32 MACOSX_BUNDLE
    main.cpp
    res/resources.rc
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_link_libraries(Kirt
    PRIVATE
        Qt::Core
        Qt::Widgets
)

# Підключаємо скрипт версіонування ПІСЛЯ створення цілі (Kirt)
include(versioning.cmake)

include(GNUInstallDirs)

install(TARGETS Kirt
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Kirt
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
