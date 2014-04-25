knockout : src/main.cpp src/RWjpeg.cpp src/ImageData.cpp include/RWjpeg.h include/ImageData.h
	g++ src/main.cpp src/RWjpeg.cpp src/ImageData.cpp -ljpeg -I.
