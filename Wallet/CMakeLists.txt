# add C language
project(Elastos-wallet LANGUAGES CXX C)

# add config.h for qrencode
add_definitions(-D HAVE_CONFIG_H)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5 COMPONENTS Gui REQUIRED)

include(ProjectDefaults)
include(CheckIncludeFile)
include(CheckFunctionExists)

check_include_file(unistd.h HAVE_UNISTD_H)
if(HAVE_UNISTD_H)
    add_definitions(-DHAVE_UNISTD_H=1)
endif()

check_include_file(getopt.h HAVE_GETOPT_H)
if(HAVE_GETOPT_H)
    add_definitions(-DHAVE_GETOPT_H=1)
endif()

check_include_file(sys/time.h HAVE_SYS_TIME_H)
if(HAVE_SYS_TIME_H)
    add_definitions(-DHAVE_SYS_TIME_H=1)
endif()

check_include_file(sys/resource.h HAVE_SYS_RESOURCE_H)
if(HAVE_SYS_RESOURCE_H)
    add_definitions(-DHAVE_SYS_RESOURCE_H=1)
endif()

#file(GLOB_RECURSE WALLET_SOURCE_FILES "*.cpp")
file(GLOB SRC_FILES "./*.cpp")
file(GLOB QR_SRC_FILES "qrencode/*.c")
file(GLOB HEAD_FILES "./*.h")
file(GLOB QR_HEAD_FILES "qrencode/*.h")
file(GLOB HPP_FILES "./*.hpp")
file(GLOB UI_FILES "./*.ui")
set(QRC_FILES src.qrc)

if(APPLE)
    # set AppIcon for MacOS
    set(MACOSX_BUNDLE_GUI_IDENTIFIER ${IDENTIFIER})
    set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})

    set(MACOSX_BUNDLE_COPYRIGHT ${COPYRIGHT})

    set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${spectral_VERSION})
    set(MACOSX_BUNDLE_BUNDLE_VERSION ${spectral_VERSION})

    set(ICON_NAME "AppIcon.icns")
    set(${PROJECT_NAME}_MAC_ICON "${PROJECT_SOURCE_DIR}/icons/${ICON_NAME}")
    set(MACOSX_BUNDLE_ICON_FILE ${ICON_NAME})
    set_property(SOURCE "${${PROJECT_NAME}_MAC_ICON}" PROPERTY
        MACOSX_PACKAGE_LOCATION Resources)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(SYSTEM_LIBS pthread dl)
endif()

include_directories(
    ../Interface
    ${PROJECT_INT_DIST_DIR}/include
)

link_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/../SDK
    ${PROJECT_INT_DIST_DIR}/lib
)

if (SPV_ENABLE_SHARED)
    set(LIBS spvsdk resolv)
	set(ELAWALLET_DEPENDS libspvsdk)
elseif(SPV_ENABLE_STATIC)
    set(LIBS spvsdk-static sqlite3 crypto ssl boost_system boost_filesystem boost_thread fruit resolv)
    set(ELAWALLET_DEPENDS libspvsdk libfruit boost libressl json libsqlite)
else()
	message(FATAL_ERROR "libspvsdk should enable static or shared")
endif()

if(APPLE)
    add_executable(wallet MACOSX_BUNDLE
        ${SRC_FILES}
        ${QR_SRC_FILES}
        ${HEAD_FILES}
        ${QR_HEAD_FILES}
        ${HPP_FILES}
        ${UI_FILES}
        ${QRC_FILES}
        # set AppIcon
        ${${PROJECT_NAME}_MAC_ICON}
        )

    # set MacOS Bundle
    set_property(TARGET wallet PROPERTY
        MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/macOS/Info.plist.in")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_executable(wallet
        ${SRC_FILES}
        ${QR_SRC_FILES}
        ${HEAD_FILES}
        ${QR_HEAD_FILES}
        ${HPP_FILES}
        ${UI_FILES}
        ${QRC_FILES}
        )
endif()

add_dependencies(wallet ${ELAWALLET_DEPENDS})

# link Qt libraries
target_link_libraries(wallet 
    PRIVATE 
    ${LIBS} ${SYSTEM_LIBS} 
    Qt5::Widgets Qt5::Core Qt5::Gui
    )
