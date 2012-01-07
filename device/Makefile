####################################################################
# Makefile                                                         #
####################################################################

.SUFFIXES:				# ignore builtin rules
.PHONY: all debug release internal_build makeDir clean
.SILENT:

# ALL THE PROJECT SPECIFIC CODE
include project.mk


# Default build is debug build
all:      debug

debug:    CFLAGS += -DDEBUG -O0 -g3
debug:    internal_build

release:  CFLAGS += -DNDEBUG -O3
release:  internal_build

internal_build:    makeDirs $(EXE_DIR)/$(PROJECTNAME).bin

# print all the vars out
showInfo:
	@echo "INCLUDEPATHS: "$(INCLUDEPATHS)
	@echo "C_SRC       : "$(C_SRC)
	@echo "C_FILES     : "$(C_FILES)
	@echo "S_SRC       : "$(S_SRC)
	@echo "S_FILES     : "$(S_FILES)
	@echo
	@echo "*.c         : "$(SRC_DIR)/*.c
	@echo "*.c         : "$(wildcard $(SRC_DIR)/*.c)

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
$(EXE_DIR)/$(PROJECTNAME).out: $(C_OBJS) $(S_OBJS)
	@echo
	@echo "Linking target: $@"
	$(CC) $(LDFLAGS) $(C_OBJS) $(S_OBJS) $(LIBS) -o $(EXE_DIR)/$(PROJECTNAME).out

# Create binary file
# and
# produce assembly listing of entire program
$(EXE_DIR)/$(PROJECTNAME).bin: $(EXE_DIR)/$(PROJECTNAME).out
	@echo "Creating binary file"
	$(OBJCOPY) -O binary $(EXE_DIR)/$(PROJECTNAME).out $(EXE_DIR)/$(PROJECTNAME).bin
	$(DUMP) $(EXE_DIR)/$(PROJECTNAME).out > $(LST_DIR)/$(PROJECTNAME)out.lst

clean:
	rm -r $(OBJ_DIR)
	@echo "Removing build directory."
	rm -r $(EXE_DIR)
	@echo "Removing executable directory."
	rm -r $(LST_DIR)
	@echo "Removing list directory."

# include auto-generated dependency files (explicit rules)
ifneq (clean,$(findstring clean, $(MAKECMDGOALS)))
-include $(C_DEPS)
endif
