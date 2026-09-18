# ESE5190 Lab 2 开发环境

使用 **VS Code + PlatformIO + AVR-GCC**，目标板为 **ATmega328PB Xplained Mini**。代码使用寄存器级 C，未启用 Arduino 框架。当前 `src/main.c` 是 Part B（C1）的可编译版本，提交文件保存在仓库根目录的 `lab2c1.c`。

## 在 VS Code 中使用

1. 用 **File → Open Folder** 打开本目录，确保资源管理器中能看到 `platformio.ini`。
2. 等待 PlatformIO 初始化。侧栏打开 PlatformIO，展开 **PROJECT TASKS → xplained328pb → General**。
3. 点击 **Build** 编译。编译不需要连接开发板，成功后生成 `.pio/build/xplained328pb/firmware.hex`。
4. 用可传输数据的 USB 线连接开发板，点击 **Upload**。Part B 烧录后，按下 PB0 按钮时 PB5 板载 LED 点亮，松开时熄灭。
5. 后续将实验代码写入 `src/main.c`，其他参与编译的 `.c` 文件放入 `src/`。一个工程只保留一个 `main()`；各阶段备份放在 `src/` 外。

若打开目录后没有出现工程任务，运行命令面板 **Developer: Reload Window**。若 AVR 头文件有红线，运行 **PlatformIO: Rebuild IntelliSense Index**。

## 终端命令

从命令面板打开 **PlatformIO: Open PlatformIO Core CLI** 后执行：

```sh
pio run
pio run -t upload
pio device list
pio device monitor
```

普通 PowerShell 若找不到 `pio`，这台电脑可以使用内置 Core 的完整路径：

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run
```

## 烧录与时钟

- 配置使用板载 **mEDBG** 的 `xplainedmini` ISP 协议，通过 USB 烧录；不需要 Arduino bootloader，也不需要指定 COM 口进行烧录。
- `board = ATmega328PB` 必须保留，不能换成 `uno` 或 ATmega328P。
- 编译时钟设为 **16 MHz**，适用于开发板默认的外部 16 MHz 时钟、5 V 供电。`F_CPU` 只是编译设置，不会改变熔丝。
- 上传命令只请求写入 Flash，并保留 AVRDUDE 默认的写后校验。不需要执行 **Set Fuses** 或 **Burn Bootloader**。
- 若提示找不到 `xplainedmini`，先检查数据线和 USB 连接，并关闭占用 mEDBG 的其他软件。若板子之前启用了 debugWIRE，可能需要在 MPLAB 中先关闭 debugWIRE，才能恢复 ISP。

## 后续串口输出

当前 Part B 程序不发送串口数据。课程 [avr-printf](https://github.com/upenn-embedded/avr-printf) 的默认配置是 **9600 baud、8 数据位、无校验、2 停止位（8N2）**。这台电脑已安装 VS Code **Serial Monitor** 扩展，加入课程库后可在其界面选择这些参数。

也可以使用 PlatformIO 串口监视器；当前预设为 **9600、8N1**。若使用它，建议同步把 UART 代码的停止位设为 1。使用 `pio device list` 查看 mEDBG 虚拟串口；若有多个串口，在 `platformio.ini` 中设置实际的 `monitor_port = COM数字`。退出 PlatformIO 串口监视器使用 **Ctrl+C**。

课程 UART 源码目前使用 `__init_stdout` / `__init_stdin`。这些接口需要适配到 AVR-GCC / avr-libc 的流接口（例如 `fdev_setup_stream`），不能假设从 MPLAB 工程直接复制后就能编译；本工程尚未加入该库。

## 参考

- [PlatformIO ATmega328PB 配置与调试支持](https://docs.platformio.org/en/latest/boards/atmelavr/ATmega328PB.html)：目前未提供该型号的 PlatformIO 断点调试支持。
- [PlatformIO 使用 ISP 编程器烧录](https://docs.platformio.org/en/latest/platforms/atmelavr.html#upload-using-programmer)。
- [Microchip ATmega328PB Xplained Mini 用户指南](https://ww1.microchip.com/downloads/aemDocuments/documents/MCU08/ProductDocuments/UserGuides/ATmega328PB-Xplained-Mini-UG-DS50002660B.pdf)。

课程提供的 PlatformIO PDF 在本次配置时无法访问，因此烧录配置依据上述官方资料。实体板尚未连接时，编译通过不代表已经验证烧录和 LED 行为。
