#pragma once

#include "starlight/core/Core.hh"

namespace sl {

struct Pipeline : public NonCopyable, public NonMovable {
    virtual void bind() = 0;
};

}  // namespace sl
