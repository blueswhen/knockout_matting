// Copyright 2014-4 sxniu
#ifndef  INCLUDE_RWJPEG_H_
#define  INCLUDE_RWJPEG_H_

class ImageData;

class RWjpeg {
 public:
  // image_data is not empty
  explicit RWjpeg(ImageData* image_data);

  // image_data is empty
  RWjpeg(const char* file_name, ImageData* image_data);
  ~RWjpeg() {}
  void Save(const char* out_file_name);

 private:
  void Read();
  ImageData* m_image_data;
};

#endif  // INCLUDE_RWJPEG_H_
