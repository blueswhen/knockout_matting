// Copyright 2014-4 sxniu
#include <stdio.h>
#include "include/RWjpeg.h"
#include "include/ImageData.h"
#include "include/utils.h"

int main(int argc, char** argv) {
  ImageData girl_img;
  RWjpeg girl_jpg("test_image/girl_input.jpg", &girl_img);

  ImageData girl_trimap;
  RWjpeg girl_trimap_jpg("test_image/girl_mask.jpg", &girl_trimap);

  utils::CreateAlphaForTrimap(&girl_trimap);
  utils::SetAlphaForImage(girl_trimap, &girl_img);

  girl_jpg.Save("girl_out.jpg");
}
