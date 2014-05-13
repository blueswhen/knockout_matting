
CXX = g++
LIBS = -ljpeg
INCS = -I.

knockout : \
	src/main.cpp \
	src/RWjpeg.cpp \
	src/ImageData.cpp \
	src/utils.cpp \
	include/RWjpeg.h \
	include/ImageData.h \
	include/utils.h
	$(CXX) $^ $(LIBS) $(INCS) -o $@

clean :
	rm -f knockout
	rm -f *.jpg
