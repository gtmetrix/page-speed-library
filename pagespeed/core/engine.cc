// Copyright 2009 Google Inc.
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

#include "pagespeed/core/engine.h"

#include <string>

#include "base/logging.h"
#include "base/stl_util-inl.h"  // for STLDeleteContainerPointers
#include "pagespeed/core/formatter.h"
#include "pagespeed/core/pagespeed_input.h"
#include "pagespeed/core/pagespeed_version.h"
#include "pagespeed/core/resource.h"
#include "pagespeed/core/result_provider.h"
#include "pagespeed/core/rule.h"
#include "pagespeed/core/rule_input.h"
#include "pagespeed/proto/pagespeed_output.pb.h"

namespace pagespeed {

namespace {

void FormatRuleResults(const RuleResults& rule_results,
                       const InputInformation& input_info,
                       Rule* rule,
                       const ResultFilter& filter,
                       RuleFormatter* root_formatter) {
  // Sort results according to presentation order
  ResultVector sorted_results;
  for (int result_idx = 0, end = rule_results.results_size();
       result_idx < end; ++result_idx) {
    const Result& result = rule_results.results(result_idx);

    if (filter.IsAccepted(result)) {
      sorted_results.push_back(&rule_results.results(result_idx));
    }
  }
  rule->SortResultsInPresentationOrder(&sorted_results);

  Formatter* rule_formatter =
      root_formatter->AddHeader(*rule, rule_results.rule_score());
  if (!sorted_results.empty()) {
    rule->FormatResults(sorted_results, rule_formatter);
  }
}

}  // namespace

Engine::Engine(std::vector<Rule*>* rules)
    : rules_(*rules), init_has_been_called_(false) {
  // Now that we've transferred the rule ownership to our local
  // vector, clear the passed in vector.
  rules->clear();
}

Engine::~Engine() {
  STLDeleteContainerPointers(rules_.begin(), rules_.end());
}

void Engine::Init() {
  CHECK(!init_has_been_called_);

  PopulateNameToRuleMap();
  init_has_been_called_ = true;
}

void Engine::PopulateNameToRuleMap() {
  for (std::vector<Rule*>::const_iterator iter = rules_.begin(),
           end = rules_.end();
       iter != end;
       ++iter) {
    Rule *rule = *iter;
    if (name_to_rule_map_.find(rule->name()) != name_to_rule_map_.end()) {
      LOG(DFATAL) << "Found duplicate rule while populating name to rule map.  "
                  << rule->name();
    }
    name_to_rule_map_[rule->name()] = rule;
  }
}

bool Engine::ComputeResults(const PagespeedInput& pagespeed_input,
                            Results* results) const {
  CHECK(init_has_been_called_);

  if (!pagespeed_input.is_frozen()) {
    LOG(DFATAL) << "Attempting to ComputeResults with non-frozen input.";
    return false;
  }

  PrepareResults(pagespeed_input, results);

  const RuleInput rule_input(pagespeed_input);
  int total_score = 0;
  int total_weights = 0;

  bool success = true;
  for (std::vector<Rule*>::const_iterator iter = rules_.begin(),
           end = rules_.end();
       iter != end;
       ++iter) {
    Rule* rule = *iter;
    RuleResults* rule_results = results->add_rule_results();
    rule_results->set_rule_name(rule->name());

    ResultProvider provider(*rule, rule_results);
    const bool rule_success = rule->AppendResults(rule_input, &provider);
    if (!rule_success) {
      // Record that the rule encountered an error.
      results->add_error_rules(rule->name());
      success = false;
    }

    int score = 100;
    if (rule_results->results_size() > 0) {
      score = rule->ComputeScore(results->input_info(), *rule_results);
      if (score > 100 || score < -1) {
        // Note that the value -1 indicates a valid score could not be
        // computed, so we need to allow it.
        LOG(ERROR) << "Score for " << rule->name() << " out of bounds: "
                   << score;
        score = std::max(-1, std::min(100, score));
      }
    }

    // Instead of using a -1 to indicate an error, we just don't set rule_score
    if (rule_success && score >= 0) {
      rule_results->set_rule_score(score);

      total_score += score;
      total_weights++;
    }
  }

  // TODO(mdsteele): better scoring algorithm?
  // Calculate the overall score (currently just the mean of all rule scores)
  if (total_weights)
    results->set_score(total_score / total_weights);

  if (!results->IsInitialized()) {
    LOG(DFATAL) << "Failed to fully initialize results object.";
    return false;
  }

  return success;
}

bool Engine::FormatResults(const Results& results,
                           const ResultFilter& filter,
                           RuleFormatter* formatter) const {
  CHECK(init_has_been_called_);

  if (!results.IsInitialized()) {
    LOG(ERROR) << "Results instance not fully initialized.";
    return false;
  }

  bool success = true;
  for (int idx = 0, end = results.rule_results_size(); idx < end; ++idx) {
    const RuleResults& rule_results = results.rule_results(idx);
    const std::string& rule_name = rule_results.rule_name();
    NameToRuleMap::const_iterator rule_iter = name_to_rule_map_.find(rule_name);
    if (rule_iter == name_to_rule_map_.end()) {
      // No rule registered to handle the given rule name. This could
      // happen if the Results object was generated with a different
      // version of the Page Speed library, so we do not want to CHECK
      // that the Rule is non-null here.
      LOG(WARNING) << "Unable to find rule instance with name " << rule_name;
      success = false;
      continue;
    }

    Rule* rule = rule_iter->second;
    FormatRuleResults(rule_results, results.input_info(), rule, filter,
                      formatter);
  }

  formatter->Done();
  return success;
}

bool Engine::ComputeAndFormatResults(const PagespeedInput& input,
                                     const ResultFilter& filter,
                                     RuleFormatter* formatter) const {
  CHECK(init_has_been_called_);

  Results results;
  bool success = ComputeResults(input, &results);
  success = FormatResults(results, filter, formatter) && success;
  return success;
}

void Engine::PrepareResults(const PagespeedInput& input,
                            Results* results) const {
  for (std::vector<Rule*>::const_iterator it = rules_.begin(),
           end = rules_.end(); it != end; ++it) {
    const Rule& rule = **it;
    results->add_rules(rule.name());
  }
  results->mutable_input_info()->CopyFrom(*input.input_information());
  GetPageSpeedVersion(results->mutable_version());
}

ResultFilter::ResultFilter() {}
ResultFilter::~ResultFilter() {}

AlwaysAcceptResultFilter::AlwaysAcceptResultFilter() {}
AlwaysAcceptResultFilter::~AlwaysAcceptResultFilter() {}

bool AlwaysAcceptResultFilter::IsAccepted(const Result& result) const {
  return true;
}

}  // namespace pagespeed
