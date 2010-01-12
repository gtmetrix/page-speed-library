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

#include "pagespeed/rules/put_css_in_the_document_head.h"

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/logging.h"
#include "pagespeed/core/dom.h"
#include "pagespeed/core/formatter.h"
#include "pagespeed/core/pagespeed_input.h"
#include "pagespeed/core/resource.h"
#include "pagespeed/proto/pagespeed_output.pb.h"

namespace pagespeed {

namespace {

const char* kRuleName = "PutCssInTheDocumentHead";

class StyleVisitor : public pagespeed::DomElementVisitor {
 public:
  static void CheckDocument(const DomDocument* document, Results* results) {
    if (document) {
      StyleVisitor visitor(document->GetDocumentUrl(), results);
      document->Traverse(&visitor);
      visitor.Finish();
    }
  }

  virtual void Visit(const DomElement& node) {
    const std::string tag_name(node.GetTagName());
    if (tag_name == "IFRAME") {
      CheckDocument(node.GetContentDocument(), results_);
    } else if (tag_name == "BODY") {
      is_in_body_yet_ = true;
    } else if (is_in_body_yet_) {
      if (tag_name == "LINK") {
        std::string rel, href;
        if (node.GetAttributeByName("rel", &rel) && rel == "stylesheet" &&
            node.GetAttributeByName("href", &href)) {
          external_styles_.push_back(href);
        }
      } else if (tag_name == "STYLE") {
        ++num_inline_style_blocks_;
      }
    }
  }

 private:
  StyleVisitor(const std::string& document_url, Results* results)
      : is_in_body_yet_(false), num_inline_style_blocks_(0),
        document_url_(document_url), results_(results) {}

  void Finish() {
    DCHECK(num_inline_style_blocks_ >= 0);
    if (num_inline_style_blocks_ == 0 && external_styles_.empty()) {
      return;
    }

    Result* result = results_->add_results();
    result->set_rule_name(kRuleName);
    result->add_resource_urls(document_url_);

    Savings* savings = result->mutable_savings();
    savings->set_page_reflows_saved(num_inline_style_blocks_ +
                                    external_styles_.size());

    ResultDetails* details = result->mutable_details();
    StylesInBodyDetails* style_details =
        details->MutableExtension(StylesInBodyDetails::message_set_extension);
    style_details->set_num_inline_style_blocks(num_inline_style_blocks_);
    for (std::vector<std::string>::const_iterator i = external_styles_.begin(),
             end = external_styles_.end(); i != end; ++i) {
      style_details->add_external_styles(*i);
    }
  }

  bool is_in_body_yet_;
  int num_inline_style_blocks_;
  std::vector<std::string> external_styles_;

  const std::string document_url_;
  Results* results_;

  DISALLOW_COPY_AND_ASSIGN(StyleVisitor);
};

}  // namespace

namespace rules {

PutCssInTheDocumentHead::PutCssInTheDocumentHead() {}

const char* PutCssInTheDocumentHead::name() const {
  return kRuleName;
}

const char* PutCssInTheDocumentHead::header() const {
  return "Put CSS in the document head";
}

const char* PutCssInTheDocumentHead::documentation_url() const {
  return "rendering.html#PutCSSInHead";
}

bool PutCssInTheDocumentHead::AppendResults(const PagespeedInput& input,
                                            Results* results) {
  StyleVisitor::CheckDocument(input.dom_document(), results);

  // CheckDocument adds the results in post-order.  Reverse the order of
  // results here, so that main document comes first instead of last.
  // TODO What order do we _really_ want?  Pre-order?  Alphabetical?
  google::protobuf::RepeatedPtrField<Result>* repeated =
      results->mutable_results();
  for (int a = 0, b = repeated->size() - 1; a < b; ++a, --b) {
    repeated->SwapElements(a, b);
  }

  return true;
}

void PutCssInTheDocumentHead::FormatResults(const ResultVector& results,
                                            Formatter* formatter) {
  if (results.empty()) {
    return;
  }

  formatter->AddChild("CSS in the document body adversely impacts rendering "
                      "performance.");

  for (ResultVector::const_iterator i = results.begin(), end = results.end();
       i != end; ++i) {
    const Result& result = **i;
    if (result.resource_urls_size() != 1) {
      LOG(DFATAL) << "Unexpected number of resource URLs.  Expected 1, Got "
                  << result.resource_urls_size() << ".";
      continue;
    }

    const ResultDetails& details = result.details();
    if (details.HasExtension(StylesInBodyDetails::message_set_extension)) {
      const StylesInBodyDetails& style_details = details.GetExtension(
          StylesInBodyDetails::message_set_extension);

      Argument url(Argument::URL, result.resource_urls(0));
      Formatter* entry = formatter->AddChild(
          "$1 has CSS in the document body:", url);

      int num_inline_style_blocks =
          style_details.num_inline_style_blocks();
      if (num_inline_style_blocks > 0) {
        Argument num(Argument::INTEGER, num_inline_style_blocks);
        entry->AddChild("$1 style block(s) in the body should be moved to "
                        "the document head.", num);
      }

      for (int i = 0, size = style_details.external_styles_size();
           i < size; ++i) {
        Argument href(Argument::URL, style_details.external_styles(i));
        entry->AddChild("Link node $1 should be moved to the document head.",
                        href);
      }
    } else {
      Argument url(Argument::URL, result.resource_urls(0));
      formatter->AddChild("$1 has CSS in the document body.", url);
    }
  }
}

}  // namespace rules

}  // namespace pagespeed
