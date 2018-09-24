export PATH:=/opt/gcc-arm-none-eabi-7-2017-q4-major/bin:$(PATH)
#GCC_PATH = /opt/gcc-arm-none-eabi-7-2017-q4-major/bin
#.DEFAULT_GOAL := all

# additional source files
C_SOURCES += \
			 board.c \
			 SFlash.c

# additional header folder

# additional rules
clobber:
	-rm -fR .dep $(BUILD_DIR)
	-rm -f tags

ctags:
	-rm -f tags
	-ctags -R . 

astyle:
	-find ./Src -name *.c | xargs astyle --style=allman --indent=tab
	-find ./Src -name *.orig | xargs rm -f
	-find ./Inc -name *.h | xargs astyle --style=allman --indent=tab
	-find ./Inc -name *.orig | xargs rm -f

# flash with STLINK 
sflash:
	openocd \
		-f interface/stlink-v2.cfg \
		-f ./stm32l4.cfg \
		-c 'program $(BUILD_DIR)/$(TARGET).elf reset exit'

sflashv:
	openocd \
		-f interface/stlink-v2.cfg  \
		-f ./stm32l4.cfg \
		-c 'program $(BUILD_DIR)/$(TARGET).elf verify reset exit'

# flash with J-Link
jflash:
	JLinkExe -if swd -device stm32l432kc -speed 4000 -commandfile jlinkcmd > /dev/null

jflashv:
	JLinkExe -if swd -device stm32l432kc -speed 4000 -commandfile jlinkcmd

# flash with FTDI
lflash:
	openocd \
		-f interface/ftdi/luminary-icdi.cfg \
		-c "transport select swd" \
		-f ./stm32l4.cfg \
		-c 'program $(BUILD_DIR)/$(TARGET).elf reset exit'

lflashv:
	openocd \
		-f interface/ftdi/luminary-icdi.cfg \
		-c "transport select swd" \
		-f ./stm32l4.cfg \
		-c 'program $(BUILD_DIR)/$(TARGET).elf verify reset exit'

oocd:
	openocd \
		-f interface/jlink -f target/stm32l4.cfg

gdbs:
	/opt/SEGGER/JLink/JLinkGDBServerExe -device STM32L432KC -if SWD -speed 4000 &

gdb:
	arm-none-eabi-gdb

cgdb:
	cgdb -d arm-none-eabi-gdb

debug: gdbs gdb
