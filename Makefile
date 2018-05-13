#DEBUG = -DDEBUG
CXXFLAGS = -O3 -pipe -g  ${DEBUG}
LDFLAGS = 

OBJS = nal.o hevc_nal.o hevc_cutter.o

.cpp.o:
	g++ -c ${CXXFLAGS} $<

all: hevc_cutter

hevc_cutter: ${OBJS} 
	g++ ${LDFLAGS} -o $@ ${OBJS}
	
clean: 
	rm -rf hevc_cutter ${OBJS}
