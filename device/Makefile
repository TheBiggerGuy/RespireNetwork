####################################################################
# Makefile                                                         #
####################################################################

# ALL THE PROJECT SPECIFIC CODE
include project.mk

.SUFFIXES:				# ignore builtin rules
.PHONY: all debug release internal_build makeDir clean gdb
.SILENT: # get it to be less verbose

# Default build is debug build
all:      debug

debug:    CFLAGS += -DDEBUG -DDEBUG_EFM -O0 -g3
debug:    internal_build

release:  CFLAGS += -DNDEBUG -O3
release:  internal_build

internal_build:    makeDirs $(EXE_DIR)/$(PROJECTNAME)

# print all the vars out
showInfo:
	@echo "INCLUDEPATHS: "$(INCLUDEPATHS)
	@echo "C_SRC       : "$(C_SRC)
	@echo "C_FILES     : "$(C_FILES)
	@echo "S_SRC       : "$(S_SRC)
	@echo "S_FILES     : "$(S_FILES)

# Create directories
makeDirs:
	mkdir -p $(OBJ_DIR)
	@echo "Created build directory."
	mkdir -p $(EXE_DIR)
	@echo "Created executable directory."
	mkdir -p $(LST_DIR)
	@echo "Created list directory."

# Create objects from C SRC files
$(OBJ_DIR)/%.o: %.c
	@echo
	@echo "Building file : $<"
	$(CC) $(CFLAGS) $(INCLUDEPATHS) -c -o $@ $<

# Assemble .s files
$(OBJ_DIR)/%.o: %.s
	@echo
	@echo "Assembling    : $<"
	$(CC) $(ASMFLAGS) $(INCLUDEPATHS) -c -o $@ $<

# Link
$(EXE_DIR)/$(PROJECTNAME).elf: $(C_OBJS) $(S_OBJS)
	@echo
	@echo "Linking target: $@"
	$(CC) $(LDFLAGS) $(C_OBJS) $(S_OBJS) $(LIBS) -o $(EXE_DIR)/$(PROJECTNAME).elf

# Create binary file
# and
# produce assembly listing of entire program
$(EXE_DIR)/$(PROJECTNAME): $(EXE_DIR)/$(PROJECTNAME).elf
	@echo "Creating flash file"
	$(OBJCOPY) --output-target=ihex   $(EXE_DIR)/$(PROJECTNAME).elf $(EXE_DIR)/$(PROJECTNAME).hex
	$(OBJCOPY) --output-target=binary $(EXE_DIR)/$(PROJECTNAME).elf $(EXE_DIR)/$(PROJECTNAME).bin
	@echo "Creating list file"
	$(DUMP) -h -S $(EXE_DIR)/$(PROJECTNAME).elf > $(LST_DIR)/$(PROJECTNAME).lst

gdb:
	JLinkGDBServer -vd -select usb -if swd -endian little -speed 4000

clean:
	rm -r $(OBJ_DIR)
	@echo "Removing build directory."
	rm -r $(EXE_DIR)
	@echo "Removing executable directory."
	rm -r $(LST_DIR)
	@echo "Removing list directory."
