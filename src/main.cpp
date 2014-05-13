// Copyright 2014-4 sxniu
#include <stdio.h>
#include "include/RWjpeg.h"
#include "include/ImageData.h"
#include "include/utils.h"

int main(int argc, char** argv) {
  ImageData girl_img;
  RWjpeg girl_jpg("image.jpg", &girl_img);

  ImageData girl_trimap;
  RWjpeg girl_trimap_jpg("image.jpg", &girl_trimap);

  // utils::CreateAlphaForTrimap(&girl_trimap);
  // utils::SetAlphaForImage(girl_trimap, &girl_img);
  utils::GetBand(&girl_trimap);
  utils::GetTrimap(&girl_trimap);

  girl_jpg.Save("girl.jpg");
  girl_trimap_jpg.Save("girl_trimap.jpg");
}
