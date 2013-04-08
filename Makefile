CFLAGS=-O2 -g -Wall -I . -DGLEW_STATIC
CXXFLAGS=$(CFLAGS)
LDFLAGS=-L. -static-libgcc -static-libstdc++
LDLIBS=-lopengl32 -lglu32 -lcomdlg32 -luser32 -lgdi32 -llib3ds

BASE_OBJS = glew.o bmdread.o openfile.o common.o drw1.o \
            evp1.o inf1.o jnt1.o mat3.o mdl3.o shp1.o tex1.o \
            vtx1.o transformtools.o addons/export3ds.o addons/exportTexture.o

BMDVIEW2_OBJS = drawBmd.o main.o camera.o parameters.o \
       simple_gl_common.o oglblock.o ui.o simple_gl.o \
       addons/bck.o addons/btp.o $(BASE_OBJS)

all: bmdview2 bmd23ds

bmdview2: $(BMDVIEW2_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

bmd23ds: $(BASE_OBJS) bmd23ds.o
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)
