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

#define GET_THREE_COORDINATE(colour) \
{ \
  (colour & RED) >> 16, \
  (colour & GREEN) >> 8, \
  colour & BLUE \
}

#define TURN_COORDINATE_TO_COLOUR(x, y, z) ((x << 16) + (y << 8) + z)

#define GET_DISTENCE(d, s) \
{ \
  abs(d[0] - s[0]), \
  abs(d[1] - s[1]), \
  abs(d[2] - s[2]), \
}

#define GET_DIFFERENCE(d, s) \
{ \
  d[0] - s[0], \
  d[1] - s[1], \
  d[2] - s[2], \
}

class ImageData;

namespace utils {

void TurnGray(ImageData* image);
void TurnHSI(ImageData* image);
void TurnLAB(ImageData* image);
// alpha managerment
void CreateAlphaForTrimap(ImageData* trimap);
void SetAlphaForImage(const ImageData& alpha_map, ImageData* image_data);
void ShowAlpha(ImageData* image);

}  // namespace utils

#endif  // INCLUDE_UTILS_H_
