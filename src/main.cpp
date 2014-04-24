// Copyright 2014-4 sxniu

#include "RWjpeg.h"

int main(int argc, char** argv) {
  RWjpeg input_image("test_image/girl_input.jpg");
  input_image.Save("image_out.jpg");
}
