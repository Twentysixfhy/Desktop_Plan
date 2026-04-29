cmake_minimum_required(VERSION 3.16)
project(DeskDashboard VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ─── Qt6 依赖 ──────────────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network          # DeepSeek API 请求
    Charts           # (可选) 心电曲线图
)

# ─── 平台特定配置 ───────────────────────────────────────────
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    # 树莓派 ARM64 — 启用 ADC 真实数据源
    add_compile_definitions(PLATFORM_RASPBERRY_PI)
    message(STATUS "Building for Raspberry Pi (ARM64) — ADC source enabled")

    # 可选：如果树莓派上要用 wiringPi 或 pigpio 读 SPI
    # find_package(PkgConfig)
    # pkg_check_modules(PIGPIO REQUIRED pigpio)
elseif(WIN32)
    message(STATUS "Building for Windows — simulator source only")
endif()

# ─── 源文件 ─────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/widgets/eisenhowermatrix.cpp
    src/widgets/dailytimetable.cpp
    src/widgets/apiquotawidget.cpp
    src/widgets/ecgwidget.cpp
    src/core/taskmodel.cpp
    src/core/dailyschedule.cpp
    src/core/configmanager.cpp
    src/ecg/ecgsimulator.cpp
)

set(HEADERS
    src/mainwindow.h
    src/widgets/eisenhowermatrix.h
    src/widgets/dailytimetable.h
    src/widgets/apiquotawidget.h
    src/widgets/ecgwidget.h
    src/core/taskmodel.h
    src/core/dailyschedule.h
    src/core/configmanager.h
    src/ecg/ecgdatasource.h
    src/ecg/ecgsimulator.h
    src/ecg/ecgadc.h
)

# ─── 资源文件 ───────────────────────────────────────────────
set(RESOURCES
    resources/resources.qrc
)

# ─── 编译目标 ───────────────────────────────────────────────
qt_add_executable(DeskDashboard
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

target_include_directories(DeskDashboard PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(DeskDashboard PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64" AND PIGPIO_FOUND)
    target_link_libraries(DeskDashboard PRIVATE ${PIGPIO_LIBRARIES})
    target_include_directories(DeskDashboard PRIVATE ${PIGPIO_INCLUDE_DIRS})
endif()

# ─── 安装规则 ───────────────────────────────────────────────
install(TARGETS DeskDashboard
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
