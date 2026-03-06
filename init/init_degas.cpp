/*
 * Copyright (C) 2025 The TWRP Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <android-base/properties.h>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

using android::base::GetProperty;
using std::string;

void property_override(string prop, string value)
{
    auto pi = (prop_info *)__system_property_find(prop.c_str());

    if (pi != nullptr)
        __system_property_update(pi, value.c_str(), value.size());
    else
        __system_property_add(prop.c_str(), prop.size(), value.c_str(), value.size());
}

void vendor_load_properties()
{
    string prop_partitions[] = {"", "vendor.", "odm."};
    for (const string &prop : prop_partitions)
    {
        property_override(string("ro.product.") + prop + string("brand"), "Xiaomi");
        property_override(string("ro.product.") + prop + string("manufacturer"), "Xiaomi");
        property_override(string("ro.product.") + prop + string("name"), "degas");
        property_override(string("ro.product.") + prop + string("device"), "degas");
        property_override(string("ro.product.") + prop + string("model"), "2406APNFAG");
        property_override(string("ro.product.") + prop + string("marketname"), "Xiaomi 14T");
        property_override(string("ro.product.") + prop + string("cert"), "2406APNFAG");
    }
    property_override("ro.bootimage.build.date.utc", "1767788844");
    property_override("ro.build.date.utc", "1767788844");
}
