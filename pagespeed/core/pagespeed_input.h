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

#ifndef PAGESPEED_CORE_PAGESPEED_INPUT_H_
#define PAGESPEED_CORE_PAGESPEED_INPUT_H_

#include <map>
#include <string>
#include <vector>

#include "base/basictypes.h"

namespace pagespeed {

class Resource;

typedef std::vector<const Resource*> ResourceVector;
typedef std::map<std::string, ResourceVector> HostResourceMap;

/**
 * Input set representation
 */
class PagespeedInput {
 public:
  PagespeedInput();
  virtual ~PagespeedInput();

  // Setters
  // Adds a resource to the list.
  // Ownership of the resource is transfered over to the
  // PagespeedInput object.
  void AddResource(const Resource* resource);

  // Resource access.
  int num_resources() const;
  const Resource& GetResource(int idx) const;
  const HostResourceMap* GetHostResourceMap() const;

 private:
  std::vector<const Resource*> resources_;
  HostResourceMap host_resource_map_;

  DISALLOW_COPY_AND_ASSIGN(PagespeedInput);
};

}  // namespace pagespeed

#endif  // PAGESPEED_CORE_PAGESPEED_INPUT_H_
