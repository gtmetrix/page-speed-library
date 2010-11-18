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

#include "pagespeed/rules/serve_resources_from_a_consistent_url.h"

#include <map>
#include <string>
#include <vector>

#include "base/string_util.h"
#include "pagespeed/core/formatter.h"
#include "pagespeed/core/pagespeed_input.h"
#include "pagespeed/core/resource.h"
#include "pagespeed/core/result_provider.h"
#include "pagespeed/core/rule_input.h"
#include "pagespeed/l10n/l10n.h"
#include "pagespeed/proto/pagespeed_output.pb.h"

namespace {

static const char* kCrossDomainXmlSuffix = "/crossdomain.xml";
static const size_t kCrossDomainXmlSuffixLen = strlen(kCrossDomainXmlSuffix);

struct ResourceBodyLessThan {
  bool operator()(const std::string* a,
                  const std::string* b) const {
    if (a->size() != b->size()) {
      // If the sizes differ, compare based on size. Comparing size is
      // more efficient than comparing actual string contents.
      return a->size() < b->size();
    }
    return *a < *b;
  }
};

// Map of ResourceSets, keyed by Resource bodies.
typedef std::map<const std::string*,
                 pagespeed::ResourceSet,
                 ResourceBodyLessThan> ResourcesWithSameBodyMap;

}  // namespace

namespace pagespeed {

namespace rules {

ServeResourcesFromAConsistentUrl::ServeResourcesFromAConsistentUrl()
    : pagespeed::Rule(pagespeed::InputCapabilities(
        pagespeed::InputCapabilities::RESPONSE_BODY)) {
}

const char* ServeResourcesFromAConsistentUrl::name() const {
  return "ServeResourcesFromAConsistentUrl";
}

const char* ServeResourcesFromAConsistentUrl::header() const {
  // TRANSLATOR: TODO: add a translator comment describing this string
  return _("Serve resources from a consistent URL");
}

const char* ServeResourcesFromAConsistentUrl::documentation_url() const {
  return "payload.html#duplicate_resources";
}

bool ServeResourcesFromAConsistentUrl::
AppendResults(const RuleInput& rule_input, ResultProvider* provider) {
  const PagespeedInput& input = rule_input.pagespeed_input();
  ResourcesWithSameBodyMap map;
  for (int idx = 0, num = input.num_resources(); idx < num; ++idx) {
    const Resource& resource = input.GetResource(idx);
    if (resource.GetResourceType() == OTHER) {
      // Don't process resource types that we don't explicitly care
      // about.
      continue;
    }
    if (resource.GetResponseBody().empty()) {
      // Exclude responses with empty bodies.
      continue;
    }
    const std::string& url = resource.GetRequestUrl();
    if (kCrossDomainXmlSuffixLen <= url.size()) {
      const size_t offset = url.size() - kCrossDomainXmlSuffixLen;
      if (url.find(kCrossDomainXmlSuffix, offset) == offset) {
        // Looks like an Adobe crossdomain.xml resource, which may be
        // hosted on different domains in order to enable cross-domain
        // communication in Flash, so skip it. See
        // http://kb2.adobe.com/cps/142/tn_14213.html for more
        // information.
        continue;
      }
    }
    map[&resource.GetResponseBody()].insert(&resource);
  }

  for (ResourcesWithSameBodyMap::const_iterator map_iter = map.begin(),
           map_iter_end = map.end();
       map_iter != map_iter_end;
       ++map_iter) {
    const ResourceSet &resources = map_iter->second;
    if (resources.size() > 1) {
      Result* result = provider->NewResult();
      const Resource &first_resource = **resources.begin();
      const int requests_saved = resources.size() - 1;
      const int response_bytes_saved =
          (first_resource.GetResponseBody().size() * requests_saved);

      Savings* savings = result->mutable_savings();
      savings->set_requests_saved(requests_saved);
      savings->set_response_bytes_saved(response_bytes_saved);

      for (ResourceSet::const_iterator resource_iter = resources.begin(),
               resource_iter_end = resources.end();
           resource_iter != resource_iter_end;
           ++resource_iter) {
        const Resource &resource = **resource_iter;
        result->add_resource_urls(resource.GetRequestUrl());
      }
    }
  }

  return true;
}

void ServeResourcesFromAConsistentUrl::FormatResults(
    const ResultVector& results, Formatter* formatter) {
  for (ResultVector::const_iterator iter = results.begin(),
           end = results.end();
       iter != end;
       ++iter) {
    const Result& result = **iter;
    Argument num_resources_arg(
        Argument::INTEGER, result.savings().requests_saved());
    Argument num_bytes_arg(
        Argument::BYTES, result.savings().response_bytes_saved());
    Formatter* body = formatter->AddChild(
        // TRANSLATOR: TODO: add a translator comment describing this string
        _("The following resources have identical contents, but are served "
          "from different URLs.  Serve these resources from a consistent URL "
          "to save $1 request(s) and $2."),
        num_resources_arg,
        num_bytes_arg);
    for (int url_idx = 0; url_idx < result.resource_urls_size(); url_idx++) {
      Argument url(Argument::URL, result.resource_urls(url_idx));
      body->AddChild("$1", url);
    }
  }
}

}  // namespace rules

}  // namespace pagespeed
