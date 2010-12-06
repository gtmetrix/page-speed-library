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
// Author: aoates@google.com (Andrew Oates)

#include "pagespeed/formatters/proto_formatter.h"

#include <string>

#include "base/logging.h"
#include "pagespeed/l10n/localizer.h"
#include "pagespeed/proto/pagespeed_proto_formatter.pb.h"

using pagespeed::FormatString;
using pagespeed::FormattedResults;
using pagespeed::FormattedRuleResults;
using pagespeed::FormattedUrlBlockResults;
using pagespeed::FormattedUrlResult;
using pagespeed::l10n::Localizer;

namespace pagespeed {

namespace formatters {

namespace {

// Fills in a FormatString proto from a FormatterParameters object
void FillFormatString(const Localizer* loc,
                      const FormatterParameters& params,
                      FormatString& out) {
  out.set_format(loc->LocalizeString(std::string(params.format_str())));

  for (unsigned int i = 0; i < params.arguments().size(); ++i) {
    const Argument* arg = params.arguments()[i];
    FormatArgument* format_arg = out.add_args();
    switch (arg->type()) {
      case Argument::INTEGER:
        format_arg->set_type(FormatArgument::INT_LITERAL);
        format_arg->set_int_value(arg->int_value());
        format_arg->set_localized_value(loc->LocalizeInt(arg->int_value()));
        break;
      case Argument::BYTES:
        format_arg->set_type(FormatArgument::BYTES);
        format_arg->set_int_value(arg->int_value());
        format_arg->set_localized_value(loc->LocalizeBytes(arg->int_value()));
        break;
      case Argument::DURATION:
        format_arg->set_type(FormatArgument::DURATION);
        format_arg->set_int_value(arg->int_value());
        format_arg->set_localized_value(
            loc->LocalizeTimeDuration(arg->int_value()));
        break;
      case Argument::STRING:
        format_arg->set_type(FormatArgument::STRING_LITERAL);
        format_arg->set_string_value(arg->string_value());
        format_arg->set_localized_value(
            loc->LocalizeString(arg->string_value()));
        break;
      case Argument::URL:
        format_arg->set_type(FormatArgument::URL);
        format_arg->set_string_value(arg->string_value());
        format_arg->set_localized_value(loc->LocalizeUrl(arg->string_value()));
        break;
      default:
        LOG(DFATAL) << "Unknown argument type "
                    << arg->type();
        format_arg->set_type(FormatArgument::STRING_LITERAL);
        format_arg->set_string_value("?");
        format_arg->set_localized_value("?");
        break;
    }
  }
}

// Formatters for each of the proto messages that need to be constructed.  Each
// reinforces that the input its given matches the appropriate structure
// (logging to DFATAL if not), and constructs the appropriate child messages.
// TODO(aoates): move the structure constraints into the Formatter interface

// A formatter used when there should be no children
class DeadEndFormatter : public Formatter {
 protected:
  virtual Formatter* NewChild(const FormatterParameters& params) {
    LOG(DFATAL) << "NewChild() called on DeadEndFormatter ---"
                << " a Rule is not structuring its output correctly";
    return new DeadEndFormatter();
  }
  virtual void DoneAddingChildren() {}
};

class FormattedUrlResultFormatter : public Formatter {
 public:
  FormattedUrlResultFormatter(const Localizer* l,
                              FormattedUrlResult* url_result)
      : localizer_(l), url_result_(url_result) {}
 protected:
  // Called for each "detail" line about the URL's result (see
  // MinimizeRequestSize rule for an example)
  virtual Formatter* NewChild(const FormatterParameters& params) {
    FormatString* detail = url_result_->add_details();
    FillFormatString(localizer_, params, *detail);

    return new DeadEndFormatter();
  }

  virtual void DoneAddingChildren() {
  }

 private:
  const Localizer* localizer_;
  FormattedUrlResult* url_result_;
};

class FormattedUrlBlockResultsFormatter : public Formatter {
 public:
  FormattedUrlBlockResultsFormatter(const Localizer* l,
                                    FormattedUrlBlockResults* url_block_results)
      : localizer_(l), url_block_results_(url_block_results) {}
 protected:
  // Called once for each URL in a given block
  virtual Formatter* NewChild(const FormatterParameters& params) {
    FormattedUrlResult* url_result = url_block_results_->add_urls();
    FillFormatString(localizer_, params, *url_result->mutable_result());

    return new FormattedUrlResultFormatter(localizer_, url_result);
  }

  virtual void DoneAddingChildren() {}

 private:
  const Localizer* localizer_;
  FormattedUrlBlockResults* url_block_results_;
};

class FormattedRuleResultsFormatter : public Formatter {
 public:
  FormattedRuleResultsFormatter(const Localizer* l,
                                FormattedRuleResults* rule_results)
      : localizer_(l), rule_results_(rule_results) {}
 protected:
  // Called once for each block of URLs
  virtual Formatter* NewChild(const FormatterParameters& params) {
    FormattedUrlBlockResults* url_block = rule_results_->add_url_blocks();
    FillFormatString(localizer_, params, *url_block->mutable_header());

    return new FormattedUrlBlockResultsFormatter(localizer_, url_block);
  }

  virtual void DoneAddingChildren() {}

 private:
  const Localizer* localizer_;
  FormattedRuleResults* rule_results_;
};

} // namespace

ProtoFormatter::ProtoFormatter(const Localizer* localizer,
                                       FormattedResults* results)
    : localizer_(localizer), results_(results) {
  DCHECK(localizer_);
  DCHECK(results_);
}

Formatter* ProtoFormatter::AddHeader(const Rule& rule, int score) {
  FormattedRuleResults* rule_results = results_->add_rule_results();
  rule_results->set_rule(rule.name());
  rule_results->set_localized_rule_name(
      localizer_->LocalizeString(std::string(rule.header())));

  return new FormattedRuleResultsFormatter(localizer_, rule_results);
}

Formatter* ProtoFormatter::NewChild(const FormatterParameters& params) {
  // There should be no non-rule children for a FormattedResults
  LOG(DFATAL) << "NewChild() called on ProtoFormatter, which cannot"
              << " have any non-rule children";
  return new DeadEndFormatter();
}

void ProtoFormatter::DoneAddingChildren() {
}

} // namespace formatters

} // namespace pagespeed
