/**
 * Copyright 2009 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Author: Bryan McQuade

#ifndef PNG_OPTIMIZER_H_
#define PNG_OPTIMIZER_H_

#include <string>

#include "base/basictypes.h"

struct png_struct_def;
typedef struct png_struct_def png_struct;
typedef png_struct* png_structp;

struct png_info_struct;
typedef struct png_info_struct png_info;
typedef png_info* png_infop;

namespace pagespeed {

namespace image_compression {

class PngOptimizer {
 public:
  PngOptimizer();
  ~PngOptimizer();

  static bool OptimizePng(const std::string& in, std::string* out);

  // Take the given input and losslessly compress it by removing
  // all unnecessary chunks, and by choosing an optimal PNG encoding.
  // @return true on success, false on failure.
  bool CreateOptimizedPng(const std::string& in, std::string* out);

 private:
  bool ReadPng(const std::string& body);
  bool WritePng(std::string* buffer);
  void CopyReadToWrite();

  png_structp read_ptr;
  png_infop read_info_ptr;
  png_structp write_ptr;
  png_infop write_info_ptr;

  DISALLOW_COPY_AND_ASSIGN(PngOptimizer);
};

}  // namespace image_compression

}  // namespace pagespeed

#endif  // PNG_OPTIMIZER_H_