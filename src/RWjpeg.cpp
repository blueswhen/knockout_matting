// Copyright 2014-4 sxniu
#include "include/RWjpeg.h"
#include "include/ImageData.h"
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

#define COMPONENTS 3

RWjpeg::RWjpeg(ImageData* image_data)
  : m_image_data(image_data) {
  if (image_data->m_file_name == NULL) {
    printf("error: file name is null, this image data is empty\n");
    exit(1);
  }
}

RWjpeg::RWjpeg(const char* file_name, ImageData* image_data) 
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

void RWjpeg::Read() {
  const char* file_name = m_image_data->m_file_name;
  int& width = m_image_data->m_width;
  int& height = m_image_data->m_height;
  std::vector<int>* data = m_image_data->m_data;

  struct jpeg_decompress_struct jpeg_decompress;
  struct jpeg_error_mgr decompress_error;
  jpeg_decompress.err = jpeg_std_error(&decompress_error);
  jpeg_create_decompress(&jpeg_decompress);

  FILE* in_file;
  if ((in_file = fopen(file_name, "rb")) == NULL) { 
    fprintf(stderr, "can't open %s\n", file_name);
    exit(1);
  }

  jpeg_stdio_src(&jpeg_decompress, in_file);
  jpeg_read_header(&jpeg_decompress, TRUE);
  jpeg_start_decompress(&jpeg_decompress);

  width = jpeg_decompress.output_width;
  height = jpeg_decompress.output_height;

  unsigned char* buffer_src = new unsigned char[height * width * COMPONENTS];

  while (jpeg_decompress.output_scanline < height) {
      unsigned char* src_adress = buffer_src + COMPONENTS * width *
                                  jpeg_decompress.output_scanline;
      jpeg_read_scanlines(&jpeg_decompress, &src_adress, 1);
  }

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width * COMPONENTS; x += COMPONENTS) {
      int index = y * width * COMPONENTS + x;
      int colour = (static_cast<int>(buffer_src[index]) << 16) +
                   (static_cast<int>(buffer_src[index + 1]) << 8) +
                   (static_cast<int>(buffer_src[index + 2]));
      data->push_back(colour);
    }
  }

  delete [] buffer_src;
  jpeg_finish_decompress(&jpeg_decompress);
  fclose(in_file);
}

void RWjpeg::Save(const char* out_file_name) {
  int width = m_image_data->m_width;
  int height = m_image_data->m_height;
  const std::vector<int>* data = m_image_data->m_data;
  if (data == NULL) {
    printf("error: image data is empty");
    return;
  }

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

  jpeg_compress.image_width = width;
  jpeg_compress.image_height = height;
  jpeg_compress.input_components = COMPONENTS;

  jpeg_compress.in_color_space = JCS_RGB; 	/* colorspace of input image */
  jpeg_set_defaults(&jpeg_compress);
  jpeg_start_compress(&jpeg_compress, TRUE);
  unsigned char* buffer_dest = new unsigned char[height * width * COMPONENTS];

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int buffer_index = y * width * COMPONENTS + x * COMPONENTS;
      int colour = (*data)[y* width + x];
      buffer_dest[buffer_index] =
        static_cast<unsigned char>((colour & 0x00ff0000) >> 16);
      buffer_dest[buffer_index + 1] =
        static_cast<unsigned char>((colour & 0x0000ff00) >> 8);
      buffer_dest[buffer_index + 2] =
        static_cast<unsigned char>(colour & 0x000000ff);
    }
  }

  while (jpeg_compress.next_scanline < height) {
      unsigned char* dest_adress = buffer_dest  + COMPONENTS *
                                   width * jpeg_compress.next_scanline;
      jpeg_write_scanlines(&jpeg_compress, &dest_adress, 1);
  }
  delete [] buffer_dest;
  jpeg_finish_compress(&jpeg_compress);
  fclose(out_file);
}
