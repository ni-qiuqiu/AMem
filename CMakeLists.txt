cmake_minimum_required(VERSION 3.16)

# 项目版本定义
# ==================== 项目版本定义 ====================
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

# 协议版本（与服务端协商使用）
set(PROTOCOL_VERSION_MAJOR 1)
set(PROTOCOL_VERSION_MINOR 1)
set(PROTOCOL_VERSION_PATCH 0)
set(PROTOCOL_VERSION "${PROTOCOL_VERSION_MAJOR}.${PROTOCOL_VERSION_MINOR}.${PROTOCOL_VERSION_PATCH}")


project(ImGuiProject VERSION ${PROJECT_VERSION})
set(CMAKE_CXX_STANDARD 17)

# 渲染后端选项
option(USE_DX11 "Use DirectX 11 backend" OFF)
option(USE_DX12 "Use DirectX 12 backend" ON)

# 生成版本信息头文件
string(TIMESTAMP BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP BUILD_TIME "%H:%M:%S")

find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
else()
    set(GIT_COMMIT_HASH "unknown")
    set(GIT_BRANCH "unknown")
endif()

configure_file(${CMAKE_SOURCE_DIR}/version.h.in ${CMAKE_BINARY_DIR}/generated/version.h @ONLY)


# 输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

# 查找 Capstone 库
set(capstone_ROOT "C:/Program Files/capstone" CACHE PATH "Capstone installation directory")
find_package(capstone CONFIG QUIET)

if(capstone_FOUND)
    find_library(CAPSTONE_LIBRARY NAMES capstone capstone_static
        HINTS "${capstone_ROOT}/lib" "${capstone_ROOT}/build" "${capstone_ROOT}" PATH_SUFFIXES lib lib64)
    find_path(CAPSTONE_INCLUDE_DIR NAMES capstone/capstone.h
        HINTS "${capstone_ROOT}/include" "${capstone_ROOT}" PATH_SUFFIXES include)
else()
    find_path(CAPSTONE_INCLUDE_DIR capstone/capstone.h)
    find_library(CAPSTONE_LIBRARY NAMES capstone)
endif()

if(CAPSTONE_INCLUDE_DIR AND CAPSTONE_LIBRARY)
    set(HAVE_CAPSTONE TRUE)
    set(CAPSTONE_LIBRARIES ${CAPSTONE_LIBRARY})
    set(CAPSTONE_INCLUDE_DIRS ${CAPSTONE_INCLUDE_DIR})
else()
    set(HAVE_CAPSTONE FALSE)
    message(WARNING "Capstone not found! Disassembly features will be disabled.")
endif()

# 查找 Keystone 库
set(keystone_ROOT "C:/vcpkg/installed/x64-windows" CACHE PATH "Keystone installation directory")

find_path(KEYSTONE_INCLUDE_DIR keystone/keystone.h
    HINTS "${keystone_ROOT}/include" "${keystone_ROOT}" PATH_SUFFIXES include)
find_library(KEYSTONE_LIBRARY NAMES keystone
    HINTS "${keystone_ROOT}/lib" "${keystone_ROOT}" PATH_SUFFIXES lib lib64)

if(KEYSTONE_INCLUDE_DIR AND KEYSTONE_LIBRARY)
    set(HAVE_KEYSTONE TRUE)
    set(KEYSTONE_LIBRARIES ${KEYSTONE_LIBRARY})
    set(KEYSTONE_INCLUDE_DIRS ${KEYSTONE_INCLUDE_DIR})
else()
    set(HAVE_KEYSTONE FALSE)
    message(WARNING "Keystone not found! Assembly features will be disabled.")
endif()

set(IMGUI_SOURCES
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_demo.cpp
    imgui/backends/imgui_impl_win32.cpp
)

set(GUI_SOURCES
    gui/Gui.cpp
    gui/Window.cpp
    gui/CEWindow.cpp
    gui/ServerConnectWindow.cpp
    gui/ModulesWindow.cpp
    gui/ScanWindow.cpp
    gui/scan/ScanPanel.cpp
    gui/scan/ScanResultsPanel.cpp
    gui/scan/AddressListPanel.cpp
    gui/scan/ScanEngine.cpp
    gui/MemoryViewerWindow.cpp
    gui/memview/HexEditorPanel.cpp
    gui/memview/StructAnalyzerPanel.cpp
    gui/memview/WatchListPanel.cpp
    gui/memview/DisassemblyPanel.cpp
    gui/BreakpointWindow.cpp
    gui/breakpoint/BreakpointListPanel.cpp
    gui/breakpoint/BreakpointDetailPanel.cpp
    gui/ProcessListWindow.cpp
    gui/VersionWindow.cpp
    gui/DisassemblyHelper.cpp
    gui/AssemblyHelper.cpp
    gui/ConfigManager.cpp
    gui/LogWindow.cpp
    gui/AppContext.cpp
    gui/ValueFormatter.cpp
)

set(LUA_SOURCES
    lua/LuaEngine.cpp
    lua/LuaAPI.cpp
    lua/LuaAPI_Memory.cpp
    lua/LuaAPI_ImGui.cpp
    lua/LuaAPI_Assembly.cpp
)

# 查找 LuaJIT 库
option(LUAJIT_STATIC "Link LuaJIT statically" ON)

set(LUAJIT_DIR "${CMAKE_SOURCE_DIR}/third_party/LuaJIT")
set(LUAJIT_INCLUDE "${LUAJIT_DIR}/include")
set(LUAJIT_LIB "${LUAJIT_DIR}/lib")
set(LUAJIT_BIN "${LUAJIT_DIR}/bin")

if((EXISTS "${LUAJIT_INCLUDE}/lua.hpp" OR EXISTS "${LUAJIT_INCLUDE}/luajit.h" OR EXISTS "${LUAJIT_INCLUDE}/lua.h")
   AND EXISTS "${LUAJIT_LIB}/lua51.lib")
    set(HAVE_LUAJIT TRUE)
    set(LUAJIT_LIBRARY "${LUAJIT_LIB}/lua51.lib")
    set(LUAJIT_IS_STATIC ${LUAJIT_STATIC})
else()
    message(FATAL_ERROR "LuaJIT not found in third_party/LuaJIT directory!")
    set(HAVE_LUAJIT FALSE)
endif()

if(HAVE_LUAJIT)
    list(APPEND GUI_SOURCES gui/LuaScriptWindow.cpp gui/LuaImGuiWindow.cpp)
endif()

set(SOCKET_SOURCES
    socket/client_singleton.cpp
    socket/ProcessCommands.cpp
    socket/MemoryCommands.cpp
    socket/ScanCommands.cpp
    socket/BreakpointCommands.cpp
    socket/FreezeCommands.cpp
    socket/SymbolCommands.cpp
)

set(RENDERER_SOURCES
    renderer/DX12Renderer.cpp
    renderer/StyleSetup.cpp
)

set(IPC_SOURCES
    ipc/IpcServer.cpp
)


# 配置 DirectX 后端
if(USE_DX11)
    add_definitions(-DUSE_DX11)
    list(APPEND IMGUI_SOURCES imgui/backends/imgui_impl_dx11.cpp)
    set(DX_LIBS d3d11 dxgi d3dcompiler)
elseif(USE_DX12)
    add_definitions(-DUSE_DX12)
    list(APPEND IMGUI_SOURCES imgui/backends/imgui_impl_dx12.cpp)
    set(DX_LIBS d3d12 dxgi d3dcompiler ws2_32)
endif()

add_definitions(-DIMGUI_DISABLE_DEBUG_TOOLS)

if(MSVC)
    add_compile_options(/utf-8)
endif()

add_executable(ImGuiProject WIN32
    main.cpp
    ${IMGUI_SOURCES}
    ${GUI_SOURCES}
    ${SOCKET_SOURCES}
    ${RENDERER_SOURCES}
    ${IPC_SOURCES}
    ${LUA_SOURCES}
)

# 包含目录
target_include_directories(ImGuiProject PRIVATE
    imgui imgui/backends gui lua renderer utils ipc
    ${CMAKE_SOURCE_DIR}/third_party
    ${CMAKE_BINARY_DIR}/generated
)

if(HAVE_CAPSTONE)
    target_include_directories(ImGuiProject PRIVATE ${CAPSTONE_INCLUDE_DIRS})
endif()

if(HAVE_KEYSTONE)
    target_include_directories(ImGuiProject PRIVATE ${KEYSTONE_INCLUDE_DIRS})
endif()

if(HAVE_LUAJIT)
    target_include_directories(ImGuiProject PRIVATE ${LUAJIT_INCLUDE})
endif()

# 链接系统库
target_link_libraries(ImGuiProject ${DX_LIBS})

# 链接 LuaJIT 库
if(HAVE_LUAJIT)
    get_filename_component(LUAJIT_LIBRARY_ABS ${LUAJIT_LIBRARY} ABSOLUTE)
    
    target_link_libraries(ImGuiProject ${LUAJIT_LIBRARY_ABS})
    target_compile_definitions(ImGuiProject PRIVATE HAVE_LUAJIT)
    
    if(LUAJIT_IS_STATIC)
        target_compile_definitions(ImGuiProject PRIVATE LUAJIT_STATIC)
    elseif(EXISTS "${LUAJIT_BIN}/lua51.dll")
        add_custom_command(TARGET ImGuiProject POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${LUAJIT_BIN}/lua51.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lua51.dll")
    endif()
endif()

# 链接 Capstone 库
if(HAVE_CAPSTONE)
    target_link_libraries(ImGuiProject ${CAPSTONE_LIBRARIES})
    target_compile_definitions(ImGuiProject PRIVATE HAVE_CAPSTONE)

    # 拷贝 capstone.dll 到输出目录
    get_filename_component(_capstone_lib_dir ${CAPSTONE_LIBRARY} DIRECTORY)
    get_filename_component(_capstone_bin_dir "${_capstone_lib_dir}/../bin" ABSOLUTE)
    if(EXISTS "${_capstone_bin_dir}/capstone.dll")
        add_custom_command(TARGET ImGuiProject POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${_capstone_bin_dir}/capstone.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/capstone.dll")
    endif()
endif()

# 链接 Keystone 库
if(HAVE_KEYSTONE)
    target_link_libraries(ImGuiProject ${KEYSTONE_LIBRARIES})
    target_compile_definitions(ImGuiProject PRIVATE HAVE_KEYSTONE)

    # 拷贝 keystone.dll 到输出目录
    get_filename_component(_keystone_lib_dir ${KEYSTONE_LIBRARY} DIRECTORY)
    get_filename_component(_keystone_bin_dir "${_keystone_lib_dir}/../bin" ABSOLUTE)
    if(EXISTS "${_keystone_bin_dir}/keystone.dll")
        add_custom_command(TARGET ImGuiProject POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${_keystone_bin_dir}/keystone.dll" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/keystone.dll")
    endif()
endif()
