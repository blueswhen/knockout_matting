// Copyright 2014 sxniu
#ifndef INCLUDE_UTILS_H_
#define INCLUDE_UTILS_H_

#define EIGHT_ARROUND_POSITION(center_x, center_y, width, height) \
{ \
  center_y * width + center_x + 1, \
  (center_y + 1) * width + center_x + 1, \
  (center_y + 1) * width + center_x, \
  (center_y + 1) * width + center_x - 1, \
  center_y * width + center_x - 1, \
  (center_y - 1) * width + center_x - 1, \
  (center_y - 1) * width + center_x, \
  (center_y - 1) * width + center_x + 1, \
}

class ImageData;

namespace utils {

void GenerateForegroundWithAlphaValue(ImageData* origin, ImageData* trimap);
void SetSceneLineNearUnknownArea(ImageData* image);
void RemoveSceneLineNearUnknownArea(ImageData* image);
void GetTrimapWithScribble(ImageData* image);
void GetTrimap(ImageData* image);
void TurnGray(ImageData* image);
// alpha managerment
void CreateAlphaForTrimap(ImageData* trimap);
void SetAlphaForImage(const ImageData& alpha_map, ImageData* image_data);
void ShowAlpha(ImageData* image);

}  // namespace utils

#endif  // INCLUDE_UTILS_H_
