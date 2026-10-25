# Compile check

Run `./tests/compile-examples.ps1` from PowerShell after installing `arduino-cli` and Espressif `esp32:esp32` core **3.3.12**. The script compiles every complete sketch in `examples/` for the classic ESP32 target and fails if the expected core version is missing. This environment has no Arduino CLI, so the matrix has not been run here. Hardware commission and controller checks still need a physical ESP32-WROOM-32.
