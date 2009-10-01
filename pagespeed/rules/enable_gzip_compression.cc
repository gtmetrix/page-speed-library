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

#include "pagespeed/rules/enable_gzip_compression.h"

#include <string>

#include "base/logging.h"
#include "base/string_util.h"  // for StringToInt
#include "pagespeed/core/formatter.h"
#include "pagespeed/core/pagespeed_input.h"
#include "pagespeed/core/resource.h"
#include "pagespeed/proto/pagespeed_output.pb.h"

namespace pagespeed {

namespace rules {

EnableGzipCompression::EnableGzipCompression() : Rule("EnableGzipCompression") {
}

bool EnableGzipCompression::AppendResults(const PagespeedInput& input,
                                          Results* results) {
  for (int idx = 0, num = input.num_resources(); idx < num; ++idx) {
    const Resource& resource = input.GetResource(idx);
    if (!isViolation(resource)) {
      continue;
    }

    Result* result = results->add_results();
    result->set_rule_name(name());

    int length = GetContentLength(resource);
    int bytes_saved = 2 * length / 3;

    Savings* savings = result->mutable_savings();
    savings->set_response_bytes_saved(bytes_saved);

    result->add_resource_urls(resource.GetRequestUrl());
  }

  return true;
}

void EnableGzipCompression::FormatResults(
    const ResultVector& results, Formatter* formatter) {
  Formatter* header = formatter->AddChild("Enable Gzip");

  int total_bytes_saved = 0;

  for (ResultVector::const_iterator iter = results.begin(),
           end = results.end();
       iter != end;
       ++iter) {
    const Result& result = **iter;
    const Savings& savings = result.savings();
    total_bytes_saved += savings.response_bytes_saved();
  }

  Argument arg(Argument::BYTES, total_bytes_saved);
  Formatter* body = header->AddChild("Compressing the following "
                                     "resources with gzip could reduce "
                                     "their transfer size by about two "
                                     "thirds (~$1).",
                                     arg);

  for (ResultVector::const_iterator iter = results.begin(),
           end = results.end();
       iter != end;
       ++iter) {
    const Result& result = **iter;
    CHECK(result.resource_urls_size() == 1);
    Argument url(Argument::URL, result.resource_urls(0));
    Argument savings(Argument::BYTES, result.savings().response_bytes_saved());
    body->AddChild("Compressing $1 could save ~$2", url,
                   savings);
  }
}

bool EnableGzipCompression::isCompressed(const Resource& resource) const {
  const std::string& encoding = resource.GetResponseHeader("Content-Encoding");
  return encoding == "gzip" || encoding == "deflate";
}

bool EnableGzipCompression::isText(const Resource& resource) const {
  ResourceType type = resource.GetResourceType();
  ResourceType text_types[] = { HTML, TEXT, JS, CSS };
  for (int idx = 0; idx < arraysize(text_types); ++idx) {
    if (type == text_types[idx]) {
      return true;
    }
  }

  return false;
}

bool EnableGzipCompression::isViolation(const Resource& resource) const {
  return !isCompressed(resource) &&
      isText(resource) &&
      GetContentLength(resource) >= 150;
}

int EnableGzipCompression::GetContentLength(const Resource& resource) const {
  const std::string& length_header =
      resource.GetResponseHeader("Content-Length");
  if (!length_header.empty()) {
    int output = 0;
    bool status = StringToInt(length_header, &output);
    DCHECK(status);
    return output;
  } else {
    return resource.GetResponseBody().size();
  }

  return 0;
}

}  // namespace rules

}  // namespace pagespeed
