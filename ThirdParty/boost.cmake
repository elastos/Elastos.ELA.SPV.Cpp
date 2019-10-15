
set(BOOST_BUILD_COMPONENTS filesystem system program_options thread)
unset(BUILD_WITH_LIBRARIES)
string(REPLACE ";" "," BUILD_WITH_LIBRARIES "${BOOST_BUILD_COMPONENTS}")

set(BOOST_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/boost/build)
set(BOOST_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/boost/install)
set(BOOST_EXTRACT_DIR ${CMAKE_CURRENT_BINARY_DIR}/boost)

if (${SPV_PLATFORM} STREQUAL ${SPV_PLATFORM_ANDROID})
    set(BOOST_BUILD_COMMENT "Building boost for android...")
    set(
            BUILD_COMMAND
            ./build.sh
            --verbose
            --boost=1.67.0
            --with-libraries=${BUILD_WITH_LIBRARIES}
            --build-dir=${BOOST_BUILD_DIR}
            --prefix=${BOOST_INSTALL_DIR}
            --extract-dir=${BOOST_EXTRACT_DIR}
            --arch=${CMAKE_ANDROID_ARCH_ABI}
            --ndk-root=$ENV{ANDROID_NDK}
            --ndk-api-level=${CMAKE_SYSTEM_VERSION}
    )
    set(Boost_INCLUDE_DIRS ${BOOST_INSTALL_DIR}/${CMAKE_ANDROID_ARCH_ABI}/include)
elseif (${SPV_PLATFORM} STREQUAL ${SPV_PLATFORM_IOS})
    set(BOOST_BUILD_COMMENT "Building boost for ios...")
    set(
            BUILD_COMMAND
            SRCDIR=${BOOST_EXTRACT_DIR} OUTPUT_DIR=${BOOST_BUILD_DIR} ./boost.sh --boost-version 1.69.0 --universal -ios --boost-libs "\"${BOOST_BUILD_COMPONENTS}\""
    )
    set(Boost_INCLUDE_DIRS ${BOOST_BUILD_DIR}/ios/prefix/include)
else ()
    set(BOOST_BUILD_COMMENT "Building boost...")
    set(
            BUILD_COMMAND
            ./build.sh
            --verbose
            --with-libraries=${BUILD_WITH_LIBRARIES}
            --build-dir=${BOOST_BUILD_DIR}
            --prefix=${BOOST_INSTALL_DIR}
            --extract-dir=${BOOST_EXTRACT_DIR}
    )
    set(Boost_INCLUDE_DIRS ${BOOST_INSTALL_DIR}/include)
endif ()

unset(COMPONENTS_LIBRARY_PATH)
unset(SPV_Boost_LIBRARIES CACHE)
foreach (COMPONENT_NAME ${BOOST_BUILD_COMPONENTS})
    if (${SPV_PLATFORM} STREQUAL ${SPV_PLATFORM_ANDROID})
        set(COMPONENTS_LIBRARY_PATH ${COMPONENTS_LIBRARY_PATH} ${BOOST_INSTALL_DIR}/${CMAKE_ANDROID_ARCH_ABI}/lib/libboost_${COMPONENT_NAME}.a)
    elseif (${SPV_PLATFORM} STREQUAL ${SPV_PLATFORM_IOS})
        set(COMPONENTS_LIBRARY_PATH ${COMPONENTS_LIBRARY_PATH} ${BOOST_BUILD_DIR}/ios/build/universal/libboost_${COMPONENT_NAME}.a)
    else ()
        set(COMPONENTS_LIBRARY_PATH ${COMPONENTS_LIBRARY_PATH} ${BOOST_INSTALL_DIR}/lib/libboost_${COMPONENT_NAME}.a)
    endif ()
    set(SPV_Boost_LIBRARIES boost_${COMPONENT_NAME};${SPV_Boost_LIBRARIES})
endforeach ()

find_package(Boost 1.69 REQUIRED COMPONENTS ${BOOST_BUILD_COMPONENTS})
set( Boost_USE_STATIC_LIBS ON CACHE STRING "ON or OFF" )
if (Boost_FOUND)
    set(
            SPV_Boost_LIBRARIES
            ${Boost_LIBRARIES}
            CACHE INTERNAL "boost libraries" FORCE
    )

    set(
            ThirdParty_BOOST_INC_DIR
            ${Boost_INCLUDE_DIRS}
            CACHE INTERNAL "boost include directory" FORCE
    )
else ()
    add_custom_command(
            COMMENT ${BOOST_BUILD_COMMENT}
            OUTPUT ${COMPONENTS_LIBRARY_PATH}
            COMMAND ${BUILD_COMMAND}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/boost
    )
    add_custom_target(build_boost DEPENDS ${COMPONENTS_LIBRARY_PATH})

    foreach (COMPONENT_NAME ${BOOST_BUILD_COMPONENTS})
        add_library(boost_${COMPONENT_NAME} STATIC IMPORTED GLOBAL)
        if (${SPV_PLATFORM} STREQUAL ${SPV_PLATFORM_ANDROID})
            set(COMPONENT_LIBRARY_PATH ${BOOST_INSTALL_DIR}/${CMAKE_ANDROID_ARCH_ABI}/lib/libboost_${COMPONENT_NAME}.a)
        elseif (${SPV_PLATFORM} STREQUAL ${SPV_PLATFORM_IOS})
            set(COMPONENT_LIBRARY_PATH ${BOOST_BUILD_DIR}/ios/build/universal/libboost_${COMPONENT_NAME}.a)
        else ()
            set(COMPONENT_LIBRARY_PATH ${BOOST_INSTALL_DIR}/lib/libboost_${COMPONENT_NAME}.a)
        endif ()
        set_target_properties(boost_${COMPONENT_NAME} PROPERTIES IMPORTED_LOCATION ${COMPONENT_LIBRARY_PATH})
        add_dependencies(boost_${COMPONENT_NAME} build_boost)
    endforeach ()


    set(
            SPV_Boost_LIBRARIES
            ${SPV_Boost_LIBRARIES}
            CACHE INTERNAL "boost libraries" FORCE
    )

    set(
            ThirdParty_BOOST_INC_DIR
            ${Boost_INCLUDE_DIRS}
            CACHE INTERNAL "boost include directory" FORCE
    )
endif ()

