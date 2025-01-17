#include "CommandBuffer.hh"

#include "Queue.hh"

namespace sl {

ImmediateCommandBuffer::ImmediateCommandBuffer(
  OwningPtr<CommandBuffer> commandBuffer, Queue& queue
) : m_commandBuffer(std::move(commandBuffer)), m_queue(queue) {
    m_commandBuffer->begin(CommandBuffer::BeginFlags::singleUse);
}

ImmediateCommandBuffer::~ImmediateCommandBuffer() {
    m_commandBuffer->end();
    m_queue.submit(Queue::SubmitInfo{ .commandBuffer = *m_commandBuffer });
    m_queue.wait();
}

CommandBuffer& ImmediateCommandBuffer::get() { return *m_commandBuffer; }

ImmediateCommandBuffer::operator CommandBuffer&() { return get(); }

}  // namespace sl
