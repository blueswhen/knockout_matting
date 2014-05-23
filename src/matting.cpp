// Copyright 2014-4 sxniu
#include "include/matting.h"
#include <algorithm>
#include <iostream>
#include <utility>
#include <math.h>
#include <stdio.h>
#include "include/utils.h"
#include "include/ImageData.h"
#include "include/region_filling_by_edge_tracing.h"
#include "include/colour.h"

#define FILLING_COLOUR RED
#define SUBJECT_LINE 0xaa000000
#define BACKGROUND_LINE 0xbb000000
#define UNKNOWN_COLOUR GRAY
#define SUBJECT_COLOUR WHITE
#define BACKGROUND_COLOUR BLACK
#define SCRIBBLE_COLOUR BLUE
#define MAX_NUM 1000000

#define MAX_SEARCH_RADIUS 512

enum Scene {
  SUBJECT,
  BACKGROUND
};

namespace matting {

void SetSearchBox(int* search_array, int size, int x, int y,
                  int width, int height, int search_radius, int* search_size) {
  if (size != 8 * search_radius + 4) {
    printf("error: the size of array is wrong, size = %d, search_radius = %d\n", size, search_radius);
    return;
  }

  int y_top = std::max(y - search_radius, 0);
  int y_bottom = std::min(y + search_radius, height - 1);
  int x_left = std::max(x - search_radius, 0);
  int x_right = std::min(x + search_radius, width - 1);
  *search_size = 0;

  for (int x_pos = x_left; x_pos <= x_right; x_pos += x_right - x_left) {
    for (int y_pos = y_top; y_pos <= y_bottom; ++y_pos) {
      int index = y_pos * width + x_pos;
      search_array[(*search_size)++] = index;
    }
  }
  for (int y_pos = y_top; y_pos <= y_bottom; y_pos += y_bottom - y_top) {
    for (int x_pos = x_left + 1; x_pos <= x_right - 1; ++x_pos) {
      int index = y_pos * width + x_pos;
      search_array[(*search_size)++] = index;
    }
  }
}

void SetSearchOrderForTrace(int index[8], int previous_index_cen,
                            int index_cen, int image_width) {
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
    int size = 8 * search_radius + 4;
    int search_box[size];
    int search_size = 0;
    SetSearchBox(search_box, size, x, y, width, height, search_radius, &search_size);
    for (int i = 0; i < search_size; ++i) {
      if (!subs_found && (image->GetPixel(search_box[i]) & 0xff000000) == SUBJECT_LINE) {
        intersection_points_sub->push_back(search_box[i]);
        // image->SetPixel(search_box[i], GREEN);

        // if search_radius <= 8, we think the first intersection found is
        // the nearest sample point
        // if (search_radius <= 8) {
        //   subs_found = true;
        // }
      } else if (!backs_found && (image->GetPixel(search_box[i]) & 0xff000000) ==
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
    }
  }
}

void CollectInitialSamples(ImageData* trimap, int radius,
                           int width, int height, int line_colour, int center,
                           const std::vector<int>& intersections,
                           std::vector<std::pair<int, int> >* sample_points,
                           int* left_sample_index, int* right_sample_index,
                           int* smallest_distance, int* nearest_index) {
  *left_sample_index = intersections[0];
  for (int i = 0; i < intersections.size(); ++i) {
    int index_center = intersections[i];
    int index_cen_new = index_center;
    trimap->SetPixel(index_center, line_colour | center);
    int y_cen_distance = index_center / width - center / width;
    int x_cen_distance = abs(index_center - center - y_cen_distance * width);
    int euc_cen_distance = y_cen_distance * y_cen_distance +
                           x_cen_distance * x_cen_distance;
    if (euc_cen_distance < *smallest_distance) {
      *smallest_distance = euc_cen_distance;
      *nearest_index = sample_points->size();
    }
    std::pair<int, int> sample_first(euc_cen_distance, index_center);
    sample_points->push_back(sample_first);
    *right_sample_index = intersections[i];

    int k = 0;
    int y_ini = index_center / width;
    int x_ini = index_center - y_ini * width;
    int arround_index[8] = EIGHT_ARROUND_POSITION( x_ini, y_ini, width, height);
    while (k < MAX_NUM) {
      for (int j = 0; j < 8; ++j) {
        if (arround_index[j] > 0 && arround_index[j] < width * height) {
          int colour = trimap->GetPixel(arround_index[j]);
          int y_distance = arround_index[j] / width - center / width;
          int x_distance = abs(arround_index[j] - center - y_distance * width);
          y_distance = abs(y_distance);
          if ((colour & 0xff000000) == line_colour &&
              (colour & 0x00ffffff) != center &&
              x_distance <= radius &&
              y_distance <= radius) {
            trimap->SetPixel(arround_index[j], line_colour | center);
            int euc_distance = x_distance * x_distance + y_distance * y_distance;
            std::pair<int, int> sample(euc_distance, arround_index[j]);

            if (euc_distance < *smallest_distance) {
              *smallest_distance = euc_distance;
              *nearest_index = sample_points->size();
            }
            sample_points->push_back(sample);
            *right_sample_index = arround_index[j];
            index_cen_new = arround_index[j];
            break;
          }
        }
      }
      if (index_center == index_cen_new) {
        break;
      } else {
        SetSearchOrderForTrace(arround_index, index_center, index_cen_new, width);
        index_center = index_cen_new;
      }
      k++;
    }
  }
}

void CollectMoreSamplesFromLeftOrRight(ImageData* trimap, int width, int height, int line_colour,
                                       int smaples_number, int start_pos, int center,
                                       std::vector<std::pair<int, int> >* sample_points) {
  int index_center = start_pos;
  int index_cen_new = index_center;
  int y_ini = index_center / width;
  int x_ini = index_center - y_ini * width;
  int arround_index[8] = EIGHT_ARROUND_POSITION(x_ini, y_ini, width, height);
  for (int i = 0; i < smaples_number; ++i) {
    for (int k = 0; k < 8; ++k) {
      if (arround_index[k] > 0 && arround_index[k] < width * height) {
        int colour = trimap->GetPixel(arround_index[k]);
        int y_distance = arround_index[k] / width - center / width;
        int x_distance = abs(arround_index[k] - center - y_distance * width);
        if ((colour & 0xff000000) == line_colour &&
            (colour & 0x00ffffff) != center) {
          trimap->SetPixel(arround_index[k], line_colour | index_center);
          int euc_distance = x_distance * x_distance + y_distance * y_distance;
          std::pair<int, int> sample(euc_distance, arround_index[k]);
          sample_points->push_back(sample);
          index_cen_new = arround_index[k];
          break;
        }
      }
    }
    if (index_cen_new == index_center) {
      break;
    } else {
      SetSearchOrderForTrace(arround_index, index_center, index_cen_new, width);
      index_center = index_cen_new;
    }
  }
}

int CalcMeanValue(const ImageData& origin, const std::vector<std::pair<int, int> >& sample_points) {
  int sum_distance = 0;
  double sum_weighted[3] = {0, 0, 0};
  for (int i = 0; i < sample_points.size(); ++i) {
    sum_distance += sample_points[i].first;
    int sample[3] = GET_THREE_COORDINATE(origin.GetPixel(sample_points[i].second));
    sum_weighted[0] += sample_points[i].first * sample[0];
    sum_weighted[1] += sample_points[i].first * sample[1];
    sum_weighted[2] += sample_points[i].first * sample[2];
  }
  int colour_x = static_cast<int>(sum_weighted[0] / sum_distance);
  int colour_y = static_cast<int>(sum_weighted[1] / sum_distance);
  int colour_z = static_cast<int>(sum_weighted[2] / sum_distance);
  return TURN_COORDINATE_TO_COLOUR(colour_x, colour_y, colour_z);
}

int DoGenerateSampleColour(const ImageData& origin, ImageData* trimap, int radius,
                           int width, int height, int line_colour, int center,
                           const std::vector<int>& intersections,
                           int* nearest_dis) {
  std::vector<std::pair<int, int> > sample_points;
  int smallest_distance = MAX_NUM;
  int nearest_index = 0;
  int left_sample_index = 0;
  int right_sample_index = 0;
  CollectInitialSamples(trimap, radius, width, height, line_colour,
                        center, intersections, &sample_points,
                        &left_sample_index, &right_sample_index,
                        &smallest_distance, &nearest_index);

  smallest_distance = static_cast<int>(sqrt(smallest_distance));
  int left_more_samples_number =
    std::max(smallest_distance - nearest_index, 0);
  int right_more_samples_number = 
    std::max(smallest_distance - static_cast<int>(sample_points.size()) +
             nearest_index, 0);
  *nearest_dis = smallest_distance;

  CollectMoreSamplesFromLeftOrRight(trimap, width, height, line_colour,
                                   left_more_samples_number, left_sample_index,
                                   center, &sample_points);
  CollectMoreSamplesFromLeftOrRight(trimap, width, height, line_colour,
                                   right_more_samples_number, right_sample_index,
                                   center, &sample_points);
  return CalcMeanValue(origin, sample_points);
  // return origin->GetPixel(sample_points[nearest_index].second);
}

std::pair<int, int> GenerateSampleColourByIntersectionPoints(
                      const ImageData& origin, ImageData* trimap, int center,
                      int radius_sub, int radius_back,
                      const std::vector<int>& intersection_subs,
                      const std::vector<int>& intersection_backs,
                      int* nearest_dis_sub, int* nearest_dis_back) {
  int height = trimap->GetHeight();
  int width = trimap->GetWidth();
  int subject_sample = DoGenerateSampleColour(origin, trimap, radius_sub,
                                             width, height, SUBJECT_LINE,
                                             center, intersection_subs,
                                             nearest_dis_sub);
  int background_sample = DoGenerateSampleColour(origin, trimap, radius_back,
                                               width, height, BACKGROUND_LINE,
                                               center, intersection_backs,
                                               nearest_dis_back);
  std::pair<int, int> samples(subject_sample, background_sample);
  return samples;
}

std::pair<int, int> GenerateSubjectAndBackgroundSampleColour(
                      const ImageData& origin, ImageData* trimap, int x, int y,
                      int* nearest_dis_sub, int* nearest_dis_back) {
  std::vector<int> intersection_points_sub;
  std::vector<int> intersection_points_back;
  int center = y * trimap->GetWidth() + x;
  int radius_sub = 0;
  int radius_back = 0;
  SearchIntersectionPointsOfSubsAndBacks(trimap, x, y, &intersection_points_sub,
                                         &intersection_points_back,
                                         &radius_sub, &radius_back);
  if (intersection_points_sub.size() == 0 ||
      intersection_points_back.size() == 0) {
    printf("error: algorithm can not find sample points\n");
    // trimap->SetPixel(center, COLOUR_YELLOW);
    std::pair<int, int> err;
    return err;
  }
  return GenerateSampleColourByIntersectionPoints(
           origin, trimap, center, radius_sub, radius_back,
           intersection_points_sub, intersection_points_back,
           nearest_dis_sub, nearest_dis_back);
}

int DoRefinement(int sample_refined, int sample_reference, int current_col) {
  int ref[3] = GET_THREE_COORDINATE(sample_reference);
  int spl[3] = GET_THREE_COORDINATE(sample_refined);
  int curr[3] = GET_THREE_COORDINATE(current_col);

  int sr[3] = GET_DISTENCE(ref, spl);
  int sc[3] = GET_DISTENCE(curr, spl);
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

#if 1
  int bc[3] = GET_DIFFERENCE(curr, back);
  int bs[3] = GET_DIFFERENCE(sub, back);
  int bc_dot_bs = bc[0] * bs[0] + bc[1] * bs[1] + bc[2] * bs[2];
  int bs_dot_bs = bs[0] * bs[0] + bs[1] * bs[1] + bs[2] * bs[2];

  if (bs_dot_bs == 0) 
    return 1;
  double alpha = static_cast<double>(bc_dot_bs) / bs_dot_bs;
  if (alpha > 1) {
    alpha = 1;
  } else if (alpha < 0) {
    alpha = 0;
  }
  return alpha;
#else
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

#endif
}

void GenerateForegroundWithAlphaValue(const ImageData& origin, ImageData* trimap) {
  int height = trimap->GetHeight();
  int width = trimap->GetWidth();
  for (int y = 1; y < height - 1; ++y) {
    for (int x = 1; x < width - 1; ++x) {
      int index = y * width + x;
      int colour = trimap->GetPixel(index);

      if (colour == UNKNOWN_COLOUR) {
        int nearest_dis_sub = 0;
        int nearest_dis_back = 0;
        std::pair<int, int> sample_colour = 
          GenerateSubjectAndBackgroundSampleColour(origin, trimap, x, y,
                                                   &nearest_dis_sub, &nearest_dis_back);
        int sub_sample = sample_colour.first;
        int back_sample = sample_colour.second;
        int current_colour = origin.GetPixel(index);
        // if (nearest_dis_sub <= nearest_dis_back) {
        //   back_sample = RefinementSample(sub_sample, back_sample,
        //                                  current_colour, BACKGROUND);
        // } else {
        //   // sub_sample = RefinementSample(sub_sample, back_sample,
        //   //                               current_colour, SUBJECT);
        // }
        double alpha = CalcAlpha(sub_sample, back_sample, current_colour);
        int gray_alpha = static_cast<int>(alpha * 255);
        int alpha_colour = TURN_COORDINATE_TO_COLOUR(gray_alpha, gray_alpha, gray_alpha);
        // int col[3] = GET_THREE_COORDINATE(current_colour);
        // int r = static_cast<int>(col[0] * alpha);
        // int g = static_cast<int>(col[1] * alpha);
        // int b = static_cast<int>(col[2] * alpha);
        // int alpha_colour = TURN_COORDINATE_TO_COLOUR(r, g, b);
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
      if ((colour & 0xff000000) == SUBJECT_LINE) {
        image->SetPixel(index, WHITE);
      } else if ((colour & 0xff000000) == BACKGROUND_LINE) {
        image->SetPixel(index, BLACK);
      }
    }
  }
}

void RemoveSceneLineNearUnknownArea(const ImageData& origin, ImageData* trimap) {
  int height = trimap->GetHeight();
  int width = trimap->GetWidth();
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int index = y * width + x;
      int colour = trimap->GetPixel(index);
      if ((colour & 0xff000000) == SUBJECT_LINE) {
        trimap->SetPixel(index, origin.GetPixel(index));
      } else if ((colour & 0xff000000) == BACKGROUND_LINE) {
        trimap->SetPixel(index, BACKGROUND_COLOUR);
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
      int diff[3] = GET_DISTENCE(col, gray);
      if ( diff[0] < 100 && diff[1] < 100 && diff[2] < 100) {
        image->SetPixel(index, UNKNOWN_COLOUR);
      } else if(col[0] > 110 && col[1] > 150 && col[2] > 110) {
        image->SetPixel(index, SUBJECT_COLOUR);
      } else {
        image->SetPixel(index, BACKGROUND_COLOUR);
      }
    }
  }
}

}  // namespace matting 

