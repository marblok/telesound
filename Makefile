# Run: make Release 
# Run: make Debug 
CC=g++
# comflag = -m32
# comflag = -m64
comflag = $(COMFLAG)

# wxWidgets_DIR = ../wxWidgets-3.1.5
wxWidgets_DIR = ../wxWidgets-3.1.6
wxWidgets_gcc = gcc1120

SRC_DIR = .
SRC_CPP_SUBDIR = .

MAIN_APP_NAME = AutoBatchFeeder


#DFLAGS         = -DWIN32 -DDEVCPP 

ifeq ($(MODE),Release)
	CFLAGS = $(comflag) -std=c++0x -O3 -Wall -c -fmessage-length=0 -fno-strict-aliasing 
	LINKER_FLAGS = $(comflag)  -s -static-libgcc -static-libstdc++ $(MISC_LINKER_FLAGS)
	INCLUDES := -I./include -I"$(wxWidgets_DIR)/include" -I"$(wxWidgets_DIR)/lib_dev/$(wxWidgets_gcc)_x64_dll/mswu"
	MAIN_APP_EXE_FILENAME = $(MAIN_APP_NAME)_rls.exe
else
	CFLAGS   = $(comflag) -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wconversion -fstrict-aliasing -fmax-errors=5
	LINKER_FLAGS   = $(comflag)  -static-libgcc -static-libstdc++ $(MISC_LINKER_FLAGS)
	INCLUDES := -I./include -I"$(wxWidgets_DIR)/include" -I"$(wxWidgets_DIR)/lib_dev/$(wxWidgets_gcc)_x64_dll/mswud"
	MAIN_APP_EXE_FILENAME = $(MAIN_APP_NAME)_dbg.exe
endif
# -U__STRICT_ANSI__ jest potrzebne do kompilacji debug_new.cpp, jezeli pominac ten plik to mozna rowniez wyrzucic te opcje
#CFLAGS_debug   = $(comflag) -std=c++0x -O0 -g3 -Wall -c -fmessage-length=0 -W -Wshadow -Wco#nversion -fstrict-aliasing -U__STRICT_ANSI__

MAIN_APP_SOURCES_NAMES = 
MAIN_APP_SOURCES_NAMES += auto_batch_feeder_wxglade.cpp
MAIN_APP_SOURCES_NAMES += log_buffer.cpp
MAIN_APP_SOURCES_NAMES += auto_batch_feeder_main.cpp
# MAIN_APP_SOURCES_NAMES += jsonval.cpp jsonreader.cpp jsonwriter.cpp 
MAIN_APP_SOURCES = $(addprefix $(SRC_CPP_SUBDIR)/,$(MAIN_APP_SOURCES_NAMES))

SOURCES_DBG =
# SOURCES_DBG += $(SRC_DIR)/Main.cpp

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
	$(CC) $(MAIN_APP_OBJECTS) -o"$(SRC_DIR)/$(MAIN_APP_EXE_FILENAME)" $(LINKER_FLAGS) -L$(wxWidgets_DIR)/lib_dev/$(wxWidgets_gcc)_x64_dll  $(LIBS)


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

