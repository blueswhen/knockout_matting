// Copyright 2014-4 sxniu
#include <stdio.h>
#include "include/RWImage.h"
#include "include/ImageData.h"
#include "include/utils.h"
#include "include/matting.h"

// #define SCRRIBLE

int main(int argc, char** argv) {
  if (argc < 3) {
    printf("error: no input image and scribble map\n");
    return 0;
  }
  ImageData girl_img;
  RWImage girl_rw(argv[1], &girl_img);

  ImageData girl_trimap;
  RWImage girl_trimap_rw(argv[2], &girl_trimap);

  // utils::TurnGray(&girl_img);
  // utils::TurnHSI(&girl_img);
  // utils::TurnLAB(&girl_img);
  // utils::CreateAlphaForTrimap(&girl_trimap);
  // utils::SetAlphaForImage(girl_trimap, &girl_img);
#ifdef SCRRIBLE
  matting::GetTrimapWithScribble(&girl_trimap);
#else
  matting::GetTrimap(&girl_trimap);
#endif
  matting::SetSceneLineNearUnknownArea(&girl_trimap);
  matting::GenerateForegroundWithAlphaValue(girl_img, &girl_trimap);
  matting::RemoveSceneLineNearUnknownArea(&girl_trimap);

  // matting::RemoveSceneLineNearUnknownArea(girl_img, &girl_trimap);

  girl_rw.Save("girl.bmp");
  girl_trimap_rw.Save("girl_trimap.bmp");
  return 0;
}
