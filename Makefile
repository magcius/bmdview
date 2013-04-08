GLEW_DIR = glew
GLEW_INC_DIR = $(GLEW_DIR)/include
GLEW_LIB_DIR = $(GLEW_DIR)/lib
LIB3DS_INC_DIR = ../lib3ds-1.2.0
LIB3DS_LIB_DIR = $(LIB3DS_INC_DIR)/lib3ds

CXXFLAGS = -I$(GLEW_INC_DIR) -O2 -ggdb -Wall \
           -I$(LIB3DS_INC_DIR) \
           #-fpascal-strings  # needed for 3d mouse api

BASE_SRCS = bmdread.cpp openfile.cpp common.cpp drw1.cpp \
            evp1.cpp inf1.cpp jnt1.cpp mat3.cpp mdl3.cpp shp1.cpp tex1.cpp \
            vtx1.cpp

BMDINFO_SRCS = bmdinfo.cpp $(BASE_SRCS)

BMDINFO_OBJS = $(BMDINFO_SRCS:.cpp=.o)

BMDVIEW2_SRCS = simple_gl_glut.cpp main.cpp drawBmd.cpp \
       camera.cpp drawtext.cpp parameters.cpp transformtools.cpp\
       simple_gl_common.cpp oglblock.cpp ui.cpp \
       addons/exportTexture.cpp addons/export3ds.cpp addons/exportb3d.cpp \
       addons/bck.cpp addons/btp.cpp $(BASE_SRCS)

BMDVIEW2_OBJS = $(BMDVIEW2_SRCS:.cpp=.o)


# I don't know how dylibs work with bundles, so I've statically linked GLEW
# for now.

# this can be used to emped an Info.plist without building a bundled app,
# but it seems not to suffice for the 3d mouse api 
#-sectcreate __TEXT __info_plist Info.plist \

bmdview2_glut: $(BMDVIEW2_OBJS)
	g++ -o $@ $^ -O2 -lGL -lGLU -lglut -lGLEW -l3ds

Bmdview2.app: bmdview2_glut Info.plist icon.icns
	rm -rf Bmdview2.app
	mkdir -p Bmdview2.app/Contents/MacOS
	mkdir Bmdview2.app/Contents/Resources
	cp Info.plist Bmdview2.app/Contents
	cp icon.icns Bmdview2.app/Contents/Resources
	cp bmdview2_glut Bmdview2.app/Contents/MacOS

mac.o: mac.mm
	g++ -c -O2 mac.mm

mat3.o: common/gccommon.h mat3.h

#TODO: OpenGL/GLEW shouldn't be required for a mere console program...
bmdinfo: $(BMDINFO_OBJS)
	g++ -o $@ $^ \
	-L$(LIB3DS_LIB_DIR) -l3ds \
	-L$(GLEW_LIB_DIR) -lGLEW \
	-framework OpenGL

clean:
	rm -f `echo $(BMDVIEW2_OBJS) $(BMDINFO_OBJS) | sort | uniq` bmdview2_glut

run: bmdinfo
	./bmdinfo testdata/model.bmd

runglut:
	export DYLD_LIBRARY_PATH=$(GLEW_LIB_DIR);./bmdview2_glut testdata/model.bmd

zip:
	zip bmdview2_src_`date '+%Y%m%d'`.zip *.h *.cpp testdata/tests/ common/gccommon.h addons/*.cpp addons/*.h tev.html tev.markdown style.css resource.rc icon.ico Makefile *.inc gl_template.ds*

test: bmdview2_glut
	./bmdview2_glut WoodCircleCutPlanet.bdl
test2: bmdview2_glut
	./bmdview2_glut NigeroPlanet.bdl
