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

#include <algorithm>
#include <string>
#include <vector>

#include "base/scoped_ptr.h"
#include "pagespeed/core/pagespeed_input.h"
#include "pagespeed/core/resource.h"
#include "pagespeed/core/result_provider.h"
#include "pagespeed/proto/pagespeed_output.pb.h"
#include "pagespeed/rules/serve_resources_from_a_consistent_url.h"
#include "testing/gtest/include/gtest/gtest.h"

using pagespeed::rules::ServeResourcesFromAConsistentUrl;
using pagespeed::PagespeedInput;
using pagespeed::Resource;
using pagespeed::Result;
using pagespeed::Results;
using pagespeed::ResultProvider;

namespace {

const char *kResponseBodies[] = {
  "first response body"
  "                                                  "
  "                                                  ",
  "second response body"
  "                                                  "
  "                                                  ",
  "third response body"
  "                                                  "
  "                                                  ",
};

const char *kResponseUrls[2][3] = {
  {
    "http://www.example.com/bac",
    "http://www.example.com/abracadabra",
    "http://www.example.com/c",
  },
  {
    "http://www.foo.com/z",
    "http://www.foo.com/yy",
    "http://www.foo.com/abc",
  }
};

class ServeResourcesFromAConsistentUrlTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    input_.reset(new PagespeedInput);
  }

  virtual void TearDown() {
    input_.reset();
  }

  void AddTestResource(const char* url, const std::string& body) {
    Resource* resource = new Resource;
    resource->SetRequestUrl(url);
    resource->SetRequestMethod("GET");
    resource->SetRequestProtocol("HTTP");
    resource->SetResponseStatusCode(200);
    resource->SetResponseProtocol("HTTP/1.1");
    resource->SetResponseBody(body);

    input_->AddResource(resource);
  }

  void CheckNoViolations() {
    ServeResourcesFromAConsistentUrl rule;

    Results results;
    ResultProvider provider(rule, &results);
    rule.AppendResults(*input_, &provider);
    ASSERT_EQ(0, results.results_size());
  }

  int ComputeSavings(size_t num_resources, const char *body) {
    return (num_resources - 1) * strlen(body);
  }

  void CheckViolation(size_t num_collisions, size_t num_resources) {
    ServeResourcesFromAConsistentUrl rule;

    Results results;
    ResultProvider provider(rule, &results);
    rule.AppendResults(*input_, &provider);
    ASSERT_EQ(num_collisions, results.results_size());
    for (int result_idx = 0;
         result_idx < results.results_size();
         result_idx++) {
      const Result& result = results.results(result_idx);

      int expected_savings =
          ComputeSavings(num_resources, kResponseBodies[result_idx]);
      ASSERT_EQ(num_resources - 1, result.savings().requests_saved());
      ASSERT_EQ(expected_savings, result.savings().response_bytes_saved());
      ASSERT_EQ(num_resources, result.resource_urls_size());

      // Now verify that the list or resource URLs in the Result
      // contains the expected contents. We sort both lists, then
      // assert that the sorted results are equal.
      std::vector<std::string> expected_urls;
      std::vector<std::string> actual_urls;
      for (int url_idx = 0; url_idx < num_resources; url_idx++) {
        expected_urls.push_back(kResponseUrls[result_idx][url_idx]);
        actual_urls.push_back(result.resource_urls(url_idx));
      }
      std::sort(expected_urls.begin(), expected_urls.end());
      std::sort(actual_urls.begin(), actual_urls.end());

      ASSERT_TRUE(expected_urls == actual_urls);
    }
  }

 private:
  scoped_ptr<PagespeedInput> input_;
};

TEST_F(ServeResourcesFromAConsistentUrlTest, NoResources) {
  CheckNoViolations();
}

TEST_F(ServeResourcesFromAConsistentUrlTest, SingleResource) {
  AddTestResource("http://www.example.com", kResponseBodies[0]);
  CheckNoViolations();
}

TEST_F(ServeResourcesFromAConsistentUrlTest, SingleEmptyResource) {
  AddTestResource("http://www.example.com", "");
  CheckNoViolations();
}

TEST_F(ServeResourcesFromAConsistentUrlTest, MultipleEmptyResources) {
  AddTestResource(kResponseUrls[0][0], "");
  AddTestResource(kResponseUrls[0][1], "");
  AddTestResource(kResponseUrls[0][2], "");
  CheckNoViolations();
}

TEST_F(ServeResourcesFromAConsistentUrlTest, DifferentResources) {
  AddTestResource(kResponseUrls[0][0], kResponseBodies[0]);
  AddTestResource(kResponseUrls[0][1], kResponseBodies[1]);
  AddTestResource(kResponseUrls[0][2], kResponseBodies[2]);
  CheckNoViolations();
}

TEST_F(ServeResourcesFromAConsistentUrlTest, SameResourceTwoUrls) {
  AddTestResource(kResponseUrls[0][0], kResponseBodies[0]);
  AddTestResource(kResponseUrls[0][1], kResponseBodies[0]);
  CheckViolation(1, 2);
}

TEST_F(ServeResourcesFromAConsistentUrlTest, SameResourceTwoUrls2) {
  AddTestResource(kResponseUrls[0][0], kResponseBodies[0]);
  AddTestResource(kResponseUrls[0][1], kResponseBodies[0]);
  AddTestResource(kResponseUrls[0][2], "");
  CheckViolation(1, 2);
}

TEST_F(ServeResourcesFromAConsistentUrlTest, SameResourceTwoUrls3) {
  AddTestResource(kResponseUrls[0][0], kResponseBodies[0]);
  AddTestResource(kResponseUrls[0][1], kResponseBodies[0]);
  AddTestResource(kResponseUrls[0][2], kResponseBodies[1]);
  CheckViolation(1, 2);
}

TEST_F(ServeResourcesFromAConsistentUrlTest, SameResourceThreeUrls) {
  AddTestResource(kResponseUrls[0][0], kResponseBodies[0]);
  AddTestResource(kResponseUrls[0][1], kResponseBodies[0]);
  AddTestResource(kResponseUrls[0][2], kResponseBodies[0]);
  CheckViolation(1, 3);
}

TEST_F(ServeResourcesFromAConsistentUrlTest, TwoDuplicatedResources) {
  AddTestResource(kResponseUrls[0][0], kResponseBodies[0]);
  AddTestResource(kResponseUrls[0][1], kResponseBodies[0]);
  AddTestResource(kResponseUrls[1][0], kResponseBodies[1]);
  AddTestResource(kResponseUrls[1][1], kResponseBodies[1]);
  CheckViolation(2, 2);
}

TEST_F(ServeResourcesFromAConsistentUrlTest, BinaryResponseBodies) {
  std::string body_a("abcdefghij");
  std::string body_b("abcde");
  body_a[5] = '\0';
  AddTestResource("http://www.example.com/a", body_a);
  AddTestResource("http://www.example.com/b", body_b);
  CheckNoViolations();
}

}  // namespace
