# Copyright (c) 2009 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Copied from the Chromium temp_gyp directory, with relative paths modified to
# suit its new location.
{
  'variables': {
    'chromium_code': 1,
  },
  'targets': [
    {
      'target_name': 'googleurl_noicu',
      'type': '<(library)',
      'msvs_guid': 'EF5E94AB-B646-4E5B-A058-52EF07B8351C',
      'dependencies': [
        '../base/base.gyp:base',
      ],
      'sources': [
        '../googleurl/src/gurl.cc',
        '../googleurl/src/gurl.h',
        '../googleurl/src/url_canon.h',
        '../googleurl/src/url_canon_etc.cc',
        '../googleurl/src/url_canon_fileurl.cc',
        '../googleurl/src/url_canon_host.cc',
        'url_canon_noicu.cc',
        '../googleurl/src/url_canon_icu.h',
        '../googleurl/src/url_canon_internal.cc',
        '../googleurl/src/url_canon_internal.h',
        '../googleurl/src/url_canon_internal_file.h',
        '../googleurl/src/url_canon_ip.cc',
        '../googleurl/src/url_canon_ip.h',
        '../googleurl/src/url_canon_mailtourl.cc',
        '../googleurl/src/url_canon_path.cc',
        '../googleurl/src/url_canon_pathurl.cc',
        '../googleurl/src/url_canon_query.cc',
        '../googleurl/src/url_canon_relative.cc',
        '../googleurl/src/url_canon_stdstring.h',
        '../googleurl/src/url_canon_stdurl.cc',
        '../googleurl/src/url_file.h',
        '../googleurl/src/url_parse.cc',
        '../googleurl/src/url_parse.h',
        '../googleurl/src/url_parse_file.cc',
        '../googleurl/src/url_parse_internal.h',
        '../googleurl/src/url_util.cc',
        '../googleurl/src/url_util.h',
      ],
      'include_dirs': [
        '..',
      ],
      'direct_dependent_settings': {
        'include_dirs': [
          '..',
        ],
      },
    },
  ],
}
