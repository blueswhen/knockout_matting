// Copyright 2014-4 sxniu
#include "include/RWjpeg.h"
#include "include/ImageData.h"

int main(int argc, char** argv) {
  ImageData img;
  RWjpeg input_image("test_image/girl_input.jpg", &img);
  input_image.Save("io.jpg");
  RWjpeg test_image(&img);
  test_image.Save("image_out.jpg");

  // RWjpeg trimap("test_image/girl_mask.jpg");
}
