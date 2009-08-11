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

vars = {
  "chromium_trunk":
    "http://src.chromium.org/svn/trunk",
  "chromium_revision": "@22936",
}


deps = {
  "src/googleurl":
    "http://google-url.googlecode.com/svn/trunk@110",

  "src/testing/gtest":
    "http://googletest.googlecode.com/svn/trunk@267",

  "src/third_party/protobuf2/src":
    "http://protobuf.googlecode.com/svn/trunk@154",

  "src/tools/gyp":
    "http://gyp.googlecode.com/svn/trunk@580",

  # chromium deps
  "src/build/temp_gyp":
    Var("chromium_trunk") + "/src/build/temp_gyp" + Var("chromium_revision"),

  "src/build/chrome_build":
    Var("chromium_trunk") + "/src/build" + Var("chromium_revision"),

  "src/base/third_party/dmg_fp":
    (Var("chromium_trunk") + "/src/base/third_party/dmg_fp" +
     Var("chromium_revision")),

  "src/base/third_party/valgrind":
    (Var("chromium_trunk") + "/src/base/third_party/valgrind" +
     Var("chromium_revision")),

  "src/site_scons/site_tools":
    (Var("chromium_trunk") + "/src/site_scons/site_tools" +
     Var("chromium_revision")),

  "src/testing":
    Var("chromium_trunk") + "/src/testing" + Var("chromium_revision"),

  "src/third_party/icu38":
    (Var("chromium_trunk") + "/deps/third_party/icu38" +
     Var("chromium_revision")),

  "src/third_party/scons":
    Var("chromium_trunk") + "/src/third_party/scons" + Var("chromium_revision"),

  "src/tools/data_pack":
    Var("chromium_trunk") + "/src/tools/data_pack" + Var("chromium_revision"),

  "src/tools/grit":
    Var("chromium_trunk") + "/src/tools/grit" + Var("chromium_revision"),
}


deps_os = {
  "win": {
    "src/third_party/cygwin":
      Var("chromium_trunk") + "/deps/third_party/cygwin@11984",

    "src/third_party/python_24":
      Var("chromium_trunk") + "/deps/third_party/python_24@19441",
  },
  "mac": {
  },
  "unix": {
  },
}


include_rules = [
  # Everybody can use some things.
  "+base",
  "+build",
]


# checkdeps.py shouldn't check include paths for files in these dirs:
skip_child_includes = [
   "testing",
]


hooks = [
  {
    # A change to a .gyp, .gypi, or to GYP itself shound run the generator.
    "pattern": "\\.gypi?$|[/\\\\]src[/\\\\]tools[/\\\\]gyp[/\\\\]",
    "action": ["python", "src/tools/gyp/gyp_chromium"],
  },
]
