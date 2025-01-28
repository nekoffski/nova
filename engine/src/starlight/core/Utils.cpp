#include "Utils.hh"

namespace sl {

Range Range::aligned(u64 offset, u64 size, u64 granularity) {
    return Range(
      getAlignedValue(offset, granularity), getAlignedValue(size, granularity)
    );
}

}  // namespace sl
