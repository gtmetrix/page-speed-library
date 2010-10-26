// Copyright 2009 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>

#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/jsmin/cpp/jsmin.h"

namespace {

// This sample code comes from Douglas Crockford's jsmin example.
const char *kBeforeCompilation =
    "// is.js\n"
    "\n"
    "// (c) 2001 Douglas Crockford\n"
    "// 2001 June 3\n"
    "\n"
    "\n"
    "// is\n"
    "\n"
    "// The -is- object is used to identify the browser.  "
    "Every browser edition\n"
    "// identifies itself, but there is no standard way of doing it, "
    "and some of\n"
    "// the identification is deceptive. This is because the authors of web\n"
    "// browsers are liars. For example, Microsoft's IE browsers claim to be\n"
    "// Mozilla 4. Netscape 6 claims to be version 5.\n"
    "\n"
    "var is = {\n"
    "    ie:      navigator.appName == 'Microsoft Internet Explorer',\n"
    "    java:    navigator.javaEnabled(),\n"
    "    ns:      navigator.appName == 'Netscape',\n"
    "    ua:      navigator.userAgent.toLowerCase(),\n"
    "    version: parseFloat(navigator.appVersion.substr(21)) ||\n"
    "             parseFloat(navigator.appVersion),\n"
    "    win:     navigator.platform == 'Win32'\n"
    "}\n"
    "is.mac = is.ua.indexOf('mac') >= 0;\n"
    "if (is.ua.indexOf('opera') >= 0) {\n"
    "    is.ie = is.ns = false;\n"
    "    is.opera = true;\n"
    "}\n"
    "if (is.ua.indexOf('gecko') >= 0) {\n"
    "    is.ie = is.ns = false;\n"
    "    is.gecko = true;\n"
    "}\n";

const char *kAfterCompilation =
    "\n"
    "var is={ie:navigator.appName=='Microsoft Internet Explorer',"
    "java:navigator.javaEnabled(),ns:navigator.appName=='Netscape',"
    "ua:navigator.userAgent.toLowerCase(),version:parseFloat("
    "navigator.appVersion.substr(21))||parseFloat(navigator.appVersion)"
    ",win:navigator.platform=='Win32'}\n"
    "is.mac=is.ua.indexOf('mac')>=0;if(is.ua.indexOf('opera')>=0){"
    "is.ie=is.ns=false;is.opera=true;}\n"
    "if(is.ua.indexOf('gecko')>=0){is.ie=is.ns=false;is.gecko=true;}";

TEST(JsminTest, Basic) {
  std::string output;
  ASSERT_TRUE(jsmin::MinifyJs(kBeforeCompilation, &output));
  ASSERT_EQ(kAfterCompilation, output);

  int minimized_size = -1;
  ASSERT_TRUE(jsmin::GetMinifiedJsSize(kBeforeCompilation, &minimized_size));
  ASSERT_EQ(strlen(kAfterCompilation), minimized_size);
}

TEST(JsminTest, AlreadyMinified) {
  std::string output;
  ASSERT_TRUE(jsmin::MinifyJs(kAfterCompilation, &output));
  ASSERT_EQ(kAfterCompilation, output);

  int minimized_size = -1;
  ASSERT_TRUE(jsmin::GetMinifiedJsSize(kAfterCompilation, &minimized_size));
  ASSERT_EQ(strlen(kAfterCompilation), minimized_size);
}

TEST(JsminTest, Error) {
  std::string input = "/* not valid javascript";
  std::string output;
  ASSERT_FALSE(jsmin::MinifyJs(input, &output));
  ASSERT_TRUE(output.empty());

  int minimized_size = -1;
  ASSERT_FALSE(jsmin::GetMinifiedJsSize(input, &minimized_size));
  ASSERT_EQ(-1, minimized_size);
}

TEST(JsminTest, SignedCharDoesntSignExtend) {
  const char input[] = { '\n', 0xff, 0x00 };
  std::string output;
  ASSERT_TRUE(jsmin::MinifyJs(input, &output));
  ASSERT_EQ(input, output);

  int minimized_size = -1;
  ASSERT_TRUE(jsmin::GetMinifiedJsSize(input, &minimized_size));
  ASSERT_EQ(2, minimized_size);
}

TEST(JsminTest, DealWithCrlf) {
  std::string input = "var x = 1;\r\nvar y = 2;";
  std::string expected = "\nvar x=1;var y=2;";
  std::string output;
  ASSERT_TRUE(jsmin::MinifyJs(input, &output));
  ASSERT_EQ(expected, output);
}

TEST(JsminTest, DealWithTabs) {
  std::string input = "var x = 1;\n\tvar y = 2;";
  std::string expected = "\nvar x=1;var y=2;";
  std::string output;
  ASSERT_TRUE(jsmin::MinifyJs(input, &output));
  ASSERT_EQ(expected, output);
}

TEST(JsminTest, EscapedCrlfInStringLiteral) {
  std::string input = "var x = 'foo\\\r\nbar';";
  std::string expected = "\nvar x='foo\\\r\nbar';";
  std::string output;
  ASSERT_TRUE(jsmin::MinifyJs(input, &output));
  ASSERT_EQ(expected, output);
}

}  // namespace
