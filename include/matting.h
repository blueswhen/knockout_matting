// Copyright 2014 sxniu
#ifndef INCLUDE_MATTINGS_H_
#define INCLUDE_MATTINGS_H_

class ImageData;

namespace matting {

void GenerateForegroundWithAlphaValue(const ImageData& origin, ImageData* trimap);
void SetSceneLineNearUnknownArea(ImageData* image);
void RemoveSceneLineNearUnknownArea(ImageData* image);
// using origion image to restore scene line
void RemoveSceneLineNearUnknownArea(const ImageData& origin, ImageData* trimap);
void GetTrimapWithScribble(ImageData* image);
void GetTrimap(ImageData* image);

}  // namespace matting

#endif  // INCLUDE_MATTINGS_H_
