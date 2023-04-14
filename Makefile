# Run: make Release 
# Run: make Debug 
CC=g++
WINDRES=windres
# comflag = -m32
# comflag = -m64
comflag = $(COMFLAG)

wxWidgets_DIR = wxWidgets-3.2.2
wxWidgets_gcc = gcc1220


DSPElib_DIR = ./DSPE_lib_minGW/x86_64-w64-mingw32-gcc_12.2.0


SRC_DIR = .
SRC_CPP_SUBDIR = .

MAIN_APP_NAME = TeleSound


#DFLAGS         = -DWIN32 -DDEVCPP 

ifeq ($(MODE),Release)
	CFLAGS = $(comflag) -std=c++17 -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing -fpermissive
	LINKER_FLAGS = $(comflag)  -s -static-libgcc -static-libstdc++ $(MISC_LINKER_FLAGS)  -L$(DSPElib_DIR)/rls -L$(wxWidgets_DIR)/lib_dev/$(wxWidgets_gcc)_x64_dll
	INCLUDES := -Isrc/include -Isrc_support/include -I"$(wxWidgets_DIR)/include" -I"$(wxWidgets_DIR)/lib_dev/$(wxWidgets_gcc)_x64_dll/" -I"$(wxWidgets_DIR)/lib_dev/$(wxWidgets_gcc)_x64_dll/mswud" -I"$(wxWidgets_DIR)/lib_dev/$(wxWidgets_gcc)_x64_dll" -I"$(DSPElib_DIR)/include" -I"$(DSPElib_DIR)/include/rls"
	MAIN_APP_EXE_FILENAME = $(MAIN_APP_NAME)_rls.exe
else
	CFLAGS   = $(comflag) -std=c++17 -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -fmax-errors=5 -fpermissive
	LINKER_FLAGS   = $(comflag)  -static-libgcc -static-libstdc++ $(MISC_LINKER_FLAGS) -L$(DSPElib_DIR)/dbg -L$(wxWidgets_DIR)/lib_dev/$(wxWidgets_gcc)_x64_dll
	INCLUDES := -Isrc/include -Isrc_support/include -I"$(wxWidgets_DIR)/include" -I"$(wxWidgets_DIR)/lib_dev/$(wxWidgets_gcc)_x64_dll/" -I"$(wxWidgets_DIR)/lib_dev/$(wxWidgets_gcc)_x64_dll/mswud" -I"$(wxWidgets_DIR)/lib_dev/$(wxWidgets_gcc)_x64_dll" -I"$(DSPElib_DIR)/include" -I"$(DSPElib_DIR)/include/dbg" 
	MAIN_APP_EXE_FILENAME = $(MAIN_APP_NAME)_dbg.exe
endif


MAIN_APP_SOURCES_NAMES = 
MAIN_APP_SOURCES_NAMES += src/Branches.cpp src/IIR_coefs.cpp src/MorseDecoder.cpp src/Processing.cpp src/Vectors.cpp src/TeleSound_wxGlade.cpp src/main.cpp 
MAIN_APP_SOURCES_NAMES += src_support/BitmapFont.cpp src_support/DSP.cpp src_support/GUIcalls.cpp src_support/OutlineFont.cpp src_support/wxAddons.cpp 

MAIN_APP_SOURCES = $(addprefix $(SRC_CPP_SUBDIR)/,$(MAIN_APP_SOURCES_NAMES))


# ################################################# #
# DEBUG
MAIN_APP_OBJECTS 	  := $(MAIN_APP_SOURCES:%.cpp=$(OUT_DIR)_app/%.o)
MAIN_APP_DEPENDS 	  := $(MAIN_APP_SOURCES:%.cpp=$(OUT_DIR)_app/%.d)

# ################################################# #
#-include $(DEPENDS)

all: build 


# ########################################################################################### #	
# ########################################################################################### #	
build: $(SRC_DIR)/$(MAIN_APP_EXE_FILENAME)
-include $(MAIN_APP_DEPENDS)


$(SRC_DIR)/$(MAIN_APP_EXE_FILENAME): $(MAIN_APP_OBJECTS)
	@echo $(MAIN_APP_EXE_FILENAME)
	$(WINDRES) src/Telesound.rc -o $(OUT_DIR)_app/Telesound_rc.o $(INCLUDES)
	$(CC) $(MAIN_APP_OBJECTS) $(OUT_DIR)_app/Telesound_rc.o -o "$(SRC_DIR)/$(MAIN_APP_EXE_FILENAME)" $(LINKER_FLAGS) $(LIBS)

# ########################################################################################### #	
# ########################################################################################### #	
# Z podanej listy usuwany $(OUT_DIR_WIN_RLS)/ oraz '.o' zamieniamy na '.cpp'
$(MAIN_APP_OBJECTS): $(OUT_DIR)_app/%.o : %.cpp
	@echo $(@D) $< $@

	#mkdir -p $(OUT_DIR)_app/$(SRC_CPP_SUBDIR)
	mkdir -p $(@D)
	$(CC) $(DFLAGS) $(CFLAGS) $(INCLUDES) -MMD $< -o $@


clean:
	@echo MODE: $(MODE)

	@if [ -d "$(OUT_DIR)_app" ]; then \
		echo "cleaning $(OUT_DIR)_app ..."; \
		#find $(OUT_DIR)_app/ -name "*.o" -type f -delete; \
		rm -rf $(OUT_DIR)_app/*.d; \
		rm -rf $(OUT_DIR)_app/*.o; \
		rm -rf $(OUT_DIR)_app; \
		echo "cleaned $(OUT_DIR)_app"; \
	fi 
	@if [ -d "$(OUT_DIR)" ]; then \
		echo "cleaning $(OUT_DIR_DBG) ..."; \
		rm -rf $(OUT_DIR); \
		echo "cleaned $(OUT_DIR)"; \
	fi 
	rm -rf "$(SRC_DIR)/$(MAIN_APP_EXE_FILENAME)"; \
	
.PHONY: all build clean

