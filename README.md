# DeskDashboard 🖥️

桌面小屏 Dashboard — 用 Qt6 构建

## 功能

| 模块 | 说明 |
|------|------|
| 📋 **周计划** | 艾森豪威尔矩阵，四象限拖拽管理任务 |
| ⏰ **每日时间表** | 时间轴式显示今日计划，勾选完成状态 |
| 🤖 **DeepSeek API 额度** | 实时查询 API 余额，进度条可视化 |
| 🫀 **心电监测** | 流式显示 ECG 波形，支持开始/暂停，自动计算 BPM |

## 开发环境

| | Windows | Raspberry Pi |
|---|---|---|
| 系统 | Windows 10/11 | Raspberry Pi OS (ARM64) |
| Qt | Qt 6.x (MSVC/MinGW) | Qt 6.x |
| 编译器 | MSVC 2022 / MinGW | GCC (aarch64) |
| 构建 | CMake 3.16+ | CMake 3.16+ |

## 构建

```bash
# 1. 克隆
git clone <repo-url> desk-dashboard
cd desk-dashboard

# 2. 配置
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 3. 构建
cmake --build build --parallel

# 4. 运行（默认配置）
./build/DeskDashboard

# 使用自定义配置
./build/DeskDashboard --config /path/to/config.json
```

## 项目结构

```
desk-dashboard/
├── CMakeLists.txt
├── config/
│   └── config.json           # API Key、数据路径等配置
├── src/
│   ├── main.cpp               # 入口
│   ├── mainwindow.h/cpp       # 主窗口布局
│   ├── widgets/
│   │   ├── eisenhowermatrix   # 艾森豪威尔矩阵
│   │   ├── dailytimetable     # 每日时间表
│   │   ├── apiquotawidget     # API 额度
│   │   └── ecgwidget          # 心电波形显示
│   ├── core/
│   │   ├── taskmodel          # 任务数据模型
│   │   ├── dailyschedule      # 日程数据模型
│   │   └── configmanager      # 配置管理
│   └── ecg/
│       ├── ecgdatasource      # ECG 数据源接口
│       ├── ecgsimulator       # 模拟 ECG（Windows 开发用）
│       └── ecgadc             # ADC SPI 读取（树莓派）
└── resources/
    └── resources.qrc
```

## 开发流程

1. **Windows 上开发** → 用 `EcgSimulator` 模拟心电数据
2. **Git 提交代码** → 推送到仓库
3. **树莓派上部署** → `git pull → cmake -B build → cmake --build build`
4. **树莓派上运行** → ADC 模式自动启用（`EcgAdc`）

## 配置

编辑 `config/config.json`：

```json
{
    "deepseek": { "apiKey": "sk-xxx" },
    "ecg": { "spiChannel": 0, "sampleRate": 250 }
}
```
