// Copyright 2014-4 sxniu
#ifndef  INCLUDE_RWIMAGE_H_
#define  INCLUDE_RWIMAGE_H_

class ImageData;

class RWImage {
 public:
  // image_data is not empty
  explicit RWImage(ImageData* image_data);

  // image_data is empty
  RWImage(const char* file_name, ImageData* image_data);
  ~RWImage() {}
  void Save(const char* out_file_name);

 private:
  void Read();
  ImageData* m_image_data;
};

#endif  // INCLUDE_RWIMAGE_H_
