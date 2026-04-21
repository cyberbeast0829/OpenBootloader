# blcan.py 使用说明

本文档介绍如何用 blcan.py 通过 PCAN USB 测试 OpenBootloader 的 CAN 升级协议。

当前分支目标 MCU 为 N32H473，因此文中的默认参数以 N32H473 为准。脚本也保留了 HPM6E6Y 的 MCU profile，可通过 `--mcu hpm6e6y` 切换。

## 1. 脚本位置

- 脚本: tools/blcan.py

## 2. 运行环境

- 操作系统: Windows
- Python: 建议使用项目内虚拟环境
- 依赖: python-can
- CAN 参数:
  - 接口: pcan
  - 通道: PCAN_USBBUS1
  - 波特率: 500000
  - SYNC ID: 默认随 MCU profile 变化，当前 N32H473 为 0x128

## 3. MCU Profile

- 默认 profile: `n32h473`
- `n32h473` 默认参数:
  - `--sync-id 0x128`
  - `--flash-base 0x08000000`
  - `--page-size 0x2000`
- `hpm6e6y` 默认参数:
  - `--sync-id 0x128`
  - `--flash-base 0x80000000`
  - `--page-size 0x1000`

显式切换到 HPM6E6Y:

```bash
.venv/Scripts/python.exe tools/blcan.py --mcu hpm6e6y read --addr 0x80008000 --len 64
```

如果 profile 的默认值不适合当前固件，也可以继续手工覆盖 `--sync-id`、`--flash-base`、`--page-size`。

## 4. 基本命令格式

在项目根目录执行:

```bash
.venv/Scripts/python.exe tools/blcan.py [全局参数] <子命令> [子命令参数]
```

查看帮助:

```bash
.venv/Scripts/python.exe tools/blcan.py --help
.venv/Scripts/python.exe tools/blcan.py upgrade --help
```

## 5. 全局参数

- --mcu: MCU profile，默认 n32h473
- --interface: CAN 接口，默认 pcan
- --channel: 通道，默认 PCAN_USBBUS1
- --bitrate: 波特率，默认 500000
- --timeout: 普通收包超时(秒)，默认 1.0
- --sync-id: 同步帧 ID，默认取决于 --mcu
- --verbose: 打开详细日志

示例:

```bash
.venv/Scripts/python.exe tools/blcan.py --verbose read --addr 0x08008000 --len 64
```

## 6. 关于握手

- 命令 `sync` 专门用于握手，不需要额外参数。
- 其他命令默认不握手；如果需要在该命令前先握手，可加 `--sync`。
- 推荐用法:
  1. 先执行一次 `sync` 建立通信。
  2. 后续命令直接执行，不重复握手。

握手命令:

```bash
.venv/Scripts/python.exe tools/blcan.py sync
```

在业务命令前显式握手:

```bash
.venv/Scripts/python.exe tools/blcan.py read --addr 0x08008000 --len 64 --sync
```

## 7. 常用命令

### 7.1 读内存

```bash
.venv/Scripts/python.exe tools/blcan.py read --addr 0x08008000 --len 64
```

保存到文件:

```bash
.venv/Scripts/python.exe tools/blcan.py read --addr 0x08008000 --len 64 --out readback.bin
```

### 7.2 写内存

十六进制写入:

```bash
.venv/Scripts/python.exe tools/blcan.py write --addr 0x08008000 --data DEADBEEF
```

从文件写入:

```bash
.venv/Scripts/python.exe tools/blcan.py write --addr 0x08008000 --file payload.bin
```

### 7.3 擦除

整片擦除:

```bash
.venv/Scripts/python.exe tools/blcan.py erase --mode mass --erase-timeout 120
```

按页擦除:

```bash
.venv/Scripts/python.exe tools/blcan.py erase --mode pages --pages 4,5,6,7 --erase-timeout 60
```

### 7.4 跳转执行

```bash
.venv/Scripts/python.exe tools/blcan.py go --addr 0x08008000
```

## 8. 完整升级

N32H473 示例:

```bash
.venv/Scripts/python.exe tools/blcan.py upgrade \
  --firmware MW_IAP_bootloader.bin \
  --erase auto \
  --flash-base 0x08000000 \
  --addr 0x08008000 \
  --page-size 0x2000 \
  --chunk-size 256 \
  --erase-timeout 120 \
  --verify \
  --go-addr 0x08008000
```

如果直接使用默认 N32H473 profile，也可以省略 `--flash-base` 和 `--page-size`:

```bash
.venv/Scripts/python.exe tools/blcan.py upgrade \
  --firmware MW_IAP_bootloader.bin \
  --erase auto \
  --addr 0x08008000 \
  --chunk-size 256 \
  --erase-timeout 120 \
  --verify \
  --go-addr 0x08008000
```

HPM6E6Y 等价升级示例:

```bash
.venv/Scripts/python.exe tools/blcan.py --mcu hpm6e6y upgrade \
  --firmware MW_IAP_bootloader.bin \
  --erase auto \
  --addr 0x80008000 \
  --chunk-size 256 \
  --erase-timeout 120 \
  --verify \
  --go-addr 0x80008000
```

## 9. 返回码

- 0: 成功
- 1: 未知异常
- 2: Bootloader 协议错误
- 3: CAN 驱动错误
- 4: 文件不存在

## 10. 常见问题

Q: 为什么有时会 NACK at sync?
A: 设备不在可握手窗口，或当前状态不接受再次握手。可重启设备后再执行 sync。

Q: 出现 timeout waiting for CAN frame 怎么办?
A:
1. 检查 PCAN 通道是否被其他程序占用。
2. 确认波特率、通道配置是否正确。
3. 确认 `--mcu`、`--sync-id`、地址范围与当前固件匹配。
4. 必要时在命令上加 `--sync` 或先手动执行 `sync`。
