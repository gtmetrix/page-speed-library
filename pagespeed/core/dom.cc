/**
 * Copyright 2009 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "dom.h"

#include "base/logging.h"
#include "pagespeed/rules/rule_util.h"

#define NOT_IMPLEMENTED() do {                          \
    LOG(WARNING) << __FUNCTION__ << " not implemented"; \
  } while (false)

namespace pagespeed {

DomDocument::DomDocument() {}

DomDocument::~DomDocument() {}

std::string DomDocument::GetBaseUrl() const {
  return GetDocumentUrl();
}

std::string DomDocument::ResolveUri(const std::string& uri) const {
  return pagespeed::rules::ResolveUri(uri, GetBaseUrl());
}

DomElement::DomElement() {}

DomElement::~DomElement() {}

bool DomElement::GetAttributeByName(const std::string& name,
                                    std::string* attr_value) const {
  NOT_IMPLEMENTED();
  return false;
};

bool DomElement::GetStringPropertyByName(const std::string& name,
                                         std::string* property_value) const {
  NOT_IMPLEMENTED();
  return false;
}

bool DomElement::GetIntPropertyByName(const std::string& name,
                                      int* property_value) const {
  NOT_IMPLEMENTED();
  return false;
}

bool DomElement::GetCSSPropertyByName(const std::string& name,
                                      std::string* property_value) const {
  NOT_IMPLEMENTED();
  return false;
};

DomElementVisitor::DomElementVisitor() {}

DomElementVisitor::~DomElementVisitor() {}

}  // namespace pagespeed
