
set(BOOST_BUILD_COMPONENTS filesystem system program_options thread)
unset(BUILD_WITH_LIBRARIES)
string(REPLACE ";" "," BUILD_WITH_LIBRARIES "${BOOST_BUILD_COMPONENTS}")

if (SPV_FOR_IOS)
	set(Boost_INCLUDE_DIRS /Users/abilican/Documents/projects/perso/elastos/twilio-boost-build/src/boost/1.67.0)
else()
	set(BOOST_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/boost/build)
	set(BOOST_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/boost/install)
	set(BOOST_EXTRACT_DIR ${CMAKE_CURRENT_BINARY_DIR}/boost)
endif()

if(SPV_FOR_ANDROID)
	set(
		BUILD_COMMAND
		./build.sh
		--verbose
		--with-libraries=${BUILD_WITH_LIBRARIES}
		--build-dir=${BOOST_BUILD_DIR}
		--prefix=${BOOST_INSTALL_DIR}
		--extract-dir=${BOOST_EXTRACT_DIR}
		--arch=${CMAKE_ANDROID_ARCH_ABI}
		--ndk-root=$ENV{ANDROID_NDK}
		--ndk-api-level=${CMAKE_SYSTEM_VERSION}
	)
	set(Boost_INCLUDE_DIRS ${BOOST_INSTALL_DIR}/${CMAKE_ANDROID_ARCH_ABI}/include)
elseif(SPV_FOR_IOS)
	MESSAGE ("Nothing to do!")
else()
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
endif()

unset(COMPONENTS_LIBRARY_PATH)
unset(Boost_LIBRARIES CACHE)
foreach(COMPONENT_NAME ${BOOST_BUILD_COMPONENTS})
	if(SPV_FOR_ANDROID)
		set(COMPONENTS_LIBRARY_PATH ${COMPONENTS_LIBRARY_PATH} ${BOOST_INSTALL_DIR}/${CMAKE_ANDROID_ARCH_ABI}/lib/libboost_${COMPONENT_NAME}.a)
	elseif(SPV_FOR_IOS)
		set(COMPONENTS_LIBRARY_PATH ${COMPONENTS_LIBRARY_PATH} /Users/abilican/Documents/projects/perso/elastos/twilio-boost-build/target/outputs/boost/1.67.0/ios/lib/release/arm64-v8a/libboost_${COMPONENT_NAME}.a)
	else()
		set(COMPONENTS_LIBRARY_PATH ${COMPONENTS_LIBRARY_PATH} ${BOOST_INSTALL_DIR}/lib/libboost_${COMPONENT_NAME}.a)
	endif()
	set(Boost_LIBRARIES boost_${COMPONENT_NAME};${Boost_LIBRARIES})
endforeach()

if(!SPV_FOR_IOS)
	add_custom_command(
		COMMENT "Building boost..."
		OUTPUT ${COMPONENTS_LIBRARY_PATH}
		COMMAND ${BUILD_COMMAND}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Boost-for-Android
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Boost-for-Android/build.sh
	)
endif()

add_custom_target(build_boost ALL DEPENDS ${COMPONENTS_LIBRARY_PATH})

foreach(COMPONENT_NAME ${BOOST_BUILD_COMPONENTS})
	add_library(boost_${COMPONENT_NAME} STATIC IMPORTED GLOBAL)
	if(SPV_FOR_ANDROID)
		set(COMPONENT_LIBRARY_PATH ${BOOST_INSTALL_DIR}/${CMAKE_ANDROID_ARCH_ABI}/lib/libboost_${COMPONENT_NAME}.a)
	elseif(SPV_FOR_IOS)
		set(COMPONENT_LIBRARY_PATH /Users/abilican/Documents/projects/perso/elastos/twilio-boost-build/target/outputs/boost/1.67.0/ios/lib/release/arm64-v8a/libboost_${COMPONENT_NAME}.a)
	else()
		set(COMPONENT_LIBRARY_PATH ${BOOST_INSTALL_DIR}/lib/libboost_${COMPONENT_NAME}.a)
	endif()
	set_target_properties(boost_${COMPONENT_NAME} PROPERTIES IMPORTED_LOCATION ${COMPONENT_LIBRARY_PATH})
	add_dependencies(boost_${COMPONENT_NAME} build_boost)
endforeach()


set(
	Boost_LIBRARIES
	${Boost_LIBRARIES}
	CACHE INTERNAL "boost libraries" FORCE
)

set(
	ThirdParty_BOOST_INC_DIR
	${Boost_INCLUDE_DIRS}
	CACHE INTERNAL "boost include directory" FORCE
)
