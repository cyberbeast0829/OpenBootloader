# OpenBootloader

守护兽（CyberBeast）N32H473 平台的在应用编程（IAP）Bootloader，基于 STMicroelectronics OpenBootloader 中间件移植、裁剪而来，用于通过 CAN 总线完成固件升级。

当前固件只启用 CAN 接口：上位机先发送同步字节完成握手，然后通过一组以 CAN ID 编码的命令完成擦除、写入、读回校验和跳转执行。

## 主要特性

- 基于 OpenBootloader 中间件，命令层与内存/外设接口解耦，便于移植
- Classic CAN 通信（单帧 8 字节），500 Kbps，标准 11 位 ID
- 支持整片擦除、按页擦除、内存读写、跳转执行
- 支持读保护、写保护及解除保护命令（经由 Flash Option Bytes 生效）
- 上电后等待 200 ms，无上位机接入则自动跳转到 APP
- APP 可通过共享 RAM 中的 magic value 主动请求停留在 Bootloader
- 持续刷新 IWDG，避免 APP 软复位进入 Bootloader 后看门狗超时
- 提供 Python 上位机工具 `tools/blcan.py`，支持 CANable（slcan）与 PCAN

## 目标硬件

| 项目 | 参数 |
|------|------|
| MCU | N32H473CE（Cortex-M4） |
| Flash | 512 KB，64 页 × 8 KB |
| Bootloader 可用 SRAM | 144 KB（`0x20000000` – `0x20023FFF`） |
| Bootloader 区域 | `0x08000000` – `0x08007FFF`（32 KB） |
| APP 区域 | `0x08008000` – `0x0807FFFF`（480 KB） |
| 共享请求标志 | `0x20023FF0`（UNINIT RAM，启动代码不初始化，软复位后仍保留） |

Flash 布局由 [LION_H473_Bootloader.sct](MDK-ARM/LION_H473_Bootloader.sct) 限制：Bootloader 链接区域固定为 32 KB，超出会直接链接失败，避免覆盖 APP。

## CAN 通信参数

| 参数 | 值 |
|------|-----|
| 外设 | FDCAN2 |
| TX / RX 引脚 | `PB6` / `PB7` |
| 帧格式 | Classic CAN，8 字节/帧 |
| 波特率 | 500 Kbps |
| 握手 ID | `0x128`（标准帧） |
| 握手字节 | `0x7F`（SYNC），应答 `0x79`（ACK） |
| 否定应答 | `0x1F`（NACK） |

> 当前分支 `LION_H473_PB67` 使用 `PB6/PB7`。仓库中另有 `LION_H473_PC1011` 分支，对应 `PC10/PC11` 引脚配置。

命令码放在 CAN 帧的 ID 字段，参数和数据放在 8 字节数据段中；地址统一为 4 字节大端序。完整协议见 [CAN_Bootloader_Protocol.md](CAN_Bootloader_Protocol.md)。

### 支持的命令

| 命令 | Opcode (CAN ID) | 功能 |
|------|-----------------|------|
| Get Command | `0x00` | 获取支持的命令列表 |
| Get Version | `0x01` | 获取协议版本 |
| Get ID | `0x02` | 获取设备 ID |
| Read Memory | `0x11` | 读取内存 |
| Go | `0x21` | 跳转到指定地址执行 |
| Write Memory | `0x31` | 写入内存（单次最多 256 字节） |
| Erase Memory | `0x44` | 整片擦除或按页擦除 |
| Write Protect | `0x63` | 启用写保护 |
| Write Unprotect | `0x73` | 解除写保护 |
| Read Protect | `0x82` | 启用读保护 |
| Read Unprotect | `0x92` | 解除读保护 |

## 启动流程

1. 上电或复位后初始化时钟、GPIO 和 FDCAN。
2. 在 200 ms 窗口内检测握手/命令；收到 SYNC 后进入命令处理循环。
3. 若窗口内没有上位机接入，且 APP 未请求驻留，则跳转到 `0x08008000`。
4. APP 在 `0x20023FF0` 写入 `0xB00710AD` 后软复位，可强制进入并停留在 Bootloader。
5. Bootloader 自身不启动 IWDG，但会持续喂狗，以兼容 APP 启动后已开启的看门狗。

## 编译

开发环境：Keil MDK-ARM（ARMCC 5.06u7）+ Nationstech N32H47x DFP 1.0.0。

1. 打开 [MDK-ARM/OpenBootloader.uvprojx](MDK-ARM/OpenBootloader.uvprojx)。
2. 选择 `N32H473` target。
3. 执行 `Rebuild`。
4. 构建产物位于 `MDK-ARM/N32H473/`：
   - `LION_H473_Bootloader.axf`
   - `LION_H473_Bootloader.hex`

APP 工程必须链接到 `0x08008000`，并与 Bootloader 的 32 KB 区域保持隔离。

## 上位机工具

`tools/blcan.py` 基于 python-can，默认使用 CANable（slcan），也可切换 PCAN。

```powershell
python -m venv .venv
.venv\Scripts\python.exe -m pip install python-can pyserial
```

握手：

```powershell
.venv\Scripts\python.exe tools\blcan.py sync
```

读取内存：

```powershell
.venv\Scripts\python.exe tools\blcan.py read --addr 0x08008000 --len 64
```

完整升级：

```powershell
.venv\Scripts\python.exe tools\blcan.py upgrade `
  --firmware <your_firmware.bin> `
  --erase auto `
  --addr 0x08008000 `
  --chunk-size 256 `
  --erase-timeout 120 `
  --verify `
  --go-addr 0x08008000
```

使用 PCAN 时追加 `--interface pcan`。更多参数与故障排查见 [tools/blcan_使用说明.md](tools/blcan_使用说明.md)。

## 目录结构

```text
OpenBootloader/
├── Core/                       # 启动、时钟、中断与 main
├── OpenBootloader/
│   ├── App/                    # Bootloader 应用入口与接口注册
│   └── Target/                 # Flash/RAM/OB/OTP、FDCAN、USART、IWDG 接口
├── Middlewares/N32/OpenBootloader/
│   ├── Core/                   # OpenBootloader 核心（命令分发、内存管理）
│   └── Modules/                # FDCAN/USART 命令实现、Mem 模块
├── Drivers/                    # CMSIS 与 N32H47x 标准外设库
├── MDK-ARM/                    # Keil 工程、scatter 文件与构建输出
├── segger_embedded_studio/     # SEGGER Embedded Studio 调试配置
├── tools/                      # blcan.py 上位机工具及说明
├── CAN_Bootloader_Protocol.md  # CAN 升级协议
└── 串口协议.md                  # 参考：VESC 串口协议与 OTA 流程
```

## 相关文档

- [CAN 升级协议](CAN_Bootloader_Protocol.md)
- [blcan.py 使用说明](tools/blcan_使用说明.md)
- [串口协议参考](串口协议.md)（VESC 协议，非当前 Bootloader 的 CAN 传输实现）

## 许可证

本项目原创部分采用 [MIT License](LICENSE)，Copyright (c) 2026 守护兽（CyberBeast）。

仓库中源自第三方的组件仍遵循其各自许可证：

- STMicroelectronics OpenBootloader 相关文件：Copyright (c) 2019 STMicroelectronics
- Nationstech N32 外设驱动与 CMSIS 文件

再分发或修改这些文件时，请保留原始版权声明与许可证信息。
