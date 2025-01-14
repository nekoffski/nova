#include "Sync.hh"

#include "Device.hh"

#ifdef SL_USE_VK
#include "starlight/renderer/gpu/vulkan/VKFence.hh"
#include "starlight/renderer/gpu/vulkan/VKSemaphore.hh"
#endif

namespace sl {

Fence::Fence(State state) : m_state(state) {}

}  // namespace sl
