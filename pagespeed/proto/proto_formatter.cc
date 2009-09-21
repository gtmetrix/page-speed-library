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

#include "pagespeed/proto/proto_formatter.h"

#include <string>
#include <vector>

#include "base/logging.h"
#include "pagespeed/proto/pagespeed_output.pb.h"

namespace pagespeed {

namespace proto {

ProtoFormatter::ProtoFormatter(std::vector<ResultText*>* results)
    : results_(results),
      result_text_(NULL) {
}

ProtoFormatter::ProtoFormatter(ResultText* result_text)
    : results_(NULL),
      result_text_(result_text) {
}

Formatter* ProtoFormatter::NewChild(
    const std::string& format_str,
    const std::vector<const Argument*>& arguments) {
  ResultText* new_child = NULL;
  if (results_ != NULL) {
    CHECK(result_text_ == NULL);
    new_child = new ResultText;
    results_->push_back(new_child);
  } else {
    CHECK(results_ == NULL);
    new_child = result_text_->add_children();
  }

  Format(new_child, format_str, arguments);
  return new ProtoFormatter(new_child);
}

void ProtoFormatter::Format(ResultText* result_text,
                            const std::string& format_str,
                            const std::vector<const Argument*>& arguments) {
  CHECK(result_text != NULL);

  result_text->set_format(format_str);

  for (std::vector<const Argument*>::const_iterator iter = arguments.begin(),
           end = arguments.end();
       iter != end;
       ++iter) {
    const Argument* arg = *iter;
    FormatArgument* format_arg = result_text->add_args();
    switch (arg->type()) {
      case Argument::INTEGER:
        format_arg->set_type(FormatArgument::INT_LITERAL);
        format_arg->set_int_value(arg->int_value());
        break;
      case Argument::BYTES:
        format_arg->set_type(FormatArgument::BYTES);
        format_arg->set_int_value(arg->int_value());
        break;
      case Argument::STRING:
        format_arg->set_type(FormatArgument::STRING_LITERAL);
        format_arg->set_string_value(arg->string_value());
        break;
      case Argument::URL:
        format_arg->set_type(FormatArgument::URL);
        format_arg->set_string_value(arg->string_value());
        break;
      default:
        CHECK(false);
        break;
    }
  }
}

}  // namespace proto

}  // namespace pagespeed
