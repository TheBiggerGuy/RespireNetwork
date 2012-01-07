####################################################################
# Definitions                                                      #
####################################################################

DEVICE = EFM32G890F128
PROJECTNAME = RespireNetwork

SRC_DIR = src
OBJ_DIR = build
EXE_DIR = exe
LST_DIR = lst

CMSIS_DIR       = lib/CMSIS
EFM32LIB_DIR    = lib/efm32lib
BOARDDRIVER_DIR = lib/drivers

TOOLDIR = /opt/CodeSourcery/Sourcery_CodeBench_Lite_for_ARM_EABI

####################################################################
# Definitions of toolchain.                                        #
# You might need to do changes to match your system setup          #
####################################################################

CC      = $(TOOLDIR)/bin/arm-none-eabi-gcc
LD      = $(TOOLDIR)/bin/arm-none-eabi-ld
AR      = $(TOOLDIR)/bin/arm-none-eabi-ar
OBJCOPY = $(TOOLDIR)/bin/arm-none-eabi-objcopy
DUMP    = $(TOOLDIR)/bin/arm-none-eabi-objdump --disassemble

GCCVERSION = $(shell $(CC) -dumpversion)

####################################################################
# Flags                                                            #
####################################################################

# -MMD : Don't generate dependencies on system header files.
# -MP  : Add phony targets, useful when a h-file is removed from a project.
# -MF  : Specify a file to write the dependencies to.
DEPFLAGS = -MMD -MP -MF $(@:.o=.d)

# Add -Wa,-ahld=$(LST_DIR)/$(@F:.o=.lst) to CFLAGS to produce assembly list files
CFLAGS += -D$(DEVICE)
CFLAGS += -mcpu=cortex-m3
CFLAGS += -mthumb
CFLAGS += -ffunction-sections
CFLAGS += -fdata-sections
CFLAGS += -mfix-cortex-m3-ldrd
CFLAGS += -fomit-frame-pointer
CFLAGS += -Wall
CFLAGS += -std=c99
CFLAGS += $(DEPFLAGS)

ASMFLAGS += -Ttext 0x0

LDFLAGS += -Xlinker
LDFLAGS += -Map=$(LST_DIR)/$(PROJECTNAME).map
LDFLAGS += -mcpu=cortex-m3 -mthumb
LDFLAGS += -T $(CMSIS_DIR)/CM3/DeviceSupport/EnergyMicro/EFM32/startup/cs3/efm32g.ld
LDFLAGS += -L $(TOOLDIR)/arm-none-eabi/lib/thumb2
LDFLAGS += -L $(TOOLDIR)/lib/gcc/arm-none-eabi/$(GCCVERSION)/thumb2
LDFLAGS += -Wl,--gc-sections

LIBS += -l c
LIBS += -l cs3
LIBS += -l cs3unhosted

INCLUDEPATHS += -I $(SRC_DIR)
INCLUDEPATHS += -I $(CMSIS_DIR)/CM3/CoreSupport
INCLUDEPATHS += -I $(CMSIS_DIR)/CM3/DeviceSupport/EnergyMicro/EFM32
INCLUDEPATHS += -I $(EFM32LIB_DIR)/inc
INCLUDEPATHS += -I $(BOARDDRIVER_DIR)/inc

####################################################################
# Files                                                            #
####################################################################

# CMSIS
C_SRC += $(CMSIS_DIR)/CM3/CoreSupport/core_cm3.c
C_SRC += $(CMSIS_DIR)/CM3/DeviceSupport/EnergyMicro/EFM32/system_efm32.c
# EFM32Lib
C_SRC += $(wildcard $(EFM32LIB_DIR)/src/*.c)
# board driver
C_SRC += $(wildcard $(BOARDDRIVER_DIR)/src/*.c)
# src program
C_SRC += $(wildcard $(SRC_DIR)/*.c)

# CMIS
S_SRC += $(CMSIS_DIR)/CM3/DeviceSupport/EnergyMicro/EFM32/startup/cs3/startup_efm32.s
# src program
S_SRC += $(wildcards $(SRC_DIR)/*.s)

####################################################################
# Rules                                                            #
####################################################################

C_FILES = $(notdir $(C_SRC) )
S_FILES = $(notdir $(S_SRC) )
#make list of source paths, sort also removes duplicates
# TODO why?
C_PATHS = $(sort $(dir $(C_SRC) ) )
S_PATHS = $(sort $(dir $(S_SRC) ) )

C_OBJS = $(addprefix $(OBJ_DIR)/, $(C_FILES:.c=.o))
S_OBJS = $(addprefix $(OBJ_DIR)/, $(S_FILES:.s=.o))
C_DEPS = $(addprefix $(OBJ_DIR)/, $(C_FILES:.c=.d))

vpath %.c $(C_PATHS)
vpath %.s $(S_PATHS)


