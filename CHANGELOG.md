# Changelog

## v1.6.0 (07/05/2020)

#### Core Improvements - CDVD

- CHD support (credits to @SleepyMan and @siddhartha77 too) [#4314](https://github.com/PCSX2/pcsx2/pull/4314) by {{user}}

#### Core Improvements - DEV9

- DEV9: add ATA Idle Immediate (E1) stub [#4364](https://github.com/PCSX2/pcsx2/pull/4364) by {{user}}
- DEV9: Do not apply byteswapping to TX/RX FIFO writes/reads [#4381](https://github.com/PCSX2/pcsx2/pull/4381) by {{user}}

#### Core Improvements - IPU

- IPU: Remove useless define and includes/externs [#4280](https://github.com/PCSX2/pcsx2/pull/4280) by {{user}}

#### Core Improvements - Memory Card

- Folder memcards: Add an index file to track order of files [#3941](https://github.com/PCSX2/pcsx2/pull/3941) by {{user}}

#### Core Improvements - PAD

- pad-linux: Update to latest controller database. [#4390](https://github.com/PCSX2/pcsx2/pull/4390) by {{user}}
- PAD: Just enable pressure on CMD 0x4F [#4331](https://github.com/PCSX2/pcsx2/pull/4331) by {{user}}
- PAD: Add one more byte to umask. [#4305](https://github.com/PCSX2/pcsx2/pull/4305) by {{user}}
- pad-windows: Fix a couple of warnings, make codacy happy. [#4282](https://github.com/PCSX2/pcsx2/pull/4282) by {{user}}

#### Core Improvements - SPU2

- SPU2: Change default of latency slider [#3825](https://github.com/PCSX2/pcsx2/pull/3825) by {{user}}
- Capture: Optimizations with file names, SPU2 recording, and capture menu gui [#4036](https://github.com/PCSX2/pcsx2/pull/4036) by {{user}}

#### Core Improvements - USB

- USB: Improve Force Feedback test so it doesn't block UI [#4310](https://github.com/PCSX2/pcsx2/pull/4310) by {{user}}
- [USB] Change wheel type to device subtype [#3923](https://github.com/PCSX2/pcsx2/pull/3923) by {{user}}

#### GSdx Improvements

- GSdx-hw: Some minor cleanups [#4383](https://github.com/PCSX2/pcsx2/pull/4383) by {{user}}
- gsdx-vsprops: Fix build configuration. [#4391](https://github.com/PCSX2/pcsx2/pull/4391) by {{user}}
- Format GSdx [#4363](https://github.com/PCSX2/pcsx2/pull/4363) by {{user}}
- gsvector: fix forgotten elif during purge [#4367](https://github.com/PCSX2/pcsx2/pull/4367) by {{user}}
- gsdx: Clean up a few warnings. [#4366](https://github.com/PCSX2/pcsx2/pull/4366) by {{user}}
- Purge SSE2/SSE3 [#4329](https://github.com/PCSX2/pcsx2/pull/4329) by {{user}}
- gsdx-gui: Remove partial and full crc hack levels on release builds. [#4350](https://github.com/PCSX2/pcsx2/pull/4350) by {{user}}
- GS/Core: Implement the new generation of GS Debugger [#4271](https://github.com/PCSX2/pcsx2/pull/4271) by {{user}}
- GSdx: Don't use constexpr vector constructors at runtime [#4288](https://github.com/PCSX2/pcsx2/pull/4288) by {{user}}
- gsdx-d3d11: Cleanup gsdevice11.cpp warnings reported on codacy. [#4270](https://github.com/PCSX2/pcsx2/pull/4270) by {{user}}
- gsdx: Remove TV Shader hotkey toggle (F7). [#3905](https://github.com/PCSX2/pcsx2/pull/3905) by {{user}}
- gsdx: Use constexpr to initialize avx vectors without avx instructions [#4131](https://github.com/PCSX2/pcsx2/pull/4131) by {{user}}
- GSdx: switch to unicode [#4188](https://github.com/PCSX2/pcsx2/pull/4188) by {{user}}

#### GameDB Changes

- GameDB:fix errors in GameDB [#4389](https://github.com/PCSX2/pcsx2/pull/4389) by {{user}}
- GameDB:adds gamefixes for various games  [#4388](https://github.com/PCSX2/pcsx2/pull/4388) by {{user}}
- GameDB: Add patches for KOF series [#4187](https://github.com/PCSX2/pcsx2/pull/4187) by {{user}}
- GameDB: Add improved Xenosaga save crash prevention patch [#4259](https://github.com/PCSX2/pcsx2/pull/4259) by {{user}}

#### Misc. Improvments - GUI Interface

- Core: Purge OS specific CPU usage providers [#4384](https://github.com/PCSX2/pcsx2/pull/4384) by {{user}}
- Rename main menu back to System [#4362](https://github.com/PCSX2/pcsx2/pull/4362) by {{user}}
- GUI: change window to debug in the menu and  move blockdump to  the debug menu  [#4378](https://github.com/PCSX2/pcsx2/pull/4378) by {{user}}
- GUI:fix capitalisation in GUI [#4165](https://github.com/PCSX2/pcsx2/pull/4165) by {{user}}
- Misc: Enable GS debugger on CI artifacts [#4343](https://github.com/PCSX2/pcsx2/pull/4343) by {{user}}
- GUI-Speedhacks: Grayout Instant VU1 when MTVU is enabled [#4278](https://github.com/PCSX2/pcsx2/pull/4278) by {{user}}
- gui: Visually indicate the current savestate slot in the load/save menu [#4306](https://github.com/PCSX2/pcsx2/pull/4306) by {{user}}
- gui: If available, enable menu option to load backup savestate when a game is first launched [#3556](https://github.com/PCSX2/pcsx2/pull/3556) by {{user}}

#### Misc. Improvments - Input Recording

- GUI Improvements around savestate-based input recordings [#4060](https://github.com/PCSX2/pcsx2/pull/4060) by {{user}}
- Reorganize and realign InputRecordingControls [#4019](https://github.com/PCSX2/pcsx2/pull/4019) by {{user}}

#### Misc. Improvments - Debugger

- Misc:Debugger: Support write-only GS priv reads [#4338](https://github.com/PCSX2/pcsx2/pull/4338) by {{user}}
- Misc:Debugger: Bios thread view support [#4345](https://github.com/PCSX2/pcsx2/pull/4345) by {{user}}
- Misc: Support more BIOSes for the Debugger Thread View [#4287](https://github.com/PCSX2/pcsx2/pull/4287) by {{user}}
- Debugger: Require emulator start before enabling Break/Run button [#4289](https://github.com/PCSX2/pcsx2/pull/4289) by {{user}}

#### Misc. Improvements - IPC

- IPC: Socket reusability [#4262](https://github.com/PCSX2/pcsx2/pull/4262) by {{user}}
