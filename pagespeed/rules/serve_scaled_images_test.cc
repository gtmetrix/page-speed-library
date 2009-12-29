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
#include <sstream>

#include "base/scoped_ptr.h"
#include "base/stl_util-inl.h"  // for STLDeleteContainerPointers
#include "pagespeed/core/dom.h"
#include "pagespeed/core/pagespeed_input.h"
#include "pagespeed/core/resource.h"
#include "pagespeed/formatters/text_formatter.h"
#include "pagespeed/proto/pagespeed_output.pb.h"
#include "pagespeed/rules/serve_scaled_images.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

class MockDocument : public pagespeed::DomDocument {
 public:
  MockDocument() {}
  virtual ~MockDocument() {
    STLDeleteContainerPointers(elements_.begin(), elements_.end());
  }

  virtual void Traverse(pagespeed::DomElementVisitor* visitor) const {
    for (std::vector<pagespeed::DomElement*>::const_iterator
             iter = elements_.begin(),
             end = elements_.end();
         iter != end;
         ++iter) {
      visitor->Visit(**iter);
    }
  }

  void AddElement(pagespeed::DomElement* element) {
    elements_.push_back(element);
  }

 private:
  std::vector<pagespeed::DomElement*> elements_;

  DISALLOW_COPY_AND_ASSIGN(MockDocument);
};

class MockImageElement : public pagespeed::DomElement {
 public:
  MockImageElement(const std::string& resource_url,
                   int client_width, int client_height,
                   int natural_width, int natural_height)
      : resource_url_(resource_url),
        client_width_(client_width), client_height_(client_height),
        natural_width_(natural_width), natural_height_(natural_height) {}

  virtual pagespeed::DomDocument* GetContentDocument() const {
    return NULL;
  }

  virtual std::string GetTagName() const {
    return "IMG";
  }

  virtual bool GetResourceUrl(std::string* src) const {
    src->assign(resource_url_);
    return true;
  }

  virtual bool GetIntPropertyByName(const std::string& name,
                                    int* property_value) const {
    if (name == "clientWidth") {
      *property_value = client_width_;
    } else if (name == "clientHeight") {
      *property_value = client_height_;
    } else if (name == "naturalWidth") {
      *property_value = natural_width_;
    } else if (name == "naturalHeight") {
      *property_value = natural_height_;
    } else {
      return false;
    }
    return true;
  }

 private:
  std::string resource_url_;
  int client_width_, client_height_, natural_width_, natural_height_;

  DISALLOW_COPY_AND_ASSIGN(MockImageElement);
};

class MockIframeElement : public pagespeed::DomElement {
 public:
  explicit MockIframeElement(pagespeed::DomDocument* content)
      : content_(content) {}

  virtual pagespeed::DomDocument* GetContentDocument() const {
    return content_;
  }

  virtual std::string GetTagName() const {
    return "IFRAME";
  }

 private:
  pagespeed::DomDocument* content_;

  DISALLOW_COPY_AND_ASSIGN(MockIframeElement);
};

class ServeScaledImagesTest : public ::testing::Test {
 protected:

  virtual void SetUp() {
    input_.reset(new pagespeed::PagespeedInput);
  }

  virtual void TearDown() {
    input_.reset();
  }

  void AddPngResource(const std::string &url, const int size) {
    std::string body(size, 'x');
    pagespeed::Resource* resource = new pagespeed::Resource;
    resource->SetRequestUrl(url);
    resource->SetRequestMethod("GET");
    resource->SetRequestProtocol("HTTP");
    resource->SetResponseStatusCode(200);
    resource->SetResponseProtocol("HTTP/1.1");
    resource->AddResponseHeader("Content-Type", "image/png");
    resource->SetResponseBody(body);
    input_->AddResource(resource);
  }

  void CheckNoViolations(MockDocument* document) {
    CheckExpectedViolations(document, std::vector<std::string>());
  }

  void CheckOneViolation(MockDocument* document,
                         const std::string& violation_url) {
    std::vector<std::string> expected;
    expected.push_back(violation_url);
    CheckExpectedViolations(document, expected);
  }

  void CheckTwoViolations(MockDocument* document,
                          const std::string& violation_url1,
                          const std::string& violation_url2) {
    std::vector<std::string> expected;
    expected.push_back(violation_url1);
    expected.push_back(violation_url2);
    CheckExpectedViolations(document, expected);
  }

  void CheckFormattedOutput(MockDocument* document,
                            const std::string& expected_output) {
    input_->AcquireDomDocument(document);

    pagespeed::Results results;
    {
      // compute results
      pagespeed::rules::ServeScaledImages scaling_rule;
      ASSERT_TRUE(scaling_rule.AppendResults(*input_, &results));
    }

    {
      // format results
      pagespeed::ResultVector result_vector;
      for (int ii = 0; ii < results.results_size(); ++ii) {
        result_vector.push_back(&results.results(ii));
      }

      std::stringstream output;
      pagespeed::formatters::TextFormatter formatter(&output);
      pagespeed::rules::ServeScaledImages scaling_rule;
      scaling_rule.FormatResults(result_vector, &formatter);
      EXPECT_EQ(expected_output, output.str());
    }
  }

 private:
  void CheckExpectedViolations(MockDocument* document,
                               const std::vector<std::string>& expected) {
    input_->AcquireDomDocument(document);

    pagespeed::rules::ServeScaledImages scaling_rule;

    pagespeed::Results results;
    ASSERT_TRUE(scaling_rule.AppendResults(*input_, &results));
    ASSERT_EQ(expected.size(), results.results_size());

    for (int idx = 0; idx < expected.size(); ++idx) {
      const pagespeed::Result& result = results.results(idx);
      ASSERT_EQ(result.resource_urls_size(), 1);
      EXPECT_EQ(expected[idx], result.resource_urls(0));
    }
  }

  scoped_ptr<pagespeed::PagespeedInput> input_;
};

TEST_F(ServeScaledImagesTest, EmptyDom) {
  MockDocument* doc = new MockDocument;
  CheckNoViolations(doc);
}

TEST_F(ServeScaledImagesTest, NotResized) {
  MockDocument* doc = new MockDocument;
  AddPngResource("http://test.com/image.png", 50);
  doc->AddElement(new MockImageElement("http://test.com/image.png",
                                       23, 42, 23, 42));
  CheckNoViolations(doc);
}

TEST_F(ServeScaledImagesTest, ShrunkHeight) {
  MockDocument* doc = new MockDocument;
  AddPngResource("http://test.com/image.png", 50);
  doc->AddElement(new MockImageElement("http://test.com/image.png",
                                       23, 21, 23, 42));
  CheckOneViolation(doc, "http://test.com/image.png");
}

TEST_F(ServeScaledImagesTest, ShrunkWidth) {
  MockDocument* doc = new MockDocument;
  AddPngResource("http://test.com/image.png", 50);
  doc->AddElement(new MockImageElement("http://test.com/image.png",
                                       22, 42, 23, 42));
  CheckOneViolation(doc, "http://test.com/image.png");
}

TEST_F(ServeScaledImagesTest, ShrunkBoth) {
  MockDocument* doc = new MockDocument;
  AddPngResource("http://test.com/image.png", 50);
  doc->AddElement(new MockImageElement("http://test.com/image.png",
                                       22, 21, 23, 42));
  CheckOneViolation(doc, "http://test.com/image.png");
}

TEST_F(ServeScaledImagesTest, IncreasedBoth) {
  MockDocument* doc = new MockDocument;
  AddPngResource("http://test.com/image.png", 50);
  doc->AddElement(new MockImageElement("http://test.com/image.png",
                                       46, 84, 23, 42));
  CheckNoViolations(doc);
}

TEST_F(ServeScaledImagesTest, ShrunkInIFrame) {
  MockDocument* iframe_doc = new MockDocument;
  AddPngResource("http://test.com/image.png", 50);
  iframe_doc->AddElement(new MockImageElement("http://test.com/image.png",
                                              22, 21, 23, 42));
  MockDocument* doc = new MockDocument;
  doc->AddElement(new MockIframeElement(iframe_doc));
  CheckOneViolation(doc, "http://test.com/image.png");
}

TEST_F(ServeScaledImagesTest, MultipleViolations) {
  MockDocument* doc = new MockDocument;
  AddPngResource("http://test.com/imageA.png", 50);
  AddPngResource("http://test.com/imageB.png", 40);
  doc->AddElement(new MockImageElement("http://test.com/imageA.png",
                                       22, 21, 23, 42));
  doc->AddElement(new MockImageElement("http://test.com/imageB.png",
                                       5, 15, 10, 30));
  CheckTwoViolations(doc,
                     "http://test.com/imageA.png",
                     "http://test.com/imageB.png");
}

TEST_F(ServeScaledImagesTest, ShrunkTwice) {
  MockDocument* doc = new MockDocument;
  AddPngResource("http://test.com/image.png", 50);
  doc->AddElement(new MockImageElement("http://test.com/image.png",
                                       22, 21, 23, 42));
  doc->AddElement(new MockImageElement("http://test.com/image.png",
                                       5, 15, 23, 42));
  CheckOneViolation(doc, "http://test.com/image.png");
}

TEST_F(ServeScaledImagesTest, NotAlwaysShrunk) {
  MockDocument* doc = new MockDocument;
  AddPngResource("http://test.com/image.png", 50);
  doc->AddElement(new MockImageElement("http://test.com/image.png",
                                       23, 42, 23, 42));
  doc->AddElement(new MockImageElement("http://test.com/image.png",
                                       5, 15, 23, 42));
  CheckNoViolations(doc);
}

TEST_F(ServeScaledImagesTest, ShrunkAndIncreased) {
  MockDocument* doc = new MockDocument;
  AddPngResource("http://test.com/image.png", 50);
  doc->AddElement(new MockImageElement("http://test.com/image.png",
                                       46, 84, 23, 42));
  doc->AddElement(new MockImageElement("http://test.com/image.png",
                                       5, 15, 23, 42));
  CheckNoViolations(doc);
}

TEST_F(ServeScaledImagesTest, FormatTest) {
  std::string expected =
      "The following images are resized in HTML or CSS.  "
      "Serving scaled images could save 47B (94% reduction).\n"
      "  http://test.com/a.png is resized in HTML or CSS from 23x42 to 5x15.  "
      "Serving a scaled image could save 47B (94% reduction).\n";

  MockDocument* doc = new MockDocument;
  AddPngResource("http://test.com/a.png", 50);
  doc->AddElement(new MockImageElement("http://test.com/a.png", 5, 15, 23, 42));
  CheckFormattedOutput(doc, expected);
}

TEST_F(ServeScaledImagesTest, FormatNoOutputTest) {
  MockDocument* doc = new MockDocument;
  CheckFormattedOutput(doc, "");
}

}  // namespace
