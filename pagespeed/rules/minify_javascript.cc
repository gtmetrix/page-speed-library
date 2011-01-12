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

#include "pagespeed/rules/minify_javascript.h"

#include <string>

#include "base/logging.h"
#include "pagespeed/core/resource.h"
#include "pagespeed/l10n/l10n.h"
#include "pagespeed/proto/pagespeed_output.pb.h"
#include "pagespeed/jsminify/js_minify.h"

namespace pagespeed {

namespace rules {

namespace {

// This cost weight yields an avg score of 84 and a median score of 97
// for the top 100 websites.
const double kCostWeight = 3.5;

class JsMinifier : public Minifier {
 public:
  explicit JsMinifier(bool save_optimized_content)
      : save_optimized_content_(save_optimized_content) {}

  // Minifier interface:
  virtual const char* name() const;
  virtual UserFacingString header_format() const;
  virtual const char* documentation_url() const;
  virtual UserFacingString body_format() const;
  virtual UserFacingString child_format() const;
  virtual const MinifierOutput* Minify(const Resource& resource) const;

 private:
  bool save_optimized_content_;

  DISALLOW_COPY_AND_ASSIGN(JsMinifier);
};

const char* JsMinifier::name() const {
  return "MinifyJavaScript";
}

UserFacingString JsMinifier::header_format() const {
  // TRANSLATOR: Name of a Page Speed rule. Here, minify means "remove
  // whitespace and comments". The goal is to reduce the size of the
  // JavaScript file by removing the parts that are unnecessary.
  return _("Minify JavaScript");
}

const char* JsMinifier::documentation_url() const {
  return "payload.html#MinifyJS";
}

UserFacingString JsMinifier::body_format() const {
  // TRANSLATOR: Heading that describes the savings possible from minifying
  // resources.  "$1" will be replaced by the absolute number of bytes or
  // kilobytes that can be saved (e.g. "5 bytes" or "23.2KiB").  "$2" will be
  // replaced by the percent savings (e.g. "50").
  return _("Minifying the following JavaScript resources could "
           "reduce their size by $1 ($2% reduction).");
}

UserFacingString JsMinifier::child_format() const {
  // TRANSLATOR: Subheading that describes the savings possible from minifying a
  // single resource. "$1" is a format token that will be replaced by the URL of
  // the resource. "$2" will be replaced bythe absolute number of bytes or
  // kilobytes that can be saved (e.g. "5 bytes" or "23.2KiB"). "$3" will be
  // replaced by the percent savings (e.g. "50").
  return _("Minifying $1 could save $2 ($3% reduction).");
}

const MinifierOutput* JsMinifier::Minify(const Resource& resource) const {
  if (resource.GetResourceType() != JS) {
    return new MinifierOutput();
  }

  const std::string& input = resource.GetResponseBody();
  if (save_optimized_content_) {
    std::string minified_js;
    if (!jsminify::MinifyJs(input, &minified_js)) {
      return NULL; // error
    }
    return new MinifierOutput(input.size() - minified_js.size(),
                              minified_js, "text/javascript");
  } else {
    int minified_js_size = 0;
    if (!jsminify::GetMinifiedJsSize(input, &minified_js_size)) {
      return NULL; // error
    }
    return new MinifierOutput(input.size() - minified_js_size);
  }
};

}  // namespace

MinifyJavaScript::MinifyJavaScript(bool save_optimized_content)
    : MinifyRule(new JsMinifier(save_optimized_content)) {}

int MinifyJavaScript::ComputeScore(const InputInformation& input_info,
                                   const RuleResults& results) {
  WeightedCostBasedScoreComputer score_computer(
      &results, input_info.javascript_response_bytes(), kCostWeight);
  return score_computer.ComputeScore();
}

}  // namespace rules

}  // namespace pagespeed
