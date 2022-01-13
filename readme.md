# Midnight Sun XV Firmware
This repository contains all firmware for the [University of Waterloo](https://uwaterloo.ca/)'s [Midnight Sun Solar Rayce Car](http://www.uwmidsun.com/) team's car #15.

# Known Issues
- FreeRTOS clock is slow on x86. Look for `PortCPUClockFreqHz`. This was used as a quick patch for a strange build issue.
- You can't `LOG_*` or `printf` in a FreeRTOS task on arm unless you set the stack size to >~512 on task creation.
- You can't set the task depth to >128 ish on x86 or else a segfault occurs upon task creation.
