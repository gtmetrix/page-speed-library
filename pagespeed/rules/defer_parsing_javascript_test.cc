// Copyright 2011 Google Inc. All Rights Reserved.
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

#include "base/logging.h"
#include "base/scoped_ptr.h"
#include "base/string_number_conversions.h"
#include "pagespeed/core/pagespeed_input.h"
#include "pagespeed/core/resource.h"
#include "pagespeed/core/result_provider.h"
#include "pagespeed/proto/pagespeed_output.pb.h"
#include "pagespeed/rules/defer_parsing_javascript.h"
#include "pagespeed/testing/pagespeed_test.h"

using pagespeed::PagespeedInput;
using pagespeed::Resource;
using pagespeed::Result;
using pagespeed::Results;
using pagespeed::ResultProvider;
using pagespeed::ResultVector;
using pagespeed::RuleResults;
using pagespeed::rules::DeferParsingJavaScript;
using pagespeed_testing::FakeDomElement;

namespace {

// Unminified JavaScript.
const char* kUnminified = "function () { foo(); }";

// This value should match the on in the .cc file.
const size_t kMaxBlockOfJavascript = 1024*40;
const char* kRootUrl = "http://test.com/";

class DeferParsingJavaScriptTest : public
    ::pagespeed_testing::PagespeedRuleTest<DeferParsingJavaScript> {
 protected:
  virtual void DoSetUp() {
    NewPrimaryResource(kRootUrl);
    CreateHtmlHeadBodyElements();
  }

  void CreateScriptBlock(size_t size, std::string* script, bool commented) {
    *script = "";
    for (int idx = 0; script->size() < size; ++idx) {
      if (commented) {
        script->append("//function func_");
      } else {
        script->append("function func_");
      }
      script->append(base::IntToString(idx));
      script->append("(){var abc=1;bar();}\n");
    }
  }

  void CreateInlineScriptTag(
      size_t size, std::string* script_tag, bool commented) {
    std::string script;
    CreateScriptBlock(size, &script, commented);
    *script_tag ="<script type=\"text/javascript\">";
    script_tag->append(script);
    script_tag->append("</script>\n");
  }

  void AddTestResource(const char* url,
                       const char* script_body) {
    FakeDomElement* element;
    Resource* p_resource = primary_resource();
    std::string primary_body = p_resource->GetResponseBody();
    size_t pos = primary_body.rfind("</body>");
    std::string script_tag ="<script type=\"text/javascript\" src=\"";
    script_tag += url;
    script_tag += "\"></script>\n";
    if (pos != std::string::npos) {
      primary_body.insert(pos, script_tag);
    } else {
      primary_body.append(script_tag);
    }
    p_resource->SetResponseBody(primary_body);
    Resource* resource = NewScriptResource(url, body(), &element);
    resource->SetResponseBody(script_body);
  }

  void CheckNoViolations() {
    DeferParsingJavaScript rule;
    RuleResults rule_results;
    ResultProvider provider(rule, &rule_results, 0);
    pagespeed::RuleInput rule_input(*pagespeed_input());
    ASSERT_TRUE(rule.AppendResults(rule_input, &provider));
    ASSERT_EQ(rule_results.results_size(), 0);
  }

  void CheckScore(int score) {
    DeferParsingJavaScript rule;
    RuleResults rule_results;
    ResultProvider provider(rule, &rule_results, 0);
    pagespeed::RuleInput rule_input(*pagespeed_input());
    ASSERT_TRUE(rule.AppendResults(rule_input, &provider));
    ASSERT_EQ(rule_results.results_size(), 1);
    ASSERT_EQ(score, rule.ComputeScore(
        *pagespeed_input()->input_information(),
        rule_results));
  }
};

TEST_F(DeferParsingJavaScriptTest, Basic) {
  AddTestResource("http://www.example.com/foo.js",
                  kUnminified);
  Freeze();

  CheckNoViolations();
}

TEST_F(DeferParsingJavaScriptTest, LargeUnminifiedJavascriptFile) {
  std::string script = kUnminified;
  script.append(kMaxBlockOfJavascript, ' ');

  AddTestResource("http://www.example.com/foo.js",
                  script.c_str());
  Freeze();

  CheckNoViolations();
}

TEST_F(DeferParsingJavaScriptTest, LargeMinifiedJavascriptFile) {
  std::string script = kUnminified;
  for (int idx = 0; script.size() < kMaxBlockOfJavascript; ++idx) {
    script.append("function func_");
    script.append(base::IntToString(idx));
    script.append("(){var abc=1;bar();}\n");
  }
  script.append(kMaxBlockOfJavascript, ' ');
  std::string url("http://www.example.com/foo.js");
  AddTestResource(url.c_str(), script.c_str());

  CheckOneUrlViolation(url);
}

TEST_F(DeferParsingJavaScriptTest, LargeCommentedJavascriptFile) {
  std::string script = kUnminified;
  for (int idx = 0; script.size() < kMaxBlockOfJavascript; ++idx) {
    script.append("// function func_");
    script.append(base::IntToString(idx));
    script.append("(){var abc=1;bar();}\n");
  }
  script.append(kMaxBlockOfJavascript, ' ');
  std::string url("http://www.example.com/foo.js");
  AddTestResource(url.c_str(), script.c_str());

  CheckNoViolations();
}

TEST_F(DeferParsingJavaScriptTest, LargeInlineJavascript) {
  Resource* p_resource = primary_resource();
  std::string primary_body = p_resource->GetResponseBody();
  std::string script_tag ="<script type=\"text/javascript\">";
  std::string script;
  for (int idx = 0; script.size() < kMaxBlockOfJavascript; ++idx) {
    script.append("function func_");
    script.append(base::IntToString(idx));
    script.append("(){var abc=1;bar();}\n");
  }
  script_tag += script;
  script_tag += "</script>\n";
  size_t pos = primary_body.find_last_of("</body>");
  if (pos != std::string::npos) {
    primary_body.insert(pos, script_tag);
  } else {
    primary_body.append(script_tag);
  }
  p_resource->SetResponseBody(primary_body);

  CheckOneUrlViolation(kRootUrl);
}

TEST_F(DeferParsingJavaScriptTest, LargeCombinedJavascript) {
  // Add a script file.
  std::string script;
  CreateScriptBlock(kMaxBlockOfJavascript/2, &script, false);
  std::string url("http://www.example.com/foo.js");
  AddTestResource(url.c_str(), script.c_str());

  // Add two script block.
  Resource* p_resource = primary_resource();
  std::string primary_body = p_resource->GetResponseBody();
  std::string script_tag1;
  CreateInlineScriptTag(kMaxBlockOfJavascript/3, &script_tag1, false);
  std::string script_tag2;
  CreateInlineScriptTag(kMaxBlockOfJavascript/3, &script_tag2, false);

  std::string script_tags = script_tag1 + script_tag2;
  size_t pos = primary_body.rfind("</body>");
  if (pos != std::string::npos) {
    primary_body.insert(pos, script_tags);
  } else {
    primary_body.append(script_tags);
  }
  p_resource->SetResponseBody(primary_body);

  // The inline scirpt is big than the script file.
  CheckTwoUrlViolations(kRootUrl, url);
}

TEST_F(DeferParsingJavaScriptTest, LargeCombinedCommentedJavascript) {
  // Add a script file.
  std::string script;
  CreateScriptBlock(kMaxBlockOfJavascript/2, &script, false);
  std::string url("http://www.example.com/foo.js");
  AddTestResource(url.c_str(), script.c_str());

  // Add two script block.
  Resource* p_resource = primary_resource();
  std::string primary_body = p_resource->GetResponseBody();
  std::string script_tag1;
  CreateInlineScriptTag(kMaxBlockOfJavascript/3, &script_tag1, false);
  std::string script_tag2;
  CreateInlineScriptTag(kMaxBlockOfJavascript/3, &script_tag2, true);

  std::string script_tags = script_tag1 + script_tag2;
  size_t pos = primary_body.rfind("</body>");
  if (pos != std::string::npos) {
    primary_body.insert(pos, script_tags);
  } else {
    primary_body.append(script_tags);
  }
  p_resource->SetResponseBody(primary_body);

  CheckNoViolations();
}

TEST_F(DeferParsingJavaScriptTest, ComputeScore) {
  const size_t kScore80Size = 142*1024;
  Resource* p_resource = primary_resource();
  std::string primary_body = p_resource->GetResponseBody();
  std::string script_tag ="<script type=\"text/javascript\">";
  std::string script;
  for (int idx = 0; script.size() < kScore80Size; ++idx) {
    script.append("function func_");
    script.append(base::IntToString(idx));
    script.append("(){var abc=1;bar();}\n");
  }
  script_tag += script;
  script_tag += "</script>\n";
  size_t pos = primary_body.find_last_of("</body>");
  if (pos != std::string::npos) {
    primary_body.insert(pos, script_tag);
  } else {
    primary_body.append(script_tag);
  }
  p_resource->SetResponseBody(primary_body);

  Freeze();
  CheckScore(80);
}

}  // namespace
