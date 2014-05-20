// Copyright 2014-4 sxniu
#include "include/utils.h"
#include <algorithm>
#include <iostream>
#include <utility>
#include <math.h>
#include <stdio.h>
#include "include/ImageData.h"
#include "include/region_filling_by_edge_tracing.h"
#include "include/colour.h"

#define FILLING_COLOUR RED
#define SUBJECT_LINE RED
#define BACKGROUND_LINE BLUE
#define UNKNOWN_COLOUR GRAY
#define SUBJECT_COLOUR WHITE
#define BACKGROUND_COLOUR BLACK
#define SCRIBBLE_COLOUR BLUE
#define PASS_COLOUR 0xff000000 
#define MAX_NUM 10000

#define MAX_SEARCH_RADIUS 512

enum Scene {
  SUBJECT,
  BACKGROUND
};

#define GET_THREE_COORDINATE(colour) \
{ \
  (colour & RED) >> 16, \
  (colour & GREEN) >> 8, \
  colour & BLUE \
}

#define TURN_COORDINATE_TO_COLOUR(x, y, z) ((x << 16) + (y << 8) + z)

#define GET_DIFFERENCE(d, s) \
{ \
  abs(d[0] - s[0]), \
  abs(d[1] - s[1]), \
  abs(d[2] - s[2]), \
}

namespace utils {

void SetSearchBox(int* search_array, int size, int x, int y,
                  int width, int height, int search_radius, int* search_size) {
  if (size != 8 * search_radius) {
    printf("error: the size of array is wrong, size = %d, search_radius = %d\n", size, search_radius);
    return;
  }

  int y_top = std::max(y - search_radius, 0);
  int y_bottom = std::min(y + search_radius, height);
  int x_left = std::max(x - search_radius, 0);
  int x_right = std::min(x + search_radius, width);
  *search_size = 0;

  for (int x_pos = x_left; x_pos < x_right; x_pos += x_right - x_left - 1) {
    for (int y_pos = y_top; y_pos < y_bottom; ++y_pos) {
      int index = y_pos * width + x_pos;
      search_array[(*search_size)++] = index;
    }
  }
  for (int y_pos = y_top; y_pos < y_bottom; y_pos += y_bottom - y_top - 1) {
    for (int x_pos = x_left; x_pos < x_right; ++x_pos) {
      int index = y_pos * width + x_pos;
      search_array[(*search_size)++] = index;
    }
  }
}

void SearchIntersectionPointsOfSubsAndBacks(
  ImageData* image, int x, int y, std::vector<int>* intersection_points_sub,
  std::vector<int>* intersection_points_back,
  int* radius_sub, int* radius_back) {
  int height = image->GetHeight();
  int width = image->GetWidth();
  bool subs_found = false;
  bool backs_found = false;
  if (intersection_points_sub->size() != 0 ||
      intersection_points_back->size() != 0) {
    printf("error, the vector is not empty\n");
    return;
  }

  for (int search_radius = 1; search_radius <= MAX_SEARCH_RADIUS; search_radius *= 2) {
    int size = 8 * search_radius;
    int search_box[size];
    int search_size = 0;
    SetSearchBox(search_box, size, x, y, width, height, search_radius, &search_size);
    for (int i = 0; i < search_size; ++i) {
      if (!subs_found && (image->GetPixel(search_box[i]) & 0x00ffffff) == SUBJECT_LINE) {
        intersection_points_sub->push_back(search_box[i]);
        // image->SetPixel(search_box[i], GREEN);

        // if search_radius <= 8, we think the first intersection found is
        // the nearest sample point
        // if (search_radius <= 8) {
        //   subs_found = true;
        // }
      } else if (!backs_found && (image->GetPixel(search_box[i]) & 0x00ffffff) ==
                 BACKGROUND_LINE) {
        intersection_points_back->push_back(search_box[i]);
        // image->SetPixel(y * width + x, COLOUR_YELLOW);

        // if (search_radius <= 8) {
        //   backs_found = true;
        // }
      }
      if (subs_found && backs_found) {
        break;
      }
    }
    if (!subs_found && intersection_points_sub->size() != 0) {
      *radius_sub = search_radius;
      subs_found = true;
    }
    if (!backs_found && intersection_points_back->size() != 0) {
      *radius_back = search_radius;
      backs_found = true;
    }
    if (subs_found && backs_found) {
      // for (int i = 0; i < search_size; ++i) {
      //   int colour = image->GetPixel(search_box[i]);
      //   if (colour != GREEN && colour != COLOUR_YELLOW) {
      //     image->SetPixel(search_box[i], COLOUR_PURPLE);
      //   }
      // }
      break;
    }
  }
}

void CollectInitialSamples(ImageData* origin, ImageData* trimap, int radius,
                           int width, int height, int line_colour, int center,
                           const std::vector<int>& intersections,
                           std::vector<std::pair<int, int> >* sample_points,
                           int* left_sample_index, int* right_sample_index,
                           int* smallest_distance, int* nearest_index) {
  *left_sample_index = intersections[0];
  for (int i = 0; i < intersections.size(); ++i) {
    int index_center = intersections[i];
    int index_cen_new = index_center;
    trimap->SetPixel(index_center, PASS_COLOUR | line_colour);
    int x_cen_distance = abs(index_center - center) % width;
    int y_cen_distance = abs(index_center - center) / width;
    int euc_cen_distance = y_cen_distance * y_cen_distance +
                           x_cen_distance * x_cen_distance;
    std::pair<int, int> sample_first(euc_cen_distance, origin->GetPixel(index_center));
    sample_points->push_back(sample_first);

    int k = 0;
    while (k < MAX_NUM) {
      int index_cen_y = index_center / width;
      int index_cen_x = index_center - index_cen_y * width;
      int arround_index[8] = EIGHT_ARROUND_POSITION(
                               index_cen_x, index_cen_y, width, height);

      for (int j = 0; j < 8; ++j) {
        if (arround_index[j] > 0 && arround_index[j] < width * height) {
          int colour = trimap->GetPixel(arround_index[j]);
          int x_distance = abs(arround_index[j] - center) % width;
          int y_distance = abs(arround_index[j] - center) / width;
          if (colour == line_colour &&
              x_distance <= radius &&
              y_distance <= radius) {
            trimap->SetPixel(arround_index[j], PASS_COLOUR | line_colour);
            int euc_distance = x_distance * x_distance + y_distance * y_distance;
            std::pair<int, int> sample(euc_distance,
                                       origin->GetPixel(arround_index[j]));

            sample_points->push_back(sample);
            *right_sample_index = arround_index[j];

            if (euc_distance < *smallest_distance) {
              *smallest_distance = euc_distance;
              *nearest_index = sample_points->size();
            }
            index_cen_new = arround_index[j];
            break;
          }
        }
      }
      if (index_center == index_cen_new) {
        break;
      } else {
        index_center = index_cen_new;
      }
      k++;
    }
  }
}

void CollectMoreSamplesFromLeftOrRight(ImageData* origin, ImageData* trimap,
                                   int width, int height, int line_colour,
                                   int smaples_number, int start_pos, 
                                   std::vector<std::pair<int, int> >* sample_points) {
  int index_center = start_pos;
  int index_cen_new = index_center;
  for (int i = 0; i < smaples_number; ++i) {
    int index_cen_y = index_center / width;
    int index_cen_x = index_center - index_cen_y * width;
    int arround_index[8] = EIGHT_ARROUND_POSITION(index_cen_x, index_cen_y, width, height);
    for (int k = 0; k < 8; ++k) {
      if (arround_index[k] > 0 && arround_index[k] < width * height) {
        int colour = trimap->GetPixel(arround_index[k]);
        int x_distance = abs(arround_index[k] - index_center) % width;
        int y_distance = abs(arround_index[k] - index_center) / width;
        if (colour == line_colour) {
          trimap->SetPixel(arround_index[k], PASS_COLOUR | line_colour);
          int euc_distance = x_distance * x_distance + y_distance * y_distance;
          std::pair<int, int> sample(euc_distance, origin->GetPixel(arround_index[i]));
          sample_points->push_back(sample);
          index_cen_new = arround_index[i];
          break;
        }
      }
    }
    if (index_cen_new == index_center) {
      break;
    } else {
      index_center = index_cen_new;
    }
  }
}

int CalcMeanValue(const std::vector<std::pair<int, int> >& sample_points) {
  int sum_distance = 0;
  double sum_weighted[3] = {0, 0, 0};
  for (int i = 0; i < sample_points.size(); ++i) {
    sum_distance += sample_points[i].first;
    int sample[3] = GET_THREE_COORDINATE(sample_points[i].second);
    sum_weighted[0] += sample_points[i].first * sample[0];
    sum_weighted[1] += sample_points[i].first * sample[1];
    sum_weighted[2] += sample_points[i].first * sample[2];
  }
  int colour_x = static_cast<int>(sum_weighted[0] / sum_distance);
  int colour_y = static_cast<int>(sum_weighted[1] / sum_distance);
  int colour_z = static_cast<int>(sum_weighted[2] / sum_distance);
  return TURN_COORDINATE_TO_COLOUR(colour_x, colour_y, colour_z);
}

int DoGenerateSampleColour(ImageData* origin, ImageData* trimap, int radius,
                           int width, int height, int line_colour, int center,
                           const std::vector<int>& intersections) {
  std::vector<std::pair<int, int> > sample_points;
  int smallest_distance = MAX_NUM;
  int nearest_index = 0;
  int left_sample_index = 0;
  int right_sample_index = 0;
  CollectInitialSamples(origin, trimap, radius, width, height, line_colour,
                        center, intersections, &sample_points,
                        &left_sample_index, &right_sample_index,
                        &smallest_distance, &nearest_index);

  smallest_distance = static_cast<int>(sqrt(smallest_distance));
  int left_more_samples_number =
    std::max(smallest_distance - nearest_index, 0);
  int right_more_samples_number = 
    std::max(smallest_distance - static_cast<int>(sample_points.size()) +
             nearest_index, 0);

  CollectMoreSamplesFromLeftOrRight(origin, trimap, width, height, line_colour,
                                   left_more_samples_number, left_sample_index,
                                   &sample_points);
  CollectMoreSamplesFromLeftOrRight(origin, trimap, width, height, line_colour,
                                   right_more_samples_number, right_sample_index,
                                   &sample_points);
  return CalcMeanValue(sample_points);
  // return sample_points[nearest_index].second;
}

std::pair<int, int> GenerateSampleColourByIntersectionPoints(
                      ImageData* origin, ImageData* trimap, int center,
                      int radius_sub, int radius_back,
                      const std::vector<int>& intersection_subs,
                      const std::vector<int>& intersection_backs) {
  int height = trimap->GetHeight();
  int width = trimap->GetWidth();
  int subject_sample = DoGenerateSampleColour(origin, trimap, radius_sub,
                                              width, height, SUBJECT_LINE,
                                              center, intersection_subs);
  int background_sample = DoGenerateSampleColour(origin, trimap, radius_back,
                                                 width, height, BACKGROUND_LINE,
                                                 center, intersection_backs);
  std::pair<int, int> samples(subject_sample, background_sample);
  return samples;
}

std::pair<int, int> GenerateSubjectAndBackgroundSampleColour(
                      ImageData* origin, ImageData* trimap, int x, int y,
                      int* radius_sub, int* radius_back) {
  std::vector<int> intersection_points_sub;
  std::vector<int> intersection_points_back;
  int center = y * trimap->GetWidth() + x;
  SearchIntersectionPointsOfSubsAndBacks(trimap, x, y, &intersection_points_sub,
                                         &intersection_points_back,
                                         radius_sub, radius_back);
  if (intersection_points_sub.size() == 0 ||
      intersection_points_back.size() == 0) {
    printf("error: algorithm can not find sample points\n");
    // trimap->SetPixel(center, COLOUR_YELLOW);
    std::pair<int, int> err;
    return err;
  }
  return GenerateSampleColourByIntersectionPoints(
           origin, trimap, center, *radius_sub, *radius_back,
           intersection_points_sub, intersection_points_back);
}

int DoRefinement(int sample_refined, int sample_reference, int current_col) {
  int ref[3] = GET_THREE_COORDINATE(sample_reference);
  int spl[3] = GET_THREE_COORDINATE(sample_refined);
  int curr[3] = GET_THREE_COORDINATE(current_col);

  int sr[3] = GET_DIFFERENCE(ref, spl);
  int sc[3] = GET_DIFFERENCE(curr, spl);
  if (sr[0] == 0 && sr[1] == 0 && sr[2] == 0) {
    return sample_refined;
  }

  double k = static_cast<double>(sr[0] * sc[0] + sr[1] * sc[1] + sr[2] * sc[2]) /
             (sr[0] * sr[0] + sr[1] * sr[1] + sr[2] * sr[2]);
  int rfsample_current[3] = {k * sr[0], k * sr[1], k * sr[2]};
  int rfsample[3] = {curr[0] - rfsample_current[0], curr[1] - rfsample_current[1],
                     curr[2] - rfsample_current[2]};
  return TURN_COORDINATE_TO_COLOUR(rfsample[0], rfsample[1], rfsample[2]);
}

int RefinementSample(int sub_sample, int back_sample,
                     int current_colour, Scene obj_refined) {
  if (obj_refined == BACKGROUND) {
    return DoRefinement(back_sample, sub_sample, current_colour);
  } else {
    return DoRefinement(sub_sample, back_sample, current_colour);
  }
}

double CalcAlpha(int sub_sample, int back_sample, int current_colour) {
  int sub[3] = GET_THREE_COORDINATE(sub_sample);
  int back[3] = GET_THREE_COORDINATE(back_sample);
  int curr[3] = GET_THREE_COORDINATE(current_colour);

  int c_b[3] = GET_DIFFERENCE(curr, back);
  int s_b[3] = GET_DIFFERENCE(sub, back);
  double alpha[3];
  for (int i = 0; i < 3; ++i) {
    if (s_b[i] != 0) {
      alpha[i] = static_cast<double>(c_b[i]) / s_b[i];
    } else {
      alpha[i] = 1;
    }
  }
  double alpha_v = (alpha[0] * s_b[0] + alpha[1] * s_b[1] + alpha[2] * s_b[2]) /
                   (s_b[0] + s_b[1] + s_b[2]);

  return alpha_v < 1 ? alpha_v : 1;
  // if (alpha_v > 1) {
  //   int bc[3] = {curr[0] - back[0], curr[1] - back[0], curr[2] - back[2]};
  //   int bs[3] = {sub[0] - back[0], sub[1] - back[0], sub[2] - back[2]};
  //   int bc_dot_bs = bc[0] * bs[0] + bc[1] * bs[1] + bc[2] * bs[2];
  //   if (bc_dot_bs < 0) {
  //     alpha_v = 0;
  //   } else {
  //     alpha_v = 1;
  //   }
  // }
  // return alpha_v;
}

void GenerateForegroundWithAlphaValue(ImageData* origin, ImageData* trimap) {
  int height = trimap->GetHeight();
  int width = trimap->GetWidth();
  for (int y = 1; y < height - 1; ++y) {
    for (int x = 1; x < width - 1; ++x) {
      int index = y * width + x;
      int colour = trimap->GetPixel(index);

      if (colour == UNKNOWN_COLOUR) {
        int radius_sub = 0;
        int radius_back = 0;
        std::pair<int, int> sample_colour = 
          GenerateSubjectAndBackgroundSampleColour(origin, trimap, x, y,
                                                   &radius_sub, &radius_back);
        int sub_sample = sample_colour.first;
        int back_sample = sample_colour.second;
        int current_colour = origin->GetPixel(index);
        if (radius_sub <= radius_back) {
          back_sample = RefinementSample(sub_sample, back_sample,
                                         current_colour, BACKGROUND);
        } else {
          sub_sample = RefinementSample(sub_sample, back_sample,
                                        current_colour, SUBJECT);
        }
        double alpha = CalcAlpha(sub_sample, back_sample, current_colour);
        int alpha_colour = (static_cast<int>(alpha * 255) << 16) +
                           (static_cast<int>(alpha  * 255) << 8) +
                           static_cast<int>(alpha * 255);
        printf("%d,%d alpha = %f\n", x, y, alpha);
        // trimap->SetPixel(index, bc_v);
        trimap->SetPixel(index, alpha_colour);
      }
    }
  }
}

void SetSceneLineNearUnknownArea(ImageData* image) {
  int height = image->GetHeight();
  int width = image->GetWidth();
  for (int y = 1; y < height - 1; ++y) {
    for (int x = 1; x < width - 1; ++x) {
      int index = y * width + x;
      int colour = image->GetPixel(index);
      if (colour == UNKNOWN_COLOUR) {
        int arround_index[8] = EIGHT_ARROUND_POSITION(x, y, width, height);
        for (int i = 0; i < 8; ++i) {
          int arround_colour = image->GetPixel(arround_index[i]);
          if (arround_colour == SUBJECT_COLOUR) {
            image->SetPixel(arround_index[i], SUBJECT_LINE);
          } else if (arround_colour == BACKGROUND_COLOUR) {
            image->SetPixel(arround_index[i], BACKGROUND_LINE);
          }
        }
      }
    }
  }
}

void RemoveSceneLineNearUnknownArea(ImageData* image) {
  int height = image->GetHeight();
  int width = image->GetWidth();
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int index = y * width + x;
      int colour = image->GetPixel(index);
      if ((colour & 0x00ffffff) == SUBJECT_LINE) {
        image->SetPixel(index, WHITE);
      } else if ((colour & 0x00ffffff) == BACKGROUND_LINE) {
        image->SetPixel(index, BLACK);
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
      int colour = image->GetPixel(index);
      int red = (colour & RED) >> 16;
      int green = (colour & GREEN) >> 8;
      int blue = colour & BLUE;
      if (colour == SCRIBBLE_COLOUR) {
      // filter pure blue colour
      // if (red < 50 && green < 50 && blue < 50) {
        image->SetPixel(index, WHITE);
      } else {
        image->SetPixel(index, BLACK);
      }
    }
  }
}

void GetTrimapWithScribble(ImageData* image) {
  GetBand(image);
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
        image->SetPixel(index, UNKNOWN_COLOUR);
      } else if(image_data[index] == FILLING_COLOUR) {
        image->SetPixel(index, SUBJECT_COLOUR);
      } else {
        image->SetPixel(index, BACKGROUND_COLOUR);
      }
    }
  }
  delete [] image_data;
}

void GetTrimap(ImageData* image) {
  int height = image->GetHeight();
  int width = image->GetWidth();

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int index = y * width + x;
      int colour = image->GetPixel(index);
      int col[3] = GET_THREE_COORDINATE(colour);
      int gray[3] = GET_THREE_COORDINATE(GRAY);
      int diff[3] = GET_DIFFERENCE(col, gray);
      if ( diff[0] < 50 && diff[1] < 50 && diff[2] < 50) {
        image->SetPixel(index, UNKNOWN_COLOUR);
      } else if(col[0] > 100 && col[1] > 100 && col[2] > 100) {
        image->SetPixel(index, SUBJECT_COLOUR);
      } else {
        image->SetPixel(index, BACKGROUND_COLOUR);
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
      int red = (image->GetPixel(index) & RED) >> 16;
      int green = (image->GetPixel(index) & GREEN) >> 8;
      int blue = image->GetPixel(index) & BLUE;
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

