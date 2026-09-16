#include "host_compatibility.hpp"
#include <cassert>

int main() {
    using twilight_hd_hud::supported_host_version;
    for (const char* version : {static_cast<const char*>(nullptr), "", "unknown", "1.4", "1.4.1",
            "1.4.1.259", "1.4.1.401", "1.4.1-401", "1.4.1.402bad",
            "1.4.1.", "1.4.1.99999999999999999999999"})
        assert(!supported_host_version(version));
    for (const char* version : {"1.4.1.402", "v1.4.1-402", "1.4.1.403",
            "1.4.2", "2.0.0"})
        assert(supported_host_version(version));
}
