// Copyright 2010 Google Inc. All Rights Reserved.
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
#include "base/string_util.h"  // for StringPrintf
#include "pagespeed/core/pagespeed_input.h"
#include "pagespeed/core/resource.h"
#include "pagespeed/core/result_provider.h"
#include "pagespeed/proto/pagespeed_output.pb.h"
#include "pagespeed/rules/parallelize_downloads_across_hostnames.h"
#include "testing/gtest/include/gtest/gtest.h"

using pagespeed::rules::ParallelizeDownloadsAcrossHostnames;
using pagespeed::PagespeedInput;
using pagespeed::ParallelizableHostDetails;
using pagespeed::Resource;
using pagespeed::Result;
using pagespeed::Results;
using pagespeed::ResultProvider;

namespace {

class ParallelizeDownloadsAcrossHostnamesTest : public ::testing::Test {
 protected:

  virtual void SetUp() {
    input_.reset(new PagespeedInput);
  }

  virtual void TearDown() {
    input_.reset();
  }

  void AddStaticResources(int num, const std::string& host) {
    for (int index = 0; index < num; ++index) {
      Resource* resource = new Resource;
      resource->SetRequestUrl(StringPrintf("http://%s/resource%d.css",
                                           host.c_str(), index));
      resource->SetRequestMethod("GET");
      resource->SetRequestProtocol("HTTP/1.1");
      resource->SetResponseStatusCode(200);
      resource->SetResponseProtocol("HTTP/1.1");
      resource->AddResponseHeader("Content-Type", "text/css");
      resource->SetResponseBody("Hello, world!");
      input_->AddResource(resource);
    }
  }

  void CheckNoViolations() {
    ParallelizeDownloadsAcrossHostnames rule;
    Results results;
    ResultProvider provider(rule, &results);
    ASSERT_TRUE(rule.AppendResults(*input_, &provider));
    ASSERT_EQ(0, results.results_size());
  }

  void CheckOneViolation(const std::string &host,
                         int critical_path_saved) {
    ParallelizeDownloadsAcrossHostnames rule;
    Results results;
    ResultProvider provider(rule, &results);
    ASSERT_TRUE(rule.AppendResults(*input_, &provider));
    ASSERT_EQ(1, results.results_size());
    const Result& result = results.results(0);
    ASSERT_EQ(host, result.details().GetExtension(
        ParallelizableHostDetails::message_set_extension).host());
    ASSERT_EQ(critical_path_saved,
              result.savings().critical_path_length_saved());
  }

 private:
  scoped_ptr<PagespeedInput> input_;
};

TEST_F(ParallelizeDownloadsAcrossHostnamesTest, NotManyResources) {
  AddStaticResources(7, "static.example.com");
  CheckNoViolations();
}

TEST_F(ParallelizeDownloadsAcrossHostnamesTest, BalancedResources) {
  AddStaticResources(51, "static1.example.com");
  AddStaticResources(52, "static2.example.com");
  AddStaticResources(55, "static3.example.com");
  AddStaticResources(53, "static4.example.com");
  CheckNoViolations();
}

TEST_F(ParallelizeDownloadsAcrossHostnamesTest, JustOneHost) {
  AddStaticResources(80, "static.example.com");
  CheckOneViolation("static.example.com", 60);
}

TEST_F(ParallelizeDownloadsAcrossHostnamesTest, UnbalancedResources) {
  AddStaticResources(10, "static1.example.com");
  AddStaticResources(10, "static2.example.com");
  AddStaticResources(30, "static3.example.com");
  AddStaticResources(10, "static4.example.com");
  CheckOneViolation("static3.example.com", 15);
}

}  // namespace
