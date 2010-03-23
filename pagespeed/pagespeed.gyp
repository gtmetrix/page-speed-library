# Copyright 2009 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

{
  'variables': {
    'pagespeed_root': '..'
  },
  'targets': [
    {
      'target_name': 'pagespeed_input_pb',
      'type': '<(library)',
      'hard_dependency': 1,
      'dependencies': [
          '<(DEPTH)/third_party/protobuf2/protobuf.gyp:protobuf',
          '<(DEPTH)/third_party/protobuf2/protobuf.gyp:protoc',
       ],
      'actions': [
        {
          'action_name': 'my_proto',
          'inputs': [
            '<(PRODUCT_DIR)/<(EXECUTABLE_PREFIX)protoc<(EXECUTABLE_SUFFIX)',
            '<(pagespeed_root)/pagespeed/proto/pagespeed_input.proto',
          ],
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/pagespeed/proto/pagespeed_input.pb.cc',
            '<(SHARED_INTERMEDIATE_DIR)/pagespeed/proto/pagespeed_input.pb.h',
          ],
          'dependencies': [
            '<(DEPTH)/third_party/protobuf2/protobuf.gyp:protoc',
          ],
          'action': [
            '<(PRODUCT_DIR)/<(EXECUTABLE_PREFIX)protoc<(EXECUTABLE_SUFFIX)',
            '<(pagespeed_root)/pagespeed/proto/pagespeed_input.proto',
            '--proto_path=<(pagespeed_root)',
            '--cpp_out=<(SHARED_INTERMEDIATE_DIR)',
          ],
        },
      ],
      'sources': [
        '<(SHARED_INTERMEDIATE_DIR)/pagespeed/proto/pagespeed_input.pb.cc',
      ],
      'include_dirs': [
        '<(SHARED_INTERMEDIATE_DIR)',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(SHARED_INTERMEDIATE_DIR)',
        ],
      },
      'export_dependent_settings': [
        '<(DEPTH)/third_party/protobuf2/protobuf.gyp:protobuf',
      ]
    },
    {
      'target_name': 'pagespeed_output_pb',
      'type': '<(library)',
      'hard_dependency': 1,
      'dependencies': [
          '<(DEPTH)/third_party/protobuf2/protobuf.gyp:protobuf_lite',
          '<(DEPTH)/third_party/protobuf2/protobuf.gyp:protoc',
       ],
      'actions': [
        {
          'action_name': 'my_proto',
          'inputs': [
            '<(PRODUCT_DIR)/<(EXECUTABLE_PREFIX)protoc<(EXECUTABLE_SUFFIX)',
            '<(pagespeed_root)/pagespeed/proto/pagespeed_output.proto',
          ],
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/pagespeed/proto/pagespeed_output.pb.cc',
            '<(SHARED_INTERMEDIATE_DIR)/pagespeed/proto/pagespeed_output.pb.h',
          ],
          'dependencies': [
            '<(DEPTH)/third_party/protobuf2/protobuf.gyp:protoc',
          ],
          'action': [
            '<(PRODUCT_DIR)/<(EXECUTABLE_PREFIX)protoc<(EXECUTABLE_SUFFIX)',
            '<(pagespeed_root)/pagespeed/proto/pagespeed_output.proto',
            '--proto_path=<(pagespeed_root)',
            '--cpp_out=<(SHARED_INTERMEDIATE_DIR)',
          ],
        },
      ],
      'sources': [
        '<(SHARED_INTERMEDIATE_DIR)/pagespeed/proto/pagespeed_output.pb.cc',
      ],
      'include_dirs': [
        '<(SHARED_INTERMEDIATE_DIR)',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(SHARED_INTERMEDIATE_DIR)',
        ],
      },
      'export_dependent_settings': [
        '<(DEPTH)/third_party/protobuf2/protobuf.gyp:protobuf_lite',
      ]
    },
    {
      'target_name': 'pagespeed_cssmin',
      'type': '<(library)',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
      ],
      'sources': [
        '<(pagespeed_root)/pagespeed/cssmin/cssmin.cc',
      ],
      'include_dirs': [
        '<(pagespeed_root)',
      ],
    },
    {
      'target_name': 'pagespeed_html',
      'type': '<(library)',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        'pagespeed_cssmin',
        '<(pagespeed_root)/third_party/jsmin/jsmin.gyp:jsmin',
      ],
      'sources': [
        '<(pagespeed_root)/pagespeed/html/html_compactor.cc',
        '<(pagespeed_root)/pagespeed/html/html_tag.cc',
      ],
      'include_dirs': [
        '<(pagespeed_root)',
      ],
    },
    {
      'target_name': 'pagespeed_core',
      'type': '<(library)',
      'dependencies': [
        'pagespeed_output_pb',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/build/temp_gyp/googleurl.gyp:googleurl',
      ],
      'sources': [
        '<(pagespeed_root)/pagespeed/core/dom.cc',
        '<(pagespeed_root)/pagespeed/core/engine.cc',
        '<(pagespeed_root)/pagespeed/core/formatter.cc',
        '<(pagespeed_root)/pagespeed/core/pagespeed_input.cc',
        '<(pagespeed_root)/pagespeed/core/resource.cc',
        '<(pagespeed_root)/pagespeed/core/resource_filter.cc',
        '<(pagespeed_root)/pagespeed/core/resource_util.cc',
        '<(pagespeed_root)/pagespeed/core/rule.cc',
      ],
      'include_dirs': [
        '<(pagespeed_root)',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(pagespeed_root)',
        ],
      },
      'export_dependent_settings': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/build/temp_gyp/googleurl.gyp:googleurl',
      ]
    },
    {
      'target_name': 'pagespeed_util',
      'type': '<(library)',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
      ],
      'sources': [
        '<(pagespeed_root)/pagespeed/util/regex.cc',
      ],
      'include_dirs': [
        '<(pagespeed_root)',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(pagespeed_root)',
        ],
      },
    },
    {
      'target_name': 'pagespeed_filters',
      'type': '<(library)',
      'dependencies': [
        'pagespeed_core',
        'pagespeed_util',
        '<(pagespeed_root)/third_party/adblockrules/adblockrules.gyp:adblockrules',
      ],
      'sources': [
        '<(pagespeed_root)/pagespeed/filters/ad_filter.cc',
        '<(pagespeed_root)/pagespeed/filters/protocol_filter.cc',
      ],
      'include_dirs': [
        '<(pagespeed_root)',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(pagespeed_root)',
        ],
      },
      'export_dependent_settings': [
        '<(pagespeed_root)/third_party/adblockrules/adblockrules.gyp:adblockrules',
      ]
    },
    {
      'target_name': 'pagespeed_jpeg_optimizer',
      'type': '<(library)',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(pagespeed_root)/third_party/libjpeg/libjpeg_trans.gyp:libjpeg_trans',
      ],
      'sources': [
        '<(pagespeed_root)/pagespeed/image_compression/jpeg_optimizer.cc',
      ],
      'include_dirs': [
        '<(pagespeed_root)',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(pagespeed_root)',
        ],
      },
    },
    {
      'target_name': 'pagespeed',
      'type': '<(library)',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/third_party/zlib/zlib.gyp:zlib',
        '<(pagespeed_root)/third_party/jsmin/jsmin.gyp:jsmin',
        'pagespeed_core',
        'pagespeed_cssmin',
        'pagespeed_html',
        'pagespeed_jpeg_optimizer',
        'pagespeed_output_pb',
        'pagespeed_png_optimizer',
      ],
      'sources': [
        '<(pagespeed_root)/pagespeed/rules/avoid_bad_requests.cc',
        '<(pagespeed_root)/pagespeed/rules/combine_external_resources.cc',
        '<(pagespeed_root)/pagespeed/rules/enable_gzip_compression.cc',
        '<(pagespeed_root)/pagespeed/rules/minify_css.cc',
        '<(pagespeed_root)/pagespeed/rules/minify_html.cc',
        '<(pagespeed_root)/pagespeed/rules/minify_javascript.cc',
        '<(pagespeed_root)/pagespeed/rules/minify_rule.cc',
        '<(pagespeed_root)/pagespeed/rules/minimize_dns_lookups.cc',
        '<(pagespeed_root)/pagespeed/rules/minimize_redirects.cc',
        '<(pagespeed_root)/pagespeed/rules/minimize_request_size.cc',
        '<(pagespeed_root)/pagespeed/rules/optimize_images.cc',
        '<(pagespeed_root)/pagespeed/rules/optimize_the_order_of_styles_and_scripts.cc',
        '<(pagespeed_root)/pagespeed/rules/put_css_in_the_document_head.cc',
        '<(pagespeed_root)/pagespeed/rules/rule_provider.cc',
        '<(pagespeed_root)/pagespeed/rules/rule_util.cc',
        '<(pagespeed_root)/pagespeed/rules/savings_computer.cc',
        '<(pagespeed_root)/pagespeed/rules/serve_resources_from_a_consistent_url.cc',
        '<(pagespeed_root)/pagespeed/rules/serve_scaled_images.cc',
        '<(pagespeed_root)/pagespeed/rules/specify_a_cache_expiration.cc',
        '<(pagespeed_root)/pagespeed/rules/specify_a_date_header.cc',
        '<(pagespeed_root)/pagespeed/rules/specify_charset_early.cc',
        '<(pagespeed_root)/pagespeed/rules/specify_image_dimensions.cc',
      ],
      'include_dirs': [
        '<(pagespeed_root)',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(pagespeed_root)',
        ],
      },
      'export_dependent_settings': [
        '<(DEPTH)/base/base.gyp:base',
        'pagespeed_core',
        'pagespeed_png_optimizer'
      ],
    },
    {
      'target_name': 'pagespeed_png_optimizer',
      'type': '<(library)',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/third_party/libpng/libpng.gyp:libpng',
        '<(pagespeed_root)/third_party/optipng/optipng.gyp:opngreduc',
        '<(pagespeed_root)/third_party/optipng/optipng.gyp:pngxrgif',
      ],
      'sources': [
        '<(pagespeed_root)/pagespeed/image_compression/gif_reader.cc',
        '<(pagespeed_root)/pagespeed/image_compression/png_optimizer.cc',
      ],
      'include_dirs': [
        '<(DEPTH)',
        '<(pagespeed_root)',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(DEPTH)',
          '<(pagespeed_root)',
        ],
        'defines': [
          'PAGESPEED_PNG_OPTIMIZER_GIF_READER'
        ],
      },
      'export_dependent_settings': [
        '<(DEPTH)/base/base.gyp:base',
      ],
      'defines': [
        'PAGESPEED_PNG_OPTIMIZER_GIF_READER'
      ]
    },
    {
      'target_name': 'pagespeed_formatters',
      'type': '<(library)',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        'pagespeed_core',
        'pagespeed_output_pb',
      ],
      'sources': [
        '<(pagespeed_root)/pagespeed/formatters/formatter_util.cc',
        '<(pagespeed_root)/pagespeed/formatters/json_formatter.cc',
        '<(pagespeed_root)/pagespeed/formatters/proto_formatter.cc',
        '<(pagespeed_root)/pagespeed/formatters/text_formatter.cc',
      ],
      'include_dirs': [
        '<(pagespeed_root)',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(pagespeed_root)',
        ],
      },
      'export_dependent_settings': [
        '<(DEPTH)/base/base.gyp:base',
        'pagespeed_core'
      ]
    },
    {
      'target_name': 'pagespeed_proto',
      'type': '<(library)',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        'pagespeed_core',
        'pagespeed_input_pb',
      ],
      'sources': [
        '<(pagespeed_root)/pagespeed/proto/proto_resource_utils.cc',
      ],
      'include_dirs': [
        '<(pagespeed_root)',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '<(pagespeed_root)',
        ],
      },
      'export_dependent_settings': [
        '<(DEPTH)/base/base.gyp:base',
        'pagespeed_core'
      ]
    },
    {
      'target_name': 'pagespeed_bin',
      'type': 'executable',
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        'pagespeed',
        'pagespeed_formatters',
        'pagespeed_input_pb',
        'pagespeed_proto',
      ],
      'sources': [
        '<(pagespeed_root)/pagespeed/apps/pagespeed.cc',
      ],
    },
    {
      'target_name': 'optimize_image_bin',
      'type': 'executable',
      'dependencies': [
        'pagespeed_jpeg_optimizer',
        'pagespeed_png_optimizer',
      ],
      'sources': [
        '<(pagespeed_root)/pagespeed/apps/optimize_image.cc',
      ],
    },
    {
      'target_name': 'pagespeed_test',
      'type': 'executable',
      'dependencies': [
        'pagespeed',
        'pagespeed_filters',
        'pagespeed_cssmin',
        'pagespeed_formatters',
        'pagespeed_input_pb',
        'pagespeed_output_pb',
        'pagespeed_proto',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/testing/gtest.gyp:gtestmain',
      ],
      'include_dirs': [
        '<(pagespeed_root)',
      ],
      'sources': [
        '<(pagespeed_root)/pagespeed/core/engine_test.cc',
        '<(pagespeed_root)/pagespeed/core/pagespeed_input_test.cc',
        '<(pagespeed_root)/pagespeed/core/resource_test.cc',
        '<(pagespeed_root)/pagespeed/core/resource_filter_test.cc',
        '<(pagespeed_root)/pagespeed/core/resource_util_test.cc',
        '<(pagespeed_root)/pagespeed/cssmin/cssmin_test.cc',
        '<(pagespeed_root)/pagespeed/filters/ad_filter_test.cc',
        '<(pagespeed_root)/pagespeed/filters/protocol_filter_test.cc',
        '<(pagespeed_root)/pagespeed/formatters/formatter_util_test.cc',
        '<(pagespeed_root)/pagespeed/formatters/json_formatter_test.cc',
        '<(pagespeed_root)/pagespeed/formatters/proto_formatter_test.cc',
        '<(pagespeed_root)/pagespeed/formatters/text_formatter_test.cc',
        '<(pagespeed_root)/pagespeed/html/html_compactor_test.cc',
        '<(pagespeed_root)/pagespeed/html/html_tag_test.cc',
        '<(pagespeed_root)/pagespeed/rules/avoid_bad_requests_test.cc',
        '<(pagespeed_root)/pagespeed/rules/combine_external_resources_test.cc',
        '<(pagespeed_root)/pagespeed/rules/enable_gzip_compression_test.cc',
        '<(pagespeed_root)/pagespeed/rules/minify_css_test.cc',
        '<(pagespeed_root)/pagespeed/rules/minify_html_test.cc',
        '<(pagespeed_root)/pagespeed/rules/minify_javascript_test.cc',
        '<(pagespeed_root)/pagespeed/rules/minimize_dns_lookups_test.cc',
        '<(pagespeed_root)/pagespeed/rules/minimize_redirects_test.cc',
        '<(pagespeed_root)/pagespeed/rules/minimize_request_size_test.cc',
        '<(pagespeed_root)/pagespeed/rules/optimize_the_order_of_styles_and_scripts_test.cc',
        '<(pagespeed_root)/pagespeed/rules/put_css_in_the_document_head_test.cc',
        '<(pagespeed_root)/pagespeed/rules/serve_resources_from_a_consistent_url_test.cc',
        '<(pagespeed_root)/pagespeed/rules/serve_scaled_images_test.cc',
        '<(pagespeed_root)/pagespeed/rules/specify_a_cache_expiration_test.cc',
        '<(pagespeed_root)/pagespeed/rules/specify_a_date_header_test.cc',
        '<(pagespeed_root)/pagespeed/rules/specify_charset_early_test.cc',
        '<(pagespeed_root)/pagespeed/rules/specify_image_dimensions_test.cc',
        '<(pagespeed_root)/pagespeed/util/regex_test.cc',
      ],
    },
    {
      'target_name': 'pagespeed_image_test',
      'type': 'executable',
      'dependencies': [
        'pagespeed',
        'pagespeed_input_pb',
        'pagespeed_output_pb',
        'pagespeed_proto',
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/testing/gtest.gyp:gtestmain',
        '<(pagespeed_root)/third_party/readpng/readpng.gyp:readpng',
      ],
      'include_dirs': [
        '<(pagespeed_root)',
      ],
      'sources': [
        '<(pagespeed_root)/pagespeed/image_compression/jpeg_optimizer_test.cc',
        '<(pagespeed_root)/pagespeed/image_compression/png_optimizer_test.cc',
        '<(pagespeed_root)/pagespeed/rules/optimize_images_test.cc',
      ],
      'defines': [
        'IMAGE_TEST_DIR_PATH="<(pagespeed_root)/pagespeed/image_compression/testdata/"',
      ],
    },
  ],
}
