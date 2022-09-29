# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.10.2)

#set(QXR_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../)
set(QXR_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(QXR_COMPOSITOR_DIR ${QXR_ROOT_PATH}/)

set(XR_LOADER_DISTRIBUTION_DIR ${QXR_COMPOSITOR_DIR})
#add_library(lib_xr_loader SHARED IMPORTED)
#set_target_properties(lib_xr_loader PROPERTIES IMPORTED_LOCATION
#        ${XR_LOADER_DISTRIBUTION_DIR}/src/loader/build/intermediates/cmake/debug/obj/${ANDROID_ABI}/libopenxr_loader.so)

# Declares and names the project.



project("overlaysession")

set(XR_LOADER_DISTRIBUTION_DIR ${QXR_COMPOSITOR_DIR}/)
find_package(loader REQUIRED CONFIG)


set(${CMAKE_C_FLAGS}, "${CMAKE_C_FLAGS}")
add_library(native_app_glue STATIC
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
target_include_directories(native_app_glue PUBLIC ${ANDROID_NDK}/sources/android/native_app_glue)

# now build app's shared lib
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall")

# common-log
set(COMMON_LOG_SOURCE_DIR ${QXR_COMPOSITOR_DIR}/Common/Log/)
add_library(qxr-common-log STATIC ${COMMON_LOG_SOURCE_DIR}/LogUtils.c)
target_include_directories(qxr-common-log PUBLIC ${COMMON_LOG_SOURCE_DIR})

# thirdparty-openxr
set(THIRDPARTY_OPENXR_SOURCE_DIR
        ${QXR_COMPOSITOR_DIR}/ThirdParty/OpenXR-SDK/include
        ${QXR_COMPOSITOR_DIR}/ThirdParty/OpenXR-SDK/src
        ${QXR_COMPOSITOR_DIR}/ThirdParty/OpenXR-SDK/src/common)
add_library(qxr-thirdparty-openxr INTERFACE)
target_include_directories(qxr-thirdparty-openxr
        INTERFACE ${THIRDPARTY_OPENXR_SOURCE_DIR})

# thirdparty-glm module
set(GLM_SOURCE_DIR ${QXR_COMPOSITOR_DIR}/ThirdParty/glm/)
add_library(qxr-thirdparty-glm INTERFACE)
target_include_directories(qxr-thirdparty-glm INTERFACE ${GLM_SOURCE_DIR})

# thirdparty-tinyobj module
set(THIRDPARTY_TINYOBJ_SOURCE_DIR
        ${QXR_COMPOSITOR_DIR}/ThirdParty/tinyobjloader/)
set(THIRDPARTY_TINYOBJ_SOURCE_FILES
        ${THIRDPARTY_TINYOBJ_SOURCE_DIR}/tiny_obj_loader.cc)
add_library(qxr-thirdparty-tinyobj STATIC ${THIRDPARTY_TINYOBJ_SOURCE_FILES})
target_include_directories(qxr-thirdparty-tinyobj
        INTERFACE ${THIRDPARTY_TINYOBJ_SOURCE_DIR})

# common-data-structures
set(COMMON_DATA_STRUCTURES_SOURCE_DIR
        ${QXR_COMPOSITOR_DIR}/Common/DataStructures/cpp)
add_library(qxr-common-data-structures INTERFACE)
target_include_directories(qxr-common-data-structures
        INTERFACE ${COMMON_DATA_STRUCTURES_SOURCE_DIR})

# common-gl
set(COMMON_GL_SOURCE_DIR ${QXR_COMPOSITOR_DIR}/Common/GL/cpp)
file(GLOB COMMON_GL_SOURCE_FILES ${COMMON_GL_SOURCE_DIR}/*.cpp)
add_library(qxr-common-gl STATIC ${COMMON_GL_SOURCE_FILES})
target_include_directories(qxr-common-gl PUBLIC ${COMMON_GL_SOURCE_DIR}/)
target_link_libraries(qxr-common-gl PRIVATE
        log
        qxr-common-log
        qxr-thirdparty-glm
        qxr-thirdparty-tinyobj
        qxr-common-data-structures)

# app-common module
set(APPCOMMON_SOURCE_DIR ${QXR_ROOT_PATH}/AppCommon/cpp)
add_library(qxr-app-common STATIC ${APPCOMMON_SOURCE_DIR}/AppCommon.cpp)
target_include_directories(qxr-app-common PUBLIC
        ${APPCOMMON_SOURCE_DIR}/)
target_link_libraries(qxr-app-common PRIVATE
        EGL
        native_app_glue
        qxr-common-log
#        qxr-thirdparty-openxr
        loader::openxr_loader
        qxr-thirdparty-glm
        )


# thirdparty-imgui
set(THIRDPARTY_IMGUI_SOURCE_FILES
#        ${QXR_COMPOSITOR_DIR}/XrCompositor/compositor/src/main/cpp/src/imgui_impl_opengl3.cpp
        ${QXR_COMPOSITOR_DIR}/ThirdParty/imgui/backends/imgui_impl_opengl3.cpp
        ${QXR_COMPOSITOR_DIR}/ThirdParty/imgui/backends/imgui_impl_android.cpp
        ${QXR_COMPOSITOR_DIR}/ThirdParty/imgui/imgui.cpp
        ${QXR_COMPOSITOR_DIR}/ThirdParty/imgui/imgui_draw.cpp
        ${QXR_COMPOSITOR_DIR}/ThirdParty/imgui/imgui_widgets.cpp
        ${QXR_COMPOSITOR_DIR}/ThirdParty/imgui/imgui_tables.cpp)
add_library(qxr-thirdparty-imgui STATIC ${THIRDPARTY_IMGUI_SOURCE_FILES})
target_include_directories(qxr-thirdparty-imgui
        PUBLIC
        ${QXR_COMPOSITOR_DIR}/ThirdParty/imgui
        ${QXR_COMPOSITOR_DIR}/XrCompositor/compositor/src/main/cpp/src)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             native-lib.cpp
             openxr_overlay_app.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib}
                        android
                       native_app_glue
                       EGL
                        GLESv3
#                        lib_xr_loader
        loader::openxr_loader
                        qxr-app-common
                        qxr-common-gl
                        qxr-common-data-structures
                        qxr-thirdparty-tinyobj
                        qxr-thirdparty-glm
#                        qxr-thirdparty-openxr
                        qxr-thirdparty-imgui)