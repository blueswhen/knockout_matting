// Copyright 2014-4 sxniu
#include "include/utils.h"
#include <stdio.h>
#include "include/ImageData.h"
#include "include/region_filling_by_edge_tracing.h"
#include "include/colour.h"

#define FILLING_COLOUR RED

namespace utils {

void GetTrimap(ImageData* image) {
  int height = image->GetHeight();
  int width = image->GetWidth();

  int* image_data = new int[width * height];
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int index = y * width + x;
      image_data[index] = image->GetPixel(index);
    }
  }

  region_filling_by_edge_tracing::RegionFillingByEdgeTracing(
    image_data, width, height, FILLING_COLOUR);

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int index = y * width + x;
      if (image_data[index] == WHITE) {
        image->SetPixel(index, GRAY);
      } else if(image_data[index] == FILLING_COLOUR) {
        image->SetPixel(index, WHITE);
      }
    }
  }
}

void GetBand(ImageData* image) {
  int height = image->GetHeight();
  int width = image->GetWidth();
  for (int y = 0; y < height; ++y) {
    for (int x  = 0; x < width; ++x) {
      int index = y * width + x;
      int red = (image->GetPixel(index) & RED) >> 16;
      int green = (image->GetPixel(index) & GREEN) >> 8;
      int blue = image->GetPixel(index) & BLUE;
      if (red < 60 && green < 60 && blue > 180) {
        image->SetPixel(index, WHITE);
      } else {
        image->SetPixel(index, BLACK);
      }
    }
  }
}

void TurnGray(ImageData* image) {
  int height = image->GetHeight();
  int width = image->GetWidth();
  for (int y = 0; y < height; ++y) {
    for (int x  = 0; x < width; ++x) {
      int index = y * width + x;
      int red = (image->GetPixel(index) & 0x00ff0000) >> 16;
      int green = (image->GetPixel(index) & 0x0000ff00) >> 8;
      int blue = image->GetPixel(index) & 0x000000ff;
      int gray = red * 0.3 + green * 0.59 + blue * 0.11;
      int colour = (gray << 16) + (gray << 8) + gray;
      image->SetPixel(index, colour);
    }
  }
}

void CreateAlphaForTrimap(ImageData* trimap) {
  TurnGray(trimap);
  int height = trimap->GetHeight();
  int width = trimap->GetWidth();
  for (int y = 0; y < height; ++y) {
    for (int x  = 0; x < width; ++x) {
      int index = y * width + x;
      int colour = trimap->GetPixel(index) & 0x00ffffff;
      int gray = colour & 0x000000ff;
      if (gray < 100) {
        trimap->SetPixel(index, RED);
      } else if (gray >= 100 && gray <= 200) {
        trimap->SetPixel(index, GREEN | 0x7f000000);
      } else {
        trimap->SetPixel(index, BLUE | 0xff000000);
      }
    }
  }
}

void SetAlphaForImage(const ImageData& alpha_map, ImageData* image_data) {
  int data_width = image_data->GetWidth();
  int data_height = image_data->GetHeight();
  int map_width = alpha_map.GetWidth();
  int map_height = alpha_map.GetHeight();

  if (data_width == map_width && data_height == map_height) {
    for (int i = 0; i < data_height * data_width; ++i) {
      int colour = image_data->GetPixel(i) & 0x00ffffff;
      int map_alpha = alpha_map.GetPixel(i) & 0xff000000;
      image_data->SetPixel(i, colour | map_alpha);
    }
  } else {
    printf("the map size is not matched");
  }
}

void ShowAlpha(ImageData* image) {
  int height = image->GetHeight();
  int width = image->GetWidth();
  for (int y = 0; y < height; ++y) {
    for (int x  = 0; x < width; ++x) {
      int index = y * width + x;
      int colour = image->GetPixel(index);
      if ((colour & 0xff000000) == 0) {
        image->SetPixel(index, colour & RED);
      } else if ((colour & 0xff000000) == 0x7f000000) {
        image->SetPixel(index, colour & GREEN);
      } else {
        image->SetPixel(index, colour & BLUE);
      }
    }
  }
}


}  // namespace utils

