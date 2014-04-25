// Copyright 2014-4 sxniu
#ifndef INCLUDE_IMAGEDATA_H_
#define INCLUDE_IMAGEDATA_H_

#include <vector>

class ImageData {
 public:
  friend class RWjpeg;

  ImageData();
  ImageData(const ImageData& image_data);
  ~ImageData();
  const char* GetFileName();
  int GetWidth();
  int GetHeight();
  int GetPixel(int index);
  void SetPixel(int index, int colour);

 private:
  bool IsIndexValid(int index);
  const char* m_file_name;
  int m_width;
  int m_height;
  std::vector<int>* m_data;
};

#endif  // INCLUDE_IMAGEDATA_H_
