#pragma once

#include "starlight/core/Core.hh"
#include "starlight/core/Context.hh"
#include "starlight/core/memory/Memory.hh"

namespace sl {

class Device : public NonCopyable, public NonMovable {
public:
    static OwningPtr<Device> create(Context& context);
};

}  // namespace sl
