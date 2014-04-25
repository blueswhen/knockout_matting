// Copyright 2014-4 sxniu
#include "include/ImageData.h"
#include <stdio.h>

ImageData::ImageData()
  : m_file_name(NULL)
  , m_width(0)
  , m_height(0)
  , m_data(new std::vector<int>(0)) {}

ImageData::ImageData(const ImageData& image_data)
  : m_file_name(image_data.m_file_name)
  , m_width(image_data.m_width)
  , m_height(image_data.m_height)
  , m_data(new std::vector<int>(*(image_data.m_data))) {}

ImageData::~ImageData() {
  if (m_data != NULL) {
    delete m_data;
    m_data = NULL;
  }
}

const char* ImageData::GetFileName() {
  return m_file_name;
}

int ImageData::GetWidth() {
  return m_width;
}

int ImageData::GetHeight() {
  return m_height;
}

int ImageData::GetPixel(int index) {
  if (IsIndexValid(index)) {
    return (*m_data)[index];
  }
  printf("error: get pixel failed\n");
  return 0;
}

void ImageData::SetPixel(int index, int colour) {
  if (IsIndexValid(index)) {
    (*m_data)[index] = colour;
    return;
  }
  printf("error: set pixel failed\n");
}

bool ImageData::IsIndexValid(int index) {
  if (index > 0 && index < m_width * m_height) {
    return true;
  }
  printf("error: the index is invalid\n");
  return false;
}
