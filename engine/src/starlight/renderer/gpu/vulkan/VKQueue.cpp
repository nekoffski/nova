#include "VKQueue.hh"

namespace sl {

VKQueue::VKQueue(VkQueue handle) : m_handle(handle) {}

void VKQueue::submit(
  CommandBuffer& commandBuffer, Semaphore* waitSemaphore, Semaphore* signalSemaphore,
  Fence* fence
) {}

}  // namespace sl
