#include "VulkanPipeline.hh"

#include <array>

#include "VulkanDevice.hh"
#include "VulkanRenderPassBackend.hh"
#include "VulkanShader.hh"
#include "VulkanCommandBuffer.hh"

namespace sl::vk {

static VkCullModeFlagBits toVk(CullMode mode) {
    switch (mode) {
        case CullMode::none:
            return VK_CULL_MODE_NONE;
        case CullMode::front:
            return VK_CULL_MODE_FRONT_BIT;
        case CullMode::frontAndBack:
            return VK_CULL_MODE_FRONT_AND_BACK;
        case CullMode::back:
        default:
            return VK_CULL_MODE_BACK_BIT;
    }
};

static VkPolygonMode toVk(PolygonMode mode) {
    switch (mode) {
        case PolygonMode::fill:
            return VK_POLYGON_MODE_FILL;
        case PolygonMode::line:
            return VK_POLYGON_MODE_LINE;
        case PolygonMode::point:
            return VK_POLYGON_MODE_POINT;
    }
    log::panic("Unespected polygon mode: {}", fmt::underlying(mode));
};

VulkanPipeline::VulkanPipeline(
  VulkanDevice& device, VulkanShader& shader, VulkanRenderPassBackend& renderPass,
  const Properties& props
) : m_device(device), m_layout(VK_NULL_HANDLE), m_handle(VK_NULL_HANDLE) {
    // ViewportState
    VkPipelineViewportStateCreateInfo viewportState;
    clearMemory(&viewportState);

    VkViewport viewport;
    viewport.x        = 0.0f;
    viewport.y        = static_cast<float>(props.viewport.size.w);
    viewport.width    = static_cast<float>(props.viewport.size.w);
    viewport.height   = -static_cast<float>(props.viewport.size.h);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x      = props.scissor.offset.x;
    scissor.offset.y      = props.scissor.offset.y;
    scissor.extent.width  = props.scissor.size.w;
    scissor.extent.height = props.scissor.size.h;

    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports    = &viewport;
    viewportState.scissorCount  = 1;
    viewportState.pScissors     = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizerCreateInfo;
    clearMemory(&rasterizerCreateInfo);
    rasterizerCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerCreateInfo.depthClampEnable        = false;
    rasterizerCreateInfo.rasterizerDiscardEnable = false;
    rasterizerCreateInfo.polygonMode             = toVk(props.polygonMode);
    rasterizerCreateInfo.lineWidth               = 1.0f;
    rasterizerCreateInfo.cullMode                = toVk(props.cullMode);
    rasterizerCreateInfo.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizerCreateInfo.depthBiasEnable         = false;
    rasterizerCreateInfo.depthBiasConstantFactor = 0.0f;
    rasterizerCreateInfo.depthBiasClamp          = 0.0f;
    rasterizerCreateInfo.depthBiasSlopeFactor    = 0.0f;

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisamplingCreateInfo;
    clearMemory(&multisamplingCreateInfo);
    multisamplingCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisamplingCreateInfo.sampleShadingEnable   = VK_FALSE;
    multisamplingCreateInfo.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
    multisamplingCreateInfo.minSampleShading      = 1.0f;
    multisamplingCreateInfo.pSampleMask           = 0;
    multisamplingCreateInfo.alphaToCoverageEnable = VK_FALSE;
    multisamplingCreateInfo.alphaToOneEnable      = VK_FALSE;

    // Depth and stencil testing
    VkPipelineDepthStencilStateCreateInfo dephStencilCreateInfo;
    clearMemory(&dephStencilCreateInfo);
    dephStencilCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    if (props.depthTestEnabled) {
        dephStencilCreateInfo.depthTestEnable       = VK_TRUE;
        dephStencilCreateInfo.depthWriteEnable      = VK_TRUE;
        dephStencilCreateInfo.depthCompareOp        = VK_COMPARE_OP_LESS;
        dephStencilCreateInfo.depthBoundsTestEnable = VK_FALSE;
        dephStencilCreateInfo.stencilTestEnable     = VK_FALSE;
    }

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState;
    clearMemory(&colorBlendAttachmentState);
    colorBlendAttachmentState.blendEnable         = VK_TRUE;
    colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentState.dstColorBlendFactor =
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.colorBlendOp        = VK_BLEND_OP_ADD;
    colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachmentState.dstAlphaBlendFactor =
      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

    colorBlendAttachmentState.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
      | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo;
    clearMemory(&colorBlendStateCreateInfo);
    colorBlendStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateCreateInfo.logicOpEnable   = VK_FALSE;
    colorBlendStateCreateInfo.logicOp         = VK_LOGIC_OP_COPY;
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments    = &colorBlendAttachmentState;

    // Dynamic state
    static constexpr u32 dynamicStateCount                      = 3;
    std::array<VkDynamicState, dynamicStateCount> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_LINE_WIDTH,
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;
    clearMemory(&dynamicStateCreateInfo);
    dynamicStateCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCreateInfo.dynamicStateCount = dynamicStates.size();
    dynamicStateCreateInfo.pDynamicStates    = dynamicStates.data();

    // Vertex input
    VkVertexInputBindingDescription bindingDescription;
    clearMemory(&bindingDescription);
    bindingDescription.binding   = 0;  // Binding index
    bindingDescription.stride    = shader.getInputAttributesStride();
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    // Attributes
    VkPipelineVertexInputStateCreateInfo vertexInputInfo;
    clearMemory(&vertexInputInfo);
    vertexInputInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions    = &bindingDescription;

    auto vertexAttributes = shader.getInputAttributesDescriptions();

    vertexInputInfo.vertexAttributeDescriptionCount = vertexAttributes.size();
    vertexInputInfo.pVertexAttributeDescriptions    = vertexAttributes.data();

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo;
    clearMemory(&inputAssemblyCreateInfo);
    inputAssemblyCreateInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    // VulkanPipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    clearMemory(&pipelineLayoutCreateInfo);
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    VkPushConstantRange pushConstantRange;
    clearMemory(&pushConstantRange);

    if (const auto pushConstantsSize = shader.getPushContantsSize();
        pushConstantsSize > 0) {
        pushConstantRange.stageFlags =
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0u;
        pushConstantRange.size   = pushConstantsSize;

        pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pipelineLayoutCreateInfo.pPushConstantRanges    = &pushConstantRange;
    }

    // Descriptor set layouts
    auto descriptorSetLayouts               = shader.getDescriptorSetLayouts();
    pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayouts.size();
    pipelineLayoutCreateInfo.pSetLayouts    = descriptorSetLayouts.data();

    // Create the pipeline layout.
    log::expect(vkCreatePipelineLayout(
      m_device.logical.handle, &pipelineLayoutCreateInfo, m_device.allocator,
      &m_layout
    ));
    log::trace("vkCreatePipelineLayout: {}", static_cast<void*>(m_layout));

    // VulkanPipeline create
    VkGraphicsPipelineCreateInfo pipelineCreateInfo;
    clearMemory(&pipelineCreateInfo);
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    auto stages = shader.getPipelineStageInfos();

    pipelineCreateInfo.stageCount          = stages.size();
    pipelineCreateInfo.pStages             = stages.data();
    pipelineCreateInfo.pVertexInputState   = &vertexInputInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;

    pipelineCreateInfo.pViewportState      = &viewportState;
    pipelineCreateInfo.pRasterizationState = &rasterizerCreateInfo;
    pipelineCreateInfo.pMultisampleState   = &multisamplingCreateInfo;
    pipelineCreateInfo.pDepthStencilState =
      props.depthTestEnabled ? &dephStencilCreateInfo : nullptr;
    pipelineCreateInfo.pColorBlendState   = &colorBlendStateCreateInfo;
    pipelineCreateInfo.pDynamicState      = &dynamicStateCreateInfo;
    pipelineCreateInfo.pTessellationState = 0;

    pipelineCreateInfo.layout = m_layout;

    pipelineCreateInfo.renderPass         = renderPass.getHandle();
    pipelineCreateInfo.subpass            = 0;
    pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineCreateInfo.basePipelineIndex  = -1;

    log::expect(vkCreateGraphicsPipelines(
      m_device.logical.handle, VK_NULL_HANDLE, 1, &pipelineCreateInfo,
      m_device.allocator, &m_handle
    ));
    log::trace("vkCreateGraphicsPipelines: {}", static_cast<void*>(m_handle));
}

VulkanPipeline::~VulkanPipeline() {
    m_device.waitIdle();

    if (m_handle) {
        log::trace("vkDestroyPipeline: {}", static_cast<void*>(m_handle));
        vkDestroyPipeline(m_device.logical.handle, m_handle, m_device.allocator);
    }

    if (m_layout) {
        log::trace("vkDestroyPipelineLayout: {}", static_cast<void*>(m_layout));
        vkDestroyPipelineLayout(
          m_device.logical.handle, m_layout, m_device.allocator
        );
    }
}

void VulkanPipeline::bind(CommandBuffer& commandBuffer) {
    vkCmdBindPipeline(
      static_cast<VulkanCommandBuffer&>(commandBuffer).getHandle(),
      VK_PIPELINE_BIND_POINT_GRAPHICS, m_handle
    );
}

VkPipelineLayout VulkanPipeline::getLayout() const { return m_layout; }

}  // namespace sl::vk
