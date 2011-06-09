// Copyright 2011 Google Inc. All Rights Reserved.
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

#include "pagespeed/core/timeline.h"

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/json/json_reader.h"
#include "base/logging.h"
#include "base/scoped_ptr.h"
#include "base/values.h"
#include "pagespeed/proto/timeline.pb.h"

using pagespeed::InstrumentationData;

namespace {

class ProtoPopulator {
 public:
  ProtoPopulator() : error_(false) {}

  void PopulateToplevel(const ListValue& json,
                        std::vector<const InstrumentationData*>* proto_out);
  void PopulateInstrumentationData(const DictionaryValue& json,
                                   InstrumentationData* instr);
  void PopulateStackFrame(const DictionaryValue& json,
                          pagespeed::StackFrame* out);
  void PopulateDataDictionary(InstrumentationData::RecordType type,
                              const DictionaryValue& json,
                              InstrumentationData::DataDictionary* out);

  bool error() { return error_; }

 private:
  bool error_;

  DISALLOW_COPY_AND_ASSIGN(ProtoPopulator);
};

void ProtoPopulator::PopulateToplevel(
    const ListValue& json,
    std::vector<const InstrumentationData*>* proto_out) {
  for (ListValue::const_iterator iter = json.begin(), end = json.end();
       iter != end; ++iter) {
    const Value* item = *iter;
    if (NULL == item || !item->IsType(Value::TYPE_DICTIONARY)) {
      error_ = true;
      LOG(WARNING) << "Top-level list item must be a dictionary";
      continue;
    }
    scoped_ptr<InstrumentationData> instr(new InstrumentationData);
    PopulateInstrumentationData(*static_cast<const DictionaryValue*>(item),
                                instr.get());
    proto_out->push_back(instr.release());
  }
}

void ProtoPopulator::PopulateInstrumentationData(
    const DictionaryValue& json,
    InstrumentationData* instr) {
  {
    std::string type_string;
    if (!json.GetString("type", &type_string)) {
      LOG(WARNING) << "Missing 'type' field";
      error_ = true;
      return;
    }

    if (type_string == "EventDispatch") {
      instr->set_type(InstrumentationData::EVENT_DISPATCH);
    } else if (type_string == "Layout") {
      instr->set_type(InstrumentationData::LAYOUT);
    } else if (type_string == "RecalculateStyles") {
      instr->set_type(InstrumentationData::RECALCULATE_STYLES);
    } else if (type_string == "Paint") {
      instr->set_type(InstrumentationData::PAINT);
    } else if (type_string == "ParseHTML") {
      instr->set_type(InstrumentationData::PARSE_HTML);
    } else if (type_string == "TimerInstall") {
      instr->set_type(InstrumentationData::TIMER_INSTALL);
    } else if (type_string == "TimerRemove") {
      instr->set_type(InstrumentationData::TIMER_REMOVE);
    } else if (type_string == "TimerFire") {
      instr->set_type(InstrumentationData::TIMER_FIRE);
    } else if (type_string == "XHRReadyStateChange") {
      instr->set_type(InstrumentationData::XHR_READY_STATE_CHANGE);
    } else if (type_string == "XHRLoad") {
      instr->set_type(InstrumentationData::XHR_LOAD);
    } else if (type_string == "EvaluateScript") {
      instr->set_type(InstrumentationData::EVALUATE_SCRIPT);
    } else if (type_string == "MarkTimeline") {
      instr->set_type(InstrumentationData::MARK_TIMELINE);
    } else if (type_string == "ResourceSendRequest") {
      instr->set_type(InstrumentationData::RESOURCE_SEND_REQUEST);
    } else if (type_string == "ResourceReceiveResponse") {
      instr->set_type(InstrumentationData::RESOURCE_RECEIVE_RESPONSE);
    } else if (type_string == "ResourceReceivedData") {
      instr->set_type(InstrumentationData::RESOURCE_RECEIVED_DATA);
    } else if (type_string == "ResourceFinish") {
      instr->set_type(InstrumentationData::RESOURCE_FINISH);
    } else if (type_string == "FunctionCall") {
      instr->set_type(InstrumentationData::FUNCTION_CALL);
    } else if (type_string == "GCEvent") {
      instr->set_type(InstrumentationData::GC_EVENT);
    } else if (type_string == "MarkDOMContent") {
      instr->set_type(InstrumentationData::MARK_DOM_CONTENT);
    } else if (type_string == "MarkLoad") {
      instr->set_type(InstrumentationData::MARK_LOAD);
    } else if (type_string == "ScheduleResourceRequest") {
      instr->set_type(InstrumentationData::SCHEDULE_RESOURCE_REQUEST);
    } else {
      LOG(WARNING) << "Unknown record type: " << type_string;
      error_ = true;
      return;
    }
  }

  DCHECK(instr->has_type());
  switch (instr->type()) {
    case InstrumentationData::LAYOUT:
    case InstrumentationData::MARK_DOM_CONTENT:
    case InstrumentationData::MARK_LOAD:
    case InstrumentationData::RECALCULATE_STYLES:
      // These types have no data payload, so we don't require the "data" field
      // to be present in the JSON for these types.
      break;

    default:
      DictionaryValue* data_json;
      if (!json.GetDictionary("data", &data_json)) {
        LOG(WARNING) << "Missing data dictionary";
        error_ = true;
      } else {
        PopulateDataDictionary(instr->type(), *data_json,
                               instr->mutable_data());
      }
      break;
  }

  {
    double time;
    if (json.GetReal("startTime", &time)) {
      instr->set_start_time(time);
    }
    if (json.GetReal("endTime", &time)) {
      instr->set_end_time(time);
    }
    int heap;
    if (json.GetInteger("usedHeapSize", &heap)) {
      instr->set_used_heap_size(heap);
    }
    if (json.GetInteger("totalHeapSize", &heap)) {
      instr->set_total_heap_size(heap);
    }
  }

  {
    ListValue* stack;
    if (json.GetList("stackTrace", &stack)) {
      for (ListValue::const_iterator iter = stack->begin(),
               end = stack->end(); iter != end; ++iter) {
        const Value* item = *iter;
        if (NULL == item || !item->IsType(Value::TYPE_DICTIONARY)) {
          error_ = true;
          LOG(WARNING) << "'stackTrace' list item must be a dictionary";
          continue;
        }
        PopulateStackFrame(*static_cast<const DictionaryValue*>(item),
                           instr->add_stack_trace());
      }
    }
  }

  {
    ListValue* children;
    if (json.GetList("children", &children)) {
      for (ListValue::const_iterator iter = children->begin(),
               end = children->end(); iter != end; ++iter) {
        const Value* item = *iter;
        if (NULL == item || !item->IsType(Value::TYPE_DICTIONARY)) {
          error_ = true;
          LOG(WARNING) << "'children' list item must be a dictionary";
          continue;
        }
        PopulateInstrumentationData(
            *static_cast<const DictionaryValue*>(item),
            instr->add_children());
      }
    }
  }
}

// This is a helper macro used to define the GET_*_DATA macros below.
#define GET_DATA_OF_TYPE(key, name, type, method)          \
  do {                                                     \
    type _temp_value;                                      \
    if (json.method(key, &_temp_value)) {                  \
      out->set_##name(_temp_value);                        \
    } else {                                               \
      LOG(INFO) << "Missing '" << key << "' field";        \
    }                                                      \
  } while (false)  // standard trick to require a semicolon when using macro

// The below macros are used to keep the code for PopulateDataDictionary from
// getting out of hand.  Using the macro requires `json`, `out`, and `error_`
// to be in scope.  Given a JSON key string and a protobuf field name, each of
// these macros will look for a JSON field of the appropriate type and set the
// protobuf field, and if that fails, print a warning and set error_ to true.
#define GET_BOOLEAN_DATA(key, name) \
  GET_DATA_OF_TYPE(key, name, bool, GetBoolean)
#define GET_DOUBLE_DATA(key, name) \
  GET_DATA_OF_TYPE(key, name, double, GetReal)
#define GET_INTEGER_DATA(key, name) \
  GET_DATA_OF_TYPE(key, name, int, GetInteger)
#define GET_STRING_DATA(key, name) \
  GET_DATA_OF_TYPE(key, name, std::string, GetString)

void ProtoPopulator::PopulateDataDictionary(
    InstrumentationData::RecordType type,
    const DictionaryValue& json,
    InstrumentationData::DataDictionary* out) {
  switch (type) {
    case InstrumentationData::EVALUATE_SCRIPT:
      GET_STRING_DATA("url", url);
      GET_INTEGER_DATA("lineNumber", line_number);
      break;
    case InstrumentationData::EVENT_DISPATCH:
      GET_STRING_DATA("type", type);
      break;
    case InstrumentationData::FUNCTION_CALL:
      GET_STRING_DATA("scriptName", script_name);
      GET_INTEGER_DATA("scriptLine", script_line);
      break;
    case InstrumentationData::GC_EVENT:
      GET_INTEGER_DATA("usedHeapSizeDelta", used_heap_size_delta);
      break;
    case InstrumentationData::MARK_TIMELINE:
      GET_STRING_DATA("message", message);
      break;
    case InstrumentationData::PAINT:
      GET_INTEGER_DATA("x", x);
      GET_INTEGER_DATA("y", y);
      GET_INTEGER_DATA("width", width);
      GET_INTEGER_DATA("height", height);
      break;
    case InstrumentationData::PARSE_HTML:
      GET_INTEGER_DATA("length", length);
      GET_INTEGER_DATA("startLine", start_line);
      GET_INTEGER_DATA("endLine", end_line);
      break;
    case InstrumentationData::RESOURCE_RECEIVED_DATA:
      GET_INTEGER_DATA("identifier", identifier);
      break;
    case InstrumentationData::RESOURCE_FINISH:
      GET_BOOLEAN_DATA("didFail", did_fail);
      GET_INTEGER_DATA("identifier", identifier);
      GET_DOUBLE_DATA("networkTime", network_time);
      break;
    case InstrumentationData::RESOURCE_RECEIVE_RESPONSE:
      GET_INTEGER_DATA("identifier", identifier);
      GET_INTEGER_DATA("statusCode", status_code);
      GET_STRING_DATA("mimeType", mime_type);
      GET_INTEGER_DATA("expectedContentLength", expected_content_length);
      break;
    case InstrumentationData::RESOURCE_SEND_REQUEST:
      GET_INTEGER_DATA("identifier", identifier);
      GET_STRING_DATA("requestMethod", request_method);
      GET_STRING_DATA("url", url);
      GET_BOOLEAN_DATA("isMainResource", is_main_resource);
      break;
    case InstrumentationData::SCHEDULE_RESOURCE_REQUEST:
      GET_STRING_DATA("url", url);
      break;
    case InstrumentationData::TIMER_FIRE:
      GET_INTEGER_DATA("timerId", timer_id);
      break;
    case InstrumentationData::TIMER_INSTALL:
      GET_BOOLEAN_DATA("singleShot", single_shot);
      GET_INTEGER_DATA("timeout", timeout);
      GET_INTEGER_DATA("timerId", timer_id);
    case InstrumentationData::TIMER_REMOVE:
      GET_INTEGER_DATA("timerId", timer_id);
      break;
    case InstrumentationData::XHR_LOAD:
      GET_STRING_DATA("url", url);
      break;
    case InstrumentationData::XHR_READY_STATE_CHANGE:
      GET_INTEGER_DATA("readyState", ready_state);
      GET_STRING_DATA("url", url);
      break;

    case InstrumentationData::LAYOUT:
    case InstrumentationData::MARK_DOM_CONTENT:
    case InstrumentationData::MARK_LOAD:
    case InstrumentationData::RECALCULATE_STYLES:
      // These types have no data payload.
      break;

    default:
      DCHECK(false) << "Missing DataDictionary population implementation for "
                    << "type = " << type;
      error_ = true;
      break;
  }
}

void ProtoPopulator::PopulateStackFrame(
    const DictionaryValue& json, pagespeed::StackFrame* out) {
  GET_STRING_DATA("url", url);
  GET_INTEGER_DATA("lineNumber", line_number);
  GET_INTEGER_DATA("columnNumber", column_number);
  GET_STRING_DATA("functionName", function_name);
}

#undef GET_STRING_DATA
#undef GET_INTEGER_DATA
#undef GET_DOUBLE_DATA
#undef GET_BOOLEAN_DATA
#undef GET_DATA_OF_TYPE

}  // namespace

namespace pagespeed {

bool CreateTimelineProtoFromJsonString(
    const std::string& json_string,
    std::vector<const InstrumentationData*>* proto_out) {
  scoped_ptr<const Value> json(base::JSONReader::Read(
      json_string,
      true));  // allow_trailing_comma
  if (json == NULL) {
    LOG(WARNING) << "JSON string failed to parse";
    return false;
  }
  if (!json->IsType(Value::TYPE_LIST)) {
    LOG(WARNING) << "Top-level JSON value must be a list";
    return false;
  }
  return CreateTimelineProtoFromJsonValue(
      *static_cast<const ListValue*>(json.get()), proto_out);
}

bool CreateTimelineProtoFromJsonValue(
    const ListValue& json,
    std::vector<const InstrumentationData*>* proto_out) {
  ProtoPopulator populator;
  populator.PopulateToplevel(json, proto_out);
  return !populator.error();
}

InstrumentationDataVisitor::InstrumentationDataVisitor() {}
InstrumentationDataVisitor::~InstrumentationDataVisitor() {}

// static
void InstrumentationDataVisitor::Traverse(
    InstrumentationDataVisitor* visitor,
    const std::vector<const InstrumentationData*>& data) {
  for (std::vector<const InstrumentationData*>::const_iterator
           it = data.begin(), end = data.end(); it != end; ++it) {
    Traverse(visitor, **it);
  }
}

// static
void InstrumentationDataVisitor::Traverse(InstrumentationDataVisitor* visitor,
                                          const InstrumentationData& data) {
  std::vector<const InstrumentationData*> stack;
  stack.push_back(&data);
  TraverseImpl(visitor, &stack);
  stack.pop_back();
}

// static
void InstrumentationDataVisitor::TraverseImpl(
    InstrumentationDataVisitor* visitor,
    std::vector<const InstrumentationData*>* stack) {
  const InstrumentationData& data = *stack->back();
  if (visitor->Visit(*stack)) {
    for (int i = 0; i < data.children_size(); ++i) {
      stack->push_back(&data.children(i));
      TraverseImpl(visitor, stack);
      stack->pop_back();
    }
  }
}

}  // namespace pagespeed
