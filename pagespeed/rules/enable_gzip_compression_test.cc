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
#include "pagespeed/proto/pagespeed_output.pb.h"
#include "pagespeed/rules/enable_gzip_compression.h"
#include "testing/gtest/include/gtest/gtest.h"

using pagespeed::rules::EnableGzipCompression;
using pagespeed::PagespeedInput;
using pagespeed::Resource;
using pagespeed::Result;
using pagespeed::Results;

namespace {

class EnableGzipCompressionTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    input_.reset(new PagespeedInput);
  }

  virtual void TearDown() {
    input_.reset();
  }

  void AddTestResource(const char* url,
                       const char* content_type,
                       const char* content_encoding,
                       const char* content_length,
                       const char* body) {
    Resource* resource = new Resource;
    resource->SetRequestUrl(url);
    resource->SetRequestMethod("GET");
    resource->SetRequestProtocol("HTTP");
    resource->SetResponseStatusCode(200);
    resource->SetResponseProtocol("HTTP/1.1");

    if (content_type != NULL) {
      resource->AddResponseHeader("Content-Type", content_type);
    }

    if (content_encoding != NULL) {
      resource->AddResponseHeader("Content-Encoding", content_encoding);
    }

    if (content_length != NULL) {
      resource->AddResponseHeader("Content-Length", content_length);
    }

    if (body != NULL) {
      resource->SetResponseBody(body);
    }
    input_->AddResource(resource);
  }

  void AddFirstLargeHtmlResource(const char* charset,
                                 bool gzip) {
    std::string content_type = "text/html";
    if (charset != NULL) {
      content_type += "; charset=";
      content_type += charset;
    }
    AddTestResource("http://www.test.com/",
                    content_type.c_str(),
                    gzip ? "gzip" : NULL,
                    "9000",
                    NULL);
  }

  void AddFirstLargeHtmlResource(bool gzip) {
    return AddFirstLargeHtmlResource(NULL, gzip);
  }

  void AddSecondLargeHtmlResource(bool gzip) {
    AddTestResource("http://www.test.com/foo",
                    "text/html",
                    gzip ? "gzip" : NULL,
                    "4500",
                    NULL);
  }

  void AddHtmlResourceWithBody(const char* body,
                               bool gzip) {
    AddTestResource("http://www.test.com/",
                    "text/html",
                    gzip ? "gzip" : NULL,
                    NULL,
                    body);
  }

  void AddShortHtmlResource() {
    AddTestResource("http://www.test.com/",
                    "text/html",
                    NULL,
                    "10",
                    NULL);
  }

  void CheckNoViolations() {
    EnableGzipCompression gzip_rule;

    Results results;
    gzip_rule.AppendResults(*input_, &results);
    ASSERT_EQ(results.results_size(), 0);
  }

  void CheckOneViolation() {
    EnableGzipCompression gzip_rule;

    Results results;
    gzip_rule.AppendResults(*input_, &results);
    ASSERT_EQ(results.results_size(), 1);

    const Result& result = results.results(0);
    ASSERT_EQ(result.savings().response_bytes_saved(), 6000);
    ASSERT_EQ(result.resource_urls_size(), 1);
    ASSERT_EQ(result.resource_urls(0), "http://www.test.com/");
  }

  void CheckTwoViolations() {
    EnableGzipCompression gzip_rule;

    Results results;
    gzip_rule.AppendResults(*input_, &results);
    ASSERT_EQ(results.results_size(), 2);

    const Result& result0 = results.results(0);
    ASSERT_EQ(result0.savings().response_bytes_saved(), 6000);
    ASSERT_EQ(result0.resource_urls_size(), 1);
    ASSERT_EQ(result0.resource_urls(0), "http://www.test.com/");

    const Result& result1 = results.results(1);
    ASSERT_EQ(result1.savings().response_bytes_saved(), 3000);
    ASSERT_EQ(result1.resource_urls_size(), 1);
    ASSERT_EQ(result1.resource_urls(0), "http://www.test.com/foo");
  }

 private:
  scoped_ptr<PagespeedInput> input_;
};

TEST_F(EnableGzipCompressionTest, ViolationLargeHtmlNoGzip) {
  AddFirstLargeHtmlResource(false);

  CheckOneViolation();
}

TEST_F(EnableGzipCompressionTest, ViolationLargeHtmlUtf8NoGzip) {
  AddFirstLargeHtmlResource("utf-8", false);

  CheckOneViolation();
}

TEST_F(EnableGzipCompressionTest, NoViolationLargeHtmlGzip) {
  AddFirstLargeHtmlResource(true);

  CheckNoViolations();
}

TEST_F(EnableGzipCompressionTest, NoViolationSmallHtmlNoGzip) {
  AddShortHtmlResource();

  CheckNoViolations();
}

TEST_F(EnableGzipCompressionTest, NoViolationLargeNoContentTypeNoGzip) {
  AddTestResource("http://www.test.com/", NULL, NULL, "9000", NULL);

  CheckNoViolations();
}

TEST_F(EnableGzipCompressionTest, NoViolationLargeImageNoGzip) {
  AddTestResource("http://www.test.com/", "image/jpeg", NULL, "9000", NULL);

  CheckNoViolations();
}

TEST_F(EnableGzipCompressionTest, ViolationLargeHtmlNoGzipNoContentLength) {
  std::string body;
  body.resize(9000, 'a');
  AddHtmlResourceWithBody(body.c_str(), false);

  CheckOneViolation();
}

TEST_F(EnableGzipCompressionTest, NoViolationLargeHtmlGzipNoContentLength) {
  std::string body;
  body.resize(9000, 'a');
  AddHtmlResourceWithBody(body.c_str(), true);

  CheckNoViolations();
}

TEST_F(EnableGzipCompressionTest, NoViolationTwoHtmlGzip) {
  AddFirstLargeHtmlResource(true);
  AddSecondLargeHtmlResource(true);

  CheckNoViolations();
}

TEST_F(EnableGzipCompressionTest, OneViolationTwoHtmlNoGzip) {
  AddFirstLargeHtmlResource(false);
  AddSecondLargeHtmlResource(true);

  CheckOneViolation();
}

TEST_F(EnableGzipCompressionTest, TwoViolationsTwoHtmlNoGzip) {
  AddFirstLargeHtmlResource(false);
  AddSecondLargeHtmlResource(false);

  CheckTwoViolations();
}

}  // namespace