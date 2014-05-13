#Copyright 2014-5 sxniu 

CXX = g++
LIBS = -ljpeg
INCS = -I.

knockout : \
	src/main.cpp \
	src/RWjpeg.cpp \
	src/ImageData.cpp \
	src/utils.cpp \
	src/region_filling_by_edge_tracing.cpp \
	include/RWjpeg.h \
	include/ImageData.h \
	include/utils.h \
	include/region_filling_by_edge_tracing.h
	$(CXX) $^ $(LIBS) $(INCS) -o $@

clean :
	rm -f knockout
	rm -f *.jpg
