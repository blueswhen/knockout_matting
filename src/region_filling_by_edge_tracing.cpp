// Copyright 2013-10 sxniu
#include "include/region_filling_by_edge_tracing.h"
#include <vector>
#include <algorithm>
#include "include/colour.h"

// using these colour in edge description region filling
#define FILLING_COL COLOUR_RED
#define BEGIN_SCAN COLOUR_YELLOW
#define SKIP_SCAN COLOUR_PURPLE
#define END_SCAN COLOUR_CYAN
#define SCANED COLOUR_GREEN
#define INSIDE_EDGE COLOUR_BLUE

namespace region_filling_by_edge_tracing {

void SetSearchOrder(int* index, int index_cen,
                    int previous_index_cen, int image_width) {
  if (previous_index_cen == index_cen + image_width - 1) {
    index[0] = index_cen + image_width + 1;
    index[1] = index_cen + 1;
    index[2] = index_cen - image_width + 1;
    index[3] = index_cen - image_width;
    index[4] = index_cen - image_width - 1;
    index[5] = index_cen + image_width;
    index[6] = index_cen - 1;
    index[7] = index_cen + image_width - 1;
  } else if (previous_index_cen == index_cen + image_width) {
    index[0] = index_cen + 1;
    index[1] = index_cen - image_width + 1;
    index[2] = index_cen - image_width;
    index[3] = index_cen - image_width - 1;
    index[4] = index_cen - 1;
    index[5] = index_cen + image_width + 1;
    index[6] = index_cen + image_width - 1;
    index[7] = index_cen + image_width;
  } else if (previous_index_cen == index_cen + image_width + 1) {
    index[0] = index_cen - image_width + 1;
    index[1] = index_cen - image_width;
    index[2] = index_cen - image_width - 1;
    index[3] = index_cen - 1;
    index[4] = index_cen + image_width - 1;
    index[5] = index_cen + 1;
    index[6] = index_cen + image_width;
    index[7] = index_cen + image_width + 1;
  } else if (previous_index_cen == index_cen - 1) {
    index[0] = index_cen + image_width;
    index[1] = index_cen + image_width + 1;
    index[2] = index_cen + 1;
    index[3] = index_cen - image_width + 1;
    index[4] = index_cen - image_width;
    index[5] = index_cen + image_width - 1;
    index[6] = index_cen - image_width - 1;
    index[7] = index_cen - 1;
  } else if (previous_index_cen == index_cen + 1) {
    index[0] = index_cen - image_width;
    index[1] = index_cen - image_width - 1;
    index[2] = index_cen - 1;
    index[3] = index_cen + image_width - 1;
    index[4] = index_cen + image_width;
    index[5] = index_cen - image_width + 1;
    index[6] = index_cen + image_width + 1;
    index[7] = index_cen + 1;
  } else if (previous_index_cen == index_cen - image_width - 1) {
    index[0] = index_cen + image_width - 1;
    index[1] = index_cen + image_width;
    index[2] = index_cen + image_width + 1;
    index[3] = index_cen + 1;
    index[4] = index_cen - image_width + 1;
    index[5] = index_cen - 1;
    index[6] = index_cen - image_width;
    index[7] = index_cen - image_width - 1;
  } else if (previous_index_cen == index_cen - image_width) {
    index[0] = index_cen - 1;
    index[1] = index_cen + image_width - 1;
    index[2] = index_cen + image_width;
    index[3] = index_cen + image_width + 1;
    index[4] = index_cen + 1;
    index[5] = index_cen - image_width - 1;
    index[6] = index_cen - image_width + 1;
    index[7] = index_cen - image_width;
  } else if (previous_index_cen == index_cen + 1 - image_width) {
    index[0] = index_cen - image_width - 1;
    index[1] = index_cen - 1;
    index[2] = index_cen + image_width - 1;
    index[3] = index_cen + image_width;
    index[4] = index_cen + image_width + 1;
    index[5] = index_cen - image_width;
    index[6] = index_cen + 1;
    index[7] = index_cen + 1 - image_width;
  } else {
    index[0] = index_cen - image_width - 1;
    index[1] = index_cen - 1;
    index[2] = index_cen + image_width - 1;
    index[3] = index_cen + image_width;
    index[4] = index_cen + image_width + 1;
    index[5] = index_cen - image_width;
    index[6] = index_cen + 1;
    index[7] = index_cen + 1 - image_width;
  }
}

int GetAngle(int index_cen, int arround_index, int image_width) {
  if (arround_index == index_cen - image_width)
    return 90;
  else if (arround_index == index_cen - image_width - 1)
    return 135;
  else if (arround_index == index_cen - 1)
    return 180;
  else if (arround_index == index_cen + image_width - 1)
    return 225;
  else if (arround_index == index_cen + image_width)
    return 270;
  else if (arround_index == index_cen + image_width + 1)
    return 315;
  else if (arround_index == index_cen + 1)
    return 0;
  else if (arround_index == index_cen + 1 - image_width)
    return 45;
  else
    return 0;
}

int FindNextPoint(int* image, int previous_index_cen, int index_cen,
                  int image_width, int* next_index_cen, int* delt_angle) {
  int findPoint = 0;
  int index[8];
  index[0] = index_cen - image_width;
  index[1] = index_cen - image_width - 1;
  index[2] = index_cen - 1;
  index[3] = index_cen + image_width - 1;
  index[4] = index_cen + image_width;
  index[5] = index_cen + image_width + 1;
  index[6] = index_cen + 1;
  index[7] = index_cen + 1 - image_width;

  if (previous_index_cen != 0)
    SetSearchOrder(index, index_cen, previous_index_cen, image_width);
  else
    return findPoint;

  int i = 0;
  while (i < 8) {
    if (image[index[i]] != 0) {
      *next_index_cen = index[i];
      findPoint = 1;
      break;
    }
    i++;
  }
#if 0
  int preAngle = GetAngle(previous_index_cen, index_cen, image_width);
  int currAngle = GetAngle(index_cen, *next_index_cen, image_width);

  if (currAngle - preAngle > 180)
    *delt_angle += currAngle - preAngle - 360;
  else if (currAngle - preAngle <= -180)
    *delt_angle += currAngle - preAngle + 360;
  else
    *delt_angle += currAngle - preAngle;
#endif
  return findPoint;
}

void SetColourForPoints(int* image, int* tmp_array,
                        int previous_index_cen,
                        int index_cen, int next_index_cen,
                        int image_width,
                        std::vector<int>* start_points,
                        std::vector<int>* end_points) {
  int y_pre = previous_index_cen / image_width;
  int y_next = next_index_cen / image_width;
  int delta_y = y_next - y_pre;

  tmp_array[index_cen] += delta_y;

  if (tmp_array[index_cen] > 0) {
    image[index_cen] = BEGIN_SCAN;
    start_points->push_back(index_cen);

  } else if (tmp_array[index_cen] == 0) {
    image[index_cen] = SKIP_SCAN;
  } else {
    image[index_cen] = END_SCAN;
    end_points->push_back(index_cen);
  }
}

void Filling(int* image, const std::vector<int>& start_points,
             const std::vector<int>& end_points,
             int clockwise, int filling_colour) {
  for (size_t i = 0; i < start_points.size(); i++) {
    if (image[start_points[i]] != BEGIN_SCAN)
      continue;

    int k = 1;
    while (true) {
      if (image[start_points[i] + (clockwise == 0 ? k : -k)] == END_SCAN) {
        break;
      } else if (image[start_points[i] + (clockwise == 0 ? k : -k)] == 0) {
        image[start_points[i] + (clockwise == 0 ? k : -k)] = filling_colour;
      } else if (image[start_points[i] + (clockwise == 0 ? k : -k)] == WHITE) {
        image[start_points[i] + (clockwise == 0 ? k : -k)] = INSIDE_EDGE;
      }
      k++;
    }
  }
}

void DoRegionFillingByEdgeTracing(int* image, int x, int y,
                                  int image_width, int image_height,
                                  int filling_colour) {
  int index_cen = y * image_width + x;
  int index[8];
  index[0] = std::max(y - 1, 0) * image_width + std::min(x + 1, image_width);
  index[1] = std::max(y - 1, 0) * image_width + x;
  index[2] = std::max(y - 1, 0) * image_width + std::max(x - 1, 0);
  index[3] = y * image_width + std::max(x - 1, 0);
  index[4] = y * image_width + std::min(x + 1, image_width);
  index[5] = std::min(y + 1, image_height) * image_width + std::min(x + 1, image_width);
  index[6] = std::min(y + 1, image_height) * image_width + x;
  index[7] = std::min(y + 1, image_height) * image_width + std::max(x - 1, 0);

  int firstPoint = 0;
  if (image[index_cen] == WHITE) {
    firstPoint = index_cen;
    int previous_index_cen = 0;
    int next_index_cen = 0;
    for (int i = 0; i < 8; i++) {
      if (image[index[i]] == WHITE) {
        previous_index_cen = index[i];
        break;
      }
    }

    std::vector<int> start_points;
    std::vector<int> end_points;
    int delt_angle = 0;
    int clockwise = 0;
    int* tmp_array = new int[image_width * image_height];
    while (previous_index_cen != 0) {
      int findPoint = FindNextPoint(image, previous_index_cen,
                                    index_cen, image_width,
                                    &next_index_cen, &delt_angle);
      if (findPoint == 1) {
        if (index_cen == firstPoint && image[next_index_cen] != WHITE) {

          Filling(image, start_points, end_points, clockwise, filling_colour);
          
          start_points.clear();
          end_points.clear();
          delt_angle = 0;
          delete [] tmp_array;
          return;
        }
        SetColourForPoints(image, tmp_array, previous_index_cen,
                           index_cen, next_index_cen, image_width,
                           &start_points, &end_points);
      } else {
        delete [] tmp_array;
        return;
      }
      previous_index_cen = index_cen;
      index_cen = next_index_cen;
    }
    delete [] tmp_array;
  }
}

void EdgeRecovery(int* image, int image_width,
                  int image_height, int filling_colour) {
  for (int y = 1; y < image_height - 1; y++) {
    for (int x = 1; x < image_width - 1; x++) {
      int index = y * image_width + x;
      if (image[index] != filling_colour && image[index] != 0) {
        image[index] = WHITE;
      }
    }
  }
}

void RegionFillingByEdgeTracing(int* image, int image_width,
                                int image_height, int filling_colour) {
  for (int y = 1; y < image_height - 1; y++) {
    for (int x = 1; x < image_width - 1; x++) {
      DoRegionFillingByEdgeTracing(image, x, y, image_width,
                                   image_height, filling_colour);
    }
  }
  EdgeRecovery(image, image_width, image_height, filling_colour);
  // utils::ShowNum(k);
}

}  // namespace region_filling_by_edge_tracing
