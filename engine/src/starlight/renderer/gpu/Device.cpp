#include "Device.hh"

#ifdef SL_USE_VK
#include "starlight/renderer/gpu/vulkan/VKDevice.hh"
#endif

namespace sl {

OwningPtr<Device> Device::create(Context& context) {
#ifdef SL_USE_VK
    return createOwningPtr<vk::VKDevice>(context.getWindow(), context.getConfig());
#endif
}

}  // namespace sl
