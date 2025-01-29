#include "Utils.hh"

#include <boost/algorithm/string.hpp>

namespace sl {

std::vector<std::string> split(const std::string& input, char separator) {
    std::vector<std::string> output;
    return boost::split(output, input, boost::is_any_of(std::string{ separator }));
}

Range Range::aligned(u64 offset, u64 size, u64 granularity) {
    return Range(
      getAlignedValue(offset, granularity), getAlignedValue(size, granularity)
    );
}

}  // namespace sl
