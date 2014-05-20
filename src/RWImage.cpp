// Copyright 2014-4 sxniu
#include "include/RWImage.h"
#include "include/ImageData.h"
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <opencv/highgui.h>
#include "include/colour.h"

#define COMPONENTS 3

RWImage::RWImage(ImageData* image_data)
  : m_image_data(image_data) {
  if (image_data->m_file_name == NULL) {
    printf("error: file name is null, this image data is empty\n");
    exit(1);
  }
}

RWImage::RWImage(const char* file_name, ImageData* image_data) 
  : m_image_data(image_data) {
  // image_data is empty
  if (m_image_data->m_file_name == NULL) {
    m_image_data->m_file_name = file_name;
    Read();
  } else {
    printf("error: this image data is not empty\n");
    exit(1);
  }
}

void RWImage::Read() {
  const char* file_name = m_image_data->m_file_name;
  int& width = m_image_data->m_width;
  int& height = m_image_data->m_height;
  std::vector<int>* data = m_image_data->m_data;

  IplImage* cv_image = cvLoadImage(file_name);

  width = cv_image->width;
  height = cv_image->height;

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int index = y * cv_image->widthStep + x * COMPONENTS;
      unsigned char* cv_data =
        reinterpret_cast<unsigned char*>(cv_image->imageData);
      int colour = (static_cast<int>(cv_data[index + 2]) << 16) +
                   (static_cast<int>(cv_data[index + 1]) << 8) +
                   (static_cast<int>(cv_data[index]));
      data->push_back(colour);
    }
  }
}

void RWImage::Save(const char* out_file_name) {
  int width = m_image_data->m_width;
  int height = m_image_data->m_height;
  const std::vector<int>* data = m_image_data->m_data;
  if (data == NULL) {
    printf("error: image data is empty");
    return;
  }

  CvSize size;
  size.width = width;
  size.height = height;
  IplImage* cv_image = cvCreateImage(size, 8, COMPONENTS);
  if (cv_image == NULL) {
    printf("error: the creation of cv image is failure");
    return;
  }

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int index = y * cv_image->widthStep + x * COMPONENTS;
      unsigned char* cv_data =
        reinterpret_cast<unsigned char*>(cv_image->imageData);
      int colour = (*data)[y * width + x];
      cv_data[index + 2] = static_cast<unsigned char>((colour & RED) >> 16);
      cv_data[index + 1] = static_cast<unsigned char>((colour & GREEN) >> 8);
      cv_data[index] = static_cast<unsigned char>(colour & BLUE);
    }
  }
  cvSaveImage(out_file_name, cv_image);
}
