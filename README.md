# (IDFGH-9017) #10427
This repo is a bare bones project that allowed me to reproduce the issue in [ESP-IDF Issue #10427](https://github.com/espressif/esp-idf/issues/10427).

## Environment

My environment is a specific version of ESP-IDF v4.4. `v4.4.3-316-ge86181704a`.

## Steps to reproduce

1. Run `idf.py build` (You may see the issue at this point)
1. If you don't get a link error, make a small change to the code such as a string change.
1. Run `idf.py build` 

The error I continually see on incremental builds is
```txt
/opt/esp/tools/xtensa-esp32-elf/esp-2021r2-patch5-8.4.0/xtensa-esp32-elf/bin/../lib/gcc/xtensa-esp32-elf/8.4.0/../../../../xtensa-esp32-elf/bin/ld: esp-idf/bt/libbt.a(bt.c.obj):(.literal.esp_bt_mem_release+0x0): undefined reference to `_bt_bss_start'
/opt/esp/tools/xtensa-esp32-elf/esp-2021r2-patch5-8.4.0/xtensa-esp32-elf/bin/../lib/gcc/xtensa-esp32-elf/8.4.0/../../../../xtensa-esp32-elf/bin/ld: esp-idf/bt/libbt.a(bt.c.obj):(.literal.esp_bt_mem_release+0x4): undefined reference to `_bt_bss_end'
```

Running `idf.py fullclean` and then `idf.py build` seems to fix the issue most of the time.