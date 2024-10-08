#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
# You will need GLFW (http://www.glfw.org):
# Linux:
#   apt-get install libglfw-dev
# Mac OS X:
#   brew install glfw
# MSYS2:
#   pacman -S --noconfirm --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-glfw
#

CXX = g++
#CXX = clang++

EXE = stegosaurus
IMGUI_DIR = ../..
SOURCES = main.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SOURCES += $(IMGUI_DIR)/lib/cpp_exiftool/src/ExifTool.cpp $(IMGUI_DIR)/lib/cpp_exiftool/src/ExifToolPipe.cpp $(IMGUI_DIR)/lib/cpp_exiftool/src/TagInfo.cpp
# SOURCES += $(IMGUI_DIR)/lib/opencv/include/opencv2.cpp
#SOURCES += $(IMGUI_DIR)/lib/ImageMagick-6
HDR += $(IMGUI_DIR)/lib/cpp_exiftool/inc/ExifTool.h $(IMGUI_DIR)/lib/cpp_exiftool/inc/ExifToolPipe.h $(IMGUI_DIR)/lib/cpp_exiftool/inc/TagInfo.h
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))
UNAME_S := $(shell uname -s)
LINUX_GL_LIBS = -lGL

CXXFLAGS = -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -I$(IMGUI_DIR)/lib/ImageMagick-6 -I$(IMGUI_DIR)/lib/ImageMagick-6/Magick++ -I$(IMGUI_DIR)/lib/ImageMagick-6/magick -I$(IMGUI_DIR)/lib/ImageMagick-6/wand -I$(IMGUI_DIR)/lib/opencv -I$(IMGUI_DIR)/lib/cpp_exiftool/inc
CXXFLAGS += -g -Wall -Wformat -std=c++2a -pthread
CXXFLAGS += `Magick++-config --cflags --cppflags`
CXXFLAGS += `Magick++-config --ldflags --libs`
CXXFLAGS += $(shell pkg-config --cflags opencv)
LDFLAGS += $(shell pkg-config --libs --static opencv)
CFLAGS += -Wno-deprecated-enum-enum-conversion
#CXXFLAGS += `Magick++-config --cxxflags --libs --cppflags` -O2 
#LDFLAGS = `pkg-config --libs opencv`

LIBS =

##---------------------------------------------------------------------
## OPENGL ES
##---------------------------------------------------------------------

## This assumes a GL ES library available in the system, e.g. libGLESv2.so
# CXXFLAGS += -DIMGUI_IMPL_OPENGL_ES2
# LINUX_GL_LIBS = -lGLESv2

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS += $(LINUX_GL_LIBS) `pkg-config --static --libs glfw3`
# 	LIBS += $(LINUX_GL_LIBS) `pkg-config --static --libs opencv`
	#LIBS += `/usr/local/lib/ /usr/local/lib64/`

	#LIBS += $(IMGUI_DIR)/lib/ImageMagick-6

	CXXFLAGS += `pkg-config --cflags glfw3`
	#CXXFLAGS += `../../ExifTool.h ../../ExifToolPipe.h ../../TagInfo.h`
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
	LIBS += -L/usr/local/lib -L/opt/local/lib -L/opt/homebrew/lib
	#LIBS += -lglfw3
	LIBS += -lglfw

	CXXFLAGS += -I/usr/local/include -I/opt/local/include -I/opt/homebrew/include
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
	LIBS += -lglfw3 -lgdi32 -lopengl32 -limm32

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif


##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

%.o:%.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -c -o $@ $< 
# 	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/lib/cpp_exiftool/src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# encode: encode.o; $(CXX) $< -o $@ $(LDFLAGS)
# decode: decode.o; $(CXX) $< -o $@ $(LDFLAGS)
# bitpl: bitpl.o; $(CXX) $< -o $@ $(LDFLAGS)

# %.o:$(IMGUI_DIR)/libs/opencv/%.o 
# 	$(CXX) $< -o $@ $(CXXFLAGS)

# %.o:$(IMGUI_DIR)/lib/ImGuiFileDialog/%.cpp
# 	$(CXX) $(CXXFLAGS) -c -o $@ $<

# %.o:$(IMGUI_DIR)/lib/ImageMagick-6/%.cpp
#	$(CXX) `Magick++-config` $(CXXFLAGS) $(LIBS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS) 	
# 	$(CXX) $< -o $@ $(CXXFLAGS) $(LIBS) $(LDFLAGS)

clean:
	rm -f $(EXE) $(OBJS)
