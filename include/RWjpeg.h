// Copyright 2014-4 sxniu
#ifndef  INCLUDE_RWJPEG_H_
#define  INCLUDE_RWJPEG_H_
#include <vector>
#include <stdlib.h>

class RWjpeg {
 public:
  RWjpeg(const char* file_name);
  ~RWjpeg();
  void Save(const char* out_file_name);
  int GetWidth();
  int GetHeight();
  int GetPixel(int index);
  const std::vector<int>* GetImageData();
  void UpdateImageData(const std::vector<int>& image_data);
  void SetPixel(int index, int colour);
  void SetAlpha(const std::vector<int>& alpha_map);

 private:
  void Read();
  bool IsIndexValid(int index);
  const char* m_file_name;
  int m_width;
  int m_height;
  int m_components;
  std::vector<int>* m_image_data;
};

#endif  // INCLUDE_RWJPEG_H_
