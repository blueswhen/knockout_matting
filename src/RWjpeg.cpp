// Copyright 2014-4 sxniu

#include "RWjpeg.h"
#include <stdio.h>
#include <jpeglib.h>

RWjpeg::RWjpeg(const char* file_name) 
  : m_file_name(file_name)
  , m_width(0)
  , m_height(0)
  , m_components(0)
  , m_image_data(new std::vector<int>(0)) {
  Read();
}

RWjpeg::~RWjpeg() {
  if (m_image_data != NULL) {
    delete m_image_data;
    m_image_data = NULL;
  }
}

void RWjpeg::Read() {
  struct jpeg_decompress_struct jpeg_decompress;
  struct jpeg_error_mgr decompress_error;
  jpeg_decompress.err = jpeg_std_error(&decompress_error);
  jpeg_create_decompress(&jpeg_decompress);

  FILE* in_file;
  if ((in_file = fopen(m_file_name, "rb")) == NULL) { 
    fprintf(stderr, "can't open %s\n", m_file_name);
    exit(1);
  }

  jpeg_stdio_src(&jpeg_decompress, in_file);
  jpeg_read_header(&jpeg_decompress, TRUE);
  jpeg_start_decompress(&jpeg_decompress);

  m_width = jpeg_decompress.output_width;
  m_height = jpeg_decompress.output_height;
  m_components = jpeg_decompress.output_components;

  unsigned char* buffer_src = new unsigned char[m_height * m_width * m_components];

  while (jpeg_decompress.output_scanline < m_height) {
      unsigned char* src_adress = buffer_src + m_components * m_width *
                                  jpeg_decompress.output_scanline;
      jpeg_read_scanlines(&jpeg_decompress, &src_adress, 1);
  }

  for (int y = 0; y < m_height; ++y) {
    for (int x = 0; x < m_width * m_components; x+= m_components) {
      int index = y * m_width * m_components + x;
      int colour = (static_cast<int>(buffer_src[index]) << 16) +
                   (static_cast<int>(buffer_src[index + 1]) << 8) +
                   (static_cast<int>(buffer_src[index + 2]));
      m_image_data->push_back(colour);
    }
  }

  delete [] buffer_src;
  jpeg_finish_decompress(&jpeg_decompress);
  fclose(in_file);
}

void RWjpeg::Save(const char* out_file_name) {
  struct jpeg_compress_struct jpeg_compress;
  struct jpeg_error_mgr jpeg_error;
  jpeg_compress.err = jpeg_std_error(&jpeg_error);
  jpeg_create_compress(&jpeg_compress);

  FILE * out_file;
  if ((out_file = fopen(out_file_name, "wb")) == NULL) { 
    fprintf(stderr, "can't open %s\n", out_file_name);
    exit(1);
  }
  jpeg_stdio_dest(&jpeg_compress, out_file);

  jpeg_compress.image_width = m_width;
  jpeg_compress.image_height = m_height;
  jpeg_compress.input_components = m_components;

  jpeg_compress.in_color_space = JCS_RGB; 	/* colorspace of input image */
  jpeg_set_defaults(&jpeg_compress);
  jpeg_start_compress(&jpeg_compress, TRUE);
  unsigned char* buffer_dest = new unsigned char[m_height * m_width * m_components];

  for (int y = 0; y < m_height; ++y) {
    for (int x = 0; x < m_width; ++x) {
      int buffer_index = y * m_width * m_components + x * m_components;
      int colour = (*m_image_data)[y* m_width + x];
      buffer_dest[buffer_index] =
        static_cast<unsigned char>((colour & 0x00ff0000) >> 16);
      buffer_dest[buffer_index + 1] =
        static_cast<unsigned char>((colour & 0x0000ff00) >> 8);
      buffer_dest[buffer_index + 2] =
        static_cast<unsigned char>(colour & 0x000000ff);
    }
  }

  while (jpeg_compress.next_scanline < jpeg_compress.image_height) {
      unsigned char* dest_adress = buffer_dest  + m_components *
                                   m_width * jpeg_compress.next_scanline;
      jpeg_write_scanlines(&jpeg_compress, &dest_adress, 1);
  }
  delete [] buffer_dest;
  jpeg_finish_compress(&jpeg_compress);
  fclose(out_file);
}

int RWjpeg::GetWidth() {
  return m_width;
}

int RWjpeg::GetHeight() {
  return m_height;
}

bool RWjpeg::IsIndexValid(int index) {
  if (index >= 0 && index < m_width * m_height) {
    return true;
  }
  printf("error: index is valid\n");
  return false;
}

int RWjpeg::GetPixel(int index) {
  if (IsIndexValid(index)) {
    return (*m_image_data)[index];
  }
  printf("error: pixel can not be found by the index\n");
  return 0;
}

const std::vector<int>* RWjpeg::GetImageData() {
  return m_image_data;
}

void RWjpeg::UpdateImageData(const std::vector<int>& image_data) {
  if (&image_data != m_image_data) {
    delete m_image_data;
    m_image_data = new std::vector<int>(image_data);
  }
}

void RWjpeg::SetPixel(int index, int colour) {
  if (IsIndexValid(index)) {
    (*m_image_data)[index] = colour;
    return;
  }
  printf("error: pixel can not be found by the index\n");
}

void RWjpeg::SetAlpha(const std::vector<int>& alpha_map) {
  int size = m_image_data->size();
  if (size == alpha_map.size()) {
    for (int i = 0; i < size; ++i) {
      (*m_image_data)[i] &= 0x00ffffff;
      (*m_image_data)[i] += (alpha_map[i] & 0xff000000);
    }
  }
}
