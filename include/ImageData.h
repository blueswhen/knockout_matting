// Copyright 2014-4 sxniu
#ifndef INCLUDE_IMAGEDATA_H_
#define INCLUDE_IMAGEDATA_H_

#include <vector>
#include <include/utils.h>

class ImageData {
 public:
  friend class RWjpeg;
  friend class RWImage;

  ImageData();
  ImageData(const ImageData& image_data);
  ~ImageData();
  const char* GetFileName() const;
  int GetWidth() const;
  int GetHeight() const;
  int GetPixel(int index) const;
  void SetPixel(int index, int colour);

 private:
  bool IsIndexValid(int index) const;
  const char* m_file_name;
  int m_width;
  int m_height;
  std::vector<int>* m_data;
};

#endif  // INCLUDE_IMAGEDATA_H_
