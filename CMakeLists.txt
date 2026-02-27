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
    Kirt/kirtform.h Kirt/kirtform.cpp Kirt/kirtform.ui
    Kirt/ilogparser.h Kirt/ilogparser.cpp
    Kirt/operatorlogparser.h
    Kirt/operatorlogparser.cpp
    Kirt/logmodel.h Kirt/logmodel.cpp
    Kirt/optimumlogparser.h Kirt/optimumlogparser.cpp
    Kirt/bposlogparser.h Kirt/bposlogparser.cpp
    Kirt/ecrcommxlogparser.h Kirt/ecrcommxlogparser.cpp
    Kirt/cardpaylogparser.h Kirt/cardpaylogparser.cpp
    Kirt/smartcardslogparser.h Kirt/smartcardslogparser.cpp
    Kirt/talonshttplogparser.h Kirt/talonshttplogparser.cpp
    Kirt/logsyntaxhighlighter.h Kirt/logsyntaxhighlighter.cpp
    Kirt/mposloyaltylogparser.h Kirt/mposloyaltylogparser.cpp
    Kirt/loyaltylogparser.h Kirt/loyaltylogparser.cpp
    Kirt/dlottologparser.h Kirt/dlottologparser.cpp
    Kirt/logfilterproxymodel.h Kirt/logfilterproxymodel.cpp

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
