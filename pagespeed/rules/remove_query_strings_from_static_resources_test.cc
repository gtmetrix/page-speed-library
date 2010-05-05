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

#include "base/scoped_ptr.h"
#include "pagespeed/core/pagespeed_input.h"
#include "pagespeed/core/resource.h"
#include "pagespeed/core/result_provider.h"
#include "pagespeed/proto/pagespeed_output.pb.h"
#include "pagespeed/rules/remove_query_strings_from_static_resources.h"
#include "testing/gtest/include/gtest/gtest.h"

using pagespeed::rules::RemoveQueryStringsFromStaticResources;
using pagespeed::PagespeedInput;
using pagespeed::Resource;
using pagespeed::Result;
using pagespeed::Results;
using pagespeed::ResultProvider;

namespace {

class RemoveQueryStringsFromStaticResourcesTest : public ::testing::Test {
 protected:

  virtual void SetUp() {
    input_.reset(new PagespeedInput);
  }

  virtual void TearDown() {
    input_.reset();
  }

  void AddTestResource(const std::string& url,
                       const std::string& type) {
    Resource* resource = new Resource;
    resource->SetRequestUrl(url);
    resource->SetRequestMethod("GET");
    resource->SetRequestProtocol("HTTP/1.1");
    resource->SetResponseStatusCode(200);
    resource->SetResponseProtocol("HTTP/1.1");
    resource->SetResponseBody("Hello, world!");
    resource->AddResponseHeader("Content-Type", type);
    resource->AddResponseHeader("Cache-Control", "public, max-age=1000000");
    input_->AddResource(resource);
  }

  void CheckNoViolations() {
    RemoveQueryStringsFromStaticResources rule;
    Results results;
    ResultProvider provider(rule, &results);
    ASSERT_TRUE(rule.AppendResults(*input_, &provider));
    ASSERT_EQ(0, results.results_size());
  }

  void CheckOneViolation(const std::string& url) {
    RemoveQueryStringsFromStaticResources rule;
    Results results;
    ResultProvider provider(rule, &results);
    ASSERT_TRUE(rule.AppendResults(*input_, &provider));
    ASSERT_EQ(1, results.results_size());
    const Result& result = results.results(0);
    ASSERT_EQ(1, result.resource_urls_size());
    ASSERT_EQ(url, result.resource_urls(0));
  }

 private:
  scoped_ptr<PagespeedInput> input_;
};

TEST_F(RemoveQueryStringsFromStaticResourcesTest, NoProblems) {
  AddTestResource("http://www.example.com/index.html",
                  "text/html");
  AddTestResource("http://static.example.com/image/40/30",
                  "image/png");
  CheckNoViolations();
}

TEST_F(RemoveQueryStringsFromStaticResourcesTest, OneViolation) {
  AddTestResource("http://www.example.com/index.html",
                  "text/html");
  AddTestResource("http://static.example.com/image?w=40&h=30",
                  "image/png");
  CheckOneViolation("http://static.example.com/image?w=40&h=30");
}

}  // namespace
