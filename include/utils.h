// Copyright 2014 sxniu
#ifndef INCLUDE_UTILS_H_
#define INCLUDE_UTILS_H_

class ImageData;

namespace utils {

void GetTrimap(ImageData* image);
void GetBand(ImageData* image);
void TurnGray(ImageData* image);
// alpha managerment
void CreateAlphaForTrimap(ImageData* trimap);
void SetAlphaForImage(const ImageData& alpha_map, ImageData* image_data);
void ShowAlpha(ImageData* image);

}  // namespace utils

#endif  // INCLUDE_UTILS_H_
