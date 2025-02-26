#include "CommandBuffer.hh"

#include "Queue.hh"

#ifdef SL_USE_VK
#include "vulkan/VulkanDevice.hh"
#include "vulkan/VulkanCommandBuffer.hh"
#endif

namespace sl {

CommandBuffer::Immediate::Immediate(Queue& queue
) : m_commandBuffer(CommandBuffer::create()), m_queue(queue) {
    m_commandBuffer->begin(CommandBuffer::BeginFlags::singleUse);
}

CommandBuffer::Immediate::~Immediate() {
    m_commandBuffer->end();
    m_queue.submit(Queue::SubmitInfo{ .commandBuffer = *m_commandBuffer });
    m_queue.wait();
}

CommandBuffer& CommandBuffer::Immediate::get() { return *m_commandBuffer; }

CommandBuffer::Immediate::operator CommandBuffer&() { return get(); }

UniquePtr<CommandBuffer> CommandBuffer::create(Severity severity) {
#ifdef SL_USE_VK
    return UniquePtr<vk::VulkanCommandBuffer>::create(
      static_cast<vk::VulkanDevice&>(Device::get().getImpl()), severity
    );
#else
    log::panic("GPU API vendor not specified");
#endif
}

}  // namespace sl
