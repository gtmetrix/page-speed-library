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

#include "pagespeed/core/pagespeed_init.h"

#include "googleurl/src/url_util.h"
#include "net/instaweb/htmlparse/public/html_keywords.h"
#include "pagespeed/l10n/register_locale.h"
#include "third_party/domain-registry/src/domain_registry/domain_registry.h"

namespace pagespeed {

void Init() {
  url_util::Initialize();
  net_instaweb::HtmlKeywords::Init();
  l10n::RegisterLocale::Freeze();
  InitializeDomainRegistry();
}

void ShutDown() {
  net_instaweb::HtmlKeywords::ShutDown();
  url_util::Shutdown();
}

}  // namespace pagespeed
