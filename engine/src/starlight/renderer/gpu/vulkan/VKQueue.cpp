#include "VKQueue.hh"

namespace sl {

VKQueue::VKQueue(VkQueue handle) : m_handle(handle) {}

bool VKQueue::submit(const SubmitInfo& submitInfo) { return true; }

bool VKQueue::present(const PresentInfo& presentInfo) { return true; }

}  // namespace sl
