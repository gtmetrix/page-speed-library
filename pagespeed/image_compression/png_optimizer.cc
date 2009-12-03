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

#include "pagespeed/image_compression/png_optimizer.h"

#include <string>

extern "C" {
#include "third_party/libpng/png.h"
#include "third_party/optipng/src/opngreduc.h"
}

namespace {

struct PngInput {
  const std::string* data_;
  int offset_;
};

void ReadPngFromStream(png_structp read_ptr,
                       png_bytep data,
                       png_size_t length) {
  PngInput* input = reinterpret_cast<PngInput*>(read_ptr->io_ptr);
  int copied = input->data_->copy(reinterpret_cast<char*>(data), length,
                                  input->offset_);
  input->offset_ += copied;
  if (copied < length) {
    png_error(read_ptr, "ReadPngFromStream: Unexpected EOF.");
  }
}

void WritePngToString(png_structp write_ptr,
                      png_bytep data,
                      png_size_t length) {
  std::string& buffer = *reinterpret_cast<std::string*>(write_ptr->io_ptr);
  buffer.append(reinterpret_cast<char*>(data), length);
}

// no-op
void PngFlush(png_structp write_ptr) {}

}  // namespace

namespace pagespeed {

namespace image_compression {

PngReaderInterface::~PngReaderInterface() {
}

PngOptimizer::PngOptimizer() {
  read_ptr_ = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                     NULL, NULL, NULL);
  read_info_ptr_ = png_create_info_struct(read_ptr_);
  write_ptr_ = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                       NULL, NULL, NULL);
  write_info_ptr_ = png_create_info_struct(write_ptr_);
}

PngOptimizer::~PngOptimizer() {
  png_destroy_read_struct(&read_ptr_, &read_info_ptr_, NULL);
  png_destroy_write_struct(&write_ptr_, &write_info_ptr_);
}

bool PngOptimizer::CreateOptimizedPng(PngReaderInterface& reader,
                                      const std::string& in,
                                      std::string* out) {
  // Configure error handlers.
  if (setjmp(read_ptr_->jmpbuf)) {
    return false;
  }

  if (setjmp(write_ptr_->jmpbuf)) {
    return false;
  }

  if (!reader.ReadPng(in, read_ptr_, read_info_ptr_)) {
    return false;
  }

  if (!opng_validate_image(read_ptr_, read_info_ptr_)) {
    return false;
  }

  // Copy the image data from the read structures to the write structures.
  CopyReadToWrite();

  // Perform all possible lossless image reductions
  // (e.g. RGB->palette, etc).
  opng_reduce_image(write_ptr_, write_info_ptr_, OPNG_REDUCE_ALL);

  // TODO: try a few different strategies and pick the best one.
  png_set_compression_level(write_ptr_, Z_BEST_COMPRESSION);
  png_set_compression_mem_level(write_ptr_, 8);
  png_set_compression_strategy(write_ptr_, Z_DEFAULT_STRATEGY);
  png_set_filter(write_ptr_, PNG_FILTER_TYPE_BASE, PNG_FILTER_NONE);
  png_set_compression_window_bits(write_ptr_, 9);

  if (!WritePng(out)) {
    return false;
  }

  return true;
}

bool PngOptimizer::OptimizePng(PngReaderInterface& reader,
                               const std::string& in,
                               std::string* out) {
  PngOptimizer o;
  return o.CreateOptimizedPng(reader, in, out);
}

PngReader::~PngReader() {
}

bool PngReader::ReadPng(const std::string& body,
                        png_structp png_ptr,
                        png_infop info_ptr) {
  // Wrap the resource's response body in a structure that keeps a
  // pointer to the body and a read offset, and pass a pointer to this
  // object as the user data to be received by the PNG read function.
  PngInput input;
  input.data_ = &body;
  input.offset_ = 0;
  png_set_read_fn(png_ptr, &input, &ReadPngFromStream);
  png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

  return true;
}

bool PngOptimizer::WritePng(std::string* buffer) {
  png_set_write_fn(write_ptr_, buffer, &WritePngToString, &PngFlush);
  png_write_png(write_ptr_, write_info_ptr_, PNG_TRANSFORM_IDENTITY, NULL);

  return true;
}

void PngOptimizer::CopyReadToWrite() {
  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type, compression_type, filter_type;
  png_get_IHDR(read_ptr_,
               read_info_ptr_,
               &width,
               &height,
               &bit_depth,
               &color_type,
               &interlace_type,
               &compression_type,
               &filter_type);

  png_set_IHDR(write_ptr_,
               write_info_ptr_,
               width,
               height,
               bit_depth,
               color_type,
               interlace_type,
               compression_type,
               filter_type);

  png_bytepp row_pointers = png_get_rows(read_ptr_, read_info_ptr_);
  png_set_rows(write_ptr_, write_info_ptr_, row_pointers);

  png_colorp palette;
  int num_palette;
  if (png_get_PLTE(read_ptr_, read_info_ptr_, &palette, &num_palette) != 0) {
    png_set_PLTE(write_ptr_,
                 write_info_ptr_,
                 palette,
                 num_palette);
  }

  // Transparency is not considered metadata, although tRNS is
  // ancillary.
  png_bytep trans;
  int num_trans;
  png_color_16p trans_values;
  if (png_get_tRNS(read_ptr_,
                   read_info_ptr_,
                   &trans,
                   &num_trans,
                   &trans_values) != 0) {
    png_set_tRNS(write_ptr_,
                 write_info_ptr_,
                 trans,
                 num_trans,
                 trans_values);
  }

  double gamma;
  if (png_get_gAMA(read_ptr_, read_info_ptr_, &gamma) != 0) {
    png_set_gAMA(write_ptr_, write_info_ptr_, gamma);
  }

  // Do not copy bkgd, hist or sbit sections, since they are not
  // supported in most browsers.
}

}  // namespace image_compression

}  // namespace pagespeed
