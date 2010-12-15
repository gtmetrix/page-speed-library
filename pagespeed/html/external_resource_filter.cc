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

#include "pagespeed/html/external_resource_filter.h"

#include "net/instaweb/htmlparse/public/empty_html_filter.h"
#include "net/instaweb/htmlparse/public/html_parse.h"
#include "pagespeed/core/uri_util.h"

namespace pagespeed {

namespace {

void ResolveExternalResourceUrls(
    std::vector<std::string>* external_resource_urls,
    const DomDocument* document,
    const std::string& document_url) {
  // Resolve URLs relative to their document.
  for (std::vector<std::string>::iterator
           it = external_resource_urls->begin(),
           end = external_resource_urls->end();
       it != end;
       ++it) {
    std::string resolved_uri;
    if (!uri_util::ResolveUriForDocumentWithUrl(*it,
                                                document,
                                                document_url,
                                                &resolved_uri)) {
      // We failed to resolve relative to the document, so try to
      // resolve relative to the document's URL. This will be
      // correct unless the document contains a <base> tag.
      resolved_uri = uri_util::ResolveUri(*it, document_url);
    }
    *it = resolved_uri;
  }
}

}  // namespace

namespace html {

ExternalResourceFilter::ExternalResourceFilter(
    net_instaweb::HtmlParse* html_parse)
    : script_atom_(html_parse->Intern("script")),
      src_atom_(html_parse->Intern("src")),
      link_atom_(html_parse->Intern("link")),
      rel_atom_(html_parse->Intern("rel")),
      href_atom_(html_parse->Intern("href")) {
}

ExternalResourceFilter::~ExternalResourceFilter() {}

void ExternalResourceFilter::StartDocument() {
  external_resource_urls_.clear();
}

void ExternalResourceFilter::StartElement(net_instaweb::HtmlElement* element) {
  net_instaweb::Atom tag = element->tag();
  if (tag == script_atom_) {
    const char* src = element->AttributeValue(src_atom_);
    if (src != NULL) {
      external_resource_urls_.push_back(src);
    }
    return;
  }

  if (tag == link_atom_) {
    const char* rel = element->AttributeValue(rel_atom_);
    if (rel == NULL || !LowerCaseEqualsASCII(rel, "stylesheet")) {
      return;
    }
    const char* href = element->AttributeValue(href_atom_);
    if (href != NULL) {
      external_resource_urls_.push_back(href);
    }
    return;
  }
}

bool ExternalResourceFilter::GetExternalResourceUrls(
    std::vector<std::string>* out,
    const DomDocument* document,
    const std::string& document_url) const {
  // We want to uniqueify the list of URLs. The easiest way to do that
  // is to copy the contents of the vector to a set, and then back to
  // a vector.
  std::vector<std::string> tmp(external_resource_urls_);
  ResolveExternalResourceUrls(
      &tmp, document, document_url);
  std::set<std::string> unique(tmp.begin(), tmp.end());
  out->assign(unique.begin(), unique.end());
  return !out->empty();
}

}  // namespace html

}  // namespace pagespeed
