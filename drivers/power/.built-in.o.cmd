cmd_drivers/power/built-in.o :=  arm-none-eabi-ld -EL    -r -o drivers/power/built-in.o drivers/power/power_supply.o drivers/power/battery_monitor.o drivers/power/charger_max8845.o drivers/power/fuelgauge_max17040.o drivers/power/sleep_i2c1.o drivers/power/sleep_i2c2.o drivers/power/sleep_madc.o 