/**
 * @file OverlayPass.cc
 * @author Marceline Cramer (cramermarceline@gmail.com)
 * @brief Render pass for UI, text, and debug draw.
 * @date 2020-12-19
 *
 * @copyright Copyright (c) 2020 Marceline Cramer
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 */

#include "core/renderer/OverlayPass.h"

#include <vector>

#include "core/gpu/GpuDescriptorSet.h"
#include "core/gpu/GpuDescriptorSetLayout.h"
#include "core/gpu/GpuInstance.h"
#include "core/gpu/GpuShader.h"
#include "core/gpu/GpuVector.h"
#include "log/log.h"
#include "shaders/debug.frag.h"
#include "shaders/debug.vert.h"

namespace mondradiko {

OverlayPass::OverlayPass(GpuInstance* gpu,
                         GpuDescriptorSetLayout* viewport_layout,
                         VkRenderPass parent_pass, uint32_t subpass_index)
    : gpu(gpu) {
  log_zone;

  {
    log_zone_named("Create pipeline layouts");

    std::vector<VkDescriptorSetLayout> set_layouts{
        viewport_layout->getSetLayout(),
    };

    VkPipelineLayoutCreateInfo layoutInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<uint32_t>(set_layouts.size()),
        .pSetLayouts = set_layouts.data()};

    if (vkCreatePipelineLayout(gpu->device, &layoutInfo, nullptr,
                               &debug_pipeline_layout) != VK_SUCCESS) {
      log_ftl("Failed to create pipeline layout.");
    }
  }

  {
    log_zone_named("Create pipelines");

    GpuShader vert_shader(gpu, VK_SHADER_STAGE_VERTEX_BIT, shaders_debug_vert,
                          sizeof(shaders_debug_vert));
    GpuShader frag_shader(gpu, VK_SHADER_STAGE_FRAGMENT_BIT, shaders_debug_frag,
                          sizeof(shaders_debug_frag));

    std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {
        vert_shader.getStageCreateInfo(), frag_shader.getStageCreateInfo()};

    auto binding_description = DebugDrawVertex::getBindingDescription();
    auto attribute_descriptions = DebugDrawVertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertex_input_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &binding_description,
        .vertexAttributeDescriptionCount = attribute_descriptions.size(),
        .pVertexAttributeDescriptions = attribute_descriptions.data()};

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
        .primitiveRestartEnable = VK_FALSE};

    // TODO(marceline-cramer) Get viewport state from Viewport
    VkViewport viewport{.x = 0.0f,
                        .y = 0.0f,
                        .width = static_cast<float>(500),
                        .height = static_cast<float>(500),
                        .minDepth = 0.0f,
                        .maxDepth = 1.0f};

    VkRect2D scissor{.offset = {0, 0}, .extent = {500, 500}};

    VkPipelineViewportStateCreateInfo viewport_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor};

    VkPipelineRasterizationStateCreateInfo rasterization_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f};

    VkPipelineMultisampleStateCreateInfo multisample_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE};

    VkPipelineDepthStencilStateCreateInfo depth_stencil_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE};

    VkPipelineColorBlendAttachmentState color_blend_attachment{
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

    VkPipelineColorBlendStateCreateInfo color_blend_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment};

    std::vector<VkDynamicState> dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT,
                                                  VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamic_state_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<uint32_t>(dynamic_states.size()),
        .pDynamicStates = dynamic_states.data()};

    VkGraphicsPipelineCreateInfo pipeline_info{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = static_cast<uint32_t>(shader_stages.size()),
        .pStages = shader_stages.data(),
        .pVertexInputState = &vertex_input_info,
        .pInputAssemblyState = &input_assembly_info,
        .pViewportState = &viewport_info,
        .pRasterizationState = &rasterization_info,
        .pMultisampleState = &multisample_info,
        .pDepthStencilState = &depth_stencil_info,
        .pColorBlendState = &color_blend_info,
        .pDynamicState = &dynamic_state_info,
        .layout = debug_pipeline_layout,
        .renderPass = parent_pass,
        .subpass = subpass_index,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1};

    if (vkCreateGraphicsPipelines(gpu->device, VK_NULL_HANDLE, 1,
                                  &pipeline_info, nullptr,
                                  &debug_pipeline) != VK_SUCCESS) {
      log_ftl("Failed to create pipeline.");
    }
  }
}

OverlayPass::~OverlayPass() {
  log_zone;

  if (debug_pipeline != VK_NULL_HANDLE)
    vkDestroyPipeline(gpu->device, debug_pipeline, nullptr);
  if (debug_pipeline_layout != VK_NULL_HANDLE)
    vkDestroyPipelineLayout(gpu->device, debug_pipeline_layout, nullptr);
}

void OverlayPass::createFrameData(OverlayPassFrameData& frame) {
  log_zone;

  frame.debug_vertices = new GpuVector(gpu, sizeof(DebugDrawVertex),
                                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  frame.debug_indices = new GpuVector(gpu, sizeof(DebugDrawIndex),
                                      VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}

void OverlayPass::destroyFrameData(OverlayPassFrameData& frame) {
  log_zone;

  if (frame.debug_vertices != nullptr) delete frame.debug_vertices;
  if (frame.debug_indices != nullptr) delete frame.debug_indices;
}

void OverlayPass::allocateDescriptors(entt::registry& registry,
                                      OverlayPassFrameData& frame,
                                      const AssetPool* asset_pool,
                                      GpuDescriptorPool* descriptor_pool) {
  log_zone;

  frame.index_count = 0;
}

void OverlayPass::render(entt::registry& registry, OverlayPassFrameData& frame,
                         const AssetPool* asset_pool,
                         VkCommandBuffer command_buffer,
                         GpuDescriptorSet* viewport_descriptor,
                         uint32_t viewport_offset) {
  log_zone;

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    debug_pipeline);

  // TODO(marceline-cramer) GpuPipeline + GpuPipelineLayout
  viewport_descriptor->updateDynamicOffset(0, viewport_offset);
  viewport_descriptor->cmdBind(command_buffer, debug_pipeline_layout, 0);

  VkBuffer vertex_buffers[] = {frame.debug_vertices->getBuffer()};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);
  vkCmdBindIndexBuffer(command_buffer, frame.debug_indices->getBuffer(), 0,
                       VK_INDEX_TYPE_UINT16);
  vkCmdDrawIndexed(command_buffer, frame.index_count, 1, 0, 0, 0);
}

}  // namespace mondradiko