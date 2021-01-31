// Copyright (c) 2020-2021 the Mondradiko contributors.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "core/gpu/GpuPipeline.h"

#include <xxhash.h>  // NOLINT

#include <array>
#include <cstring>

#include "core/gpu/GpuInstance.h"
#include "core/gpu/GpuShader.h"
#include "core/gpu/GraphicsState.h"
#include "log/log.h"

namespace mondradiko {

GpuPipeline::GpuPipeline(GpuInstance* gpu, VkPipelineLayout pipeline_layout,
                         VkRenderPass render_pass, uint32_t subpass_index,
                         GpuShader* vertex_shader, GpuShader* fragment_shader,
                         const VertexBindings& vertex_bindings,
                         const AttributeDescriptions& attribute_descriptions)
    : gpu(gpu),
      pipeline_layout(pipeline_layout),
      render_pass(render_pass),
      subpass_index(subpass_index),
      vertex_shader(vertex_shader),
      fragment_shader(fragment_shader),
      vertex_bindings(vertex_bindings),
      attribute_descriptions(attribute_descriptions) {}

GpuPipeline::~GpuPipeline() {
  log_zone;

  for (auto pipeline : pipelines) {
    vkDestroyPipeline(gpu->device, pipeline.second, nullptr);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Pipeline object helper functions
////////////////////////////////////////////////////////////////////////////////

#define HANDLE_INVALID_FLAGS(state, ret) \
  log_ftl("Invalid GraphicsState");      \
  return ret

VkBool32 createVkBool(GraphicsState::BoolFlag bool_flag) {
  switch (bool_flag) {
    case GraphicsState::BoolFlag::False:
      return VK_FALSE;
    case GraphicsState::BoolFlag::True:
      return VK_TRUE;
    default:
      HANDLE_INVALID_FLAGS(bool_flag, VK_FALSE);
  }
}

VkCompareOp createVkCompareOp(GraphicsState::CompareOp compare_op) {
  switch (compare_op) {
    case GraphicsState::CompareOp::Never:
      return VK_COMPARE_OP_NEVER;
    case GraphicsState::CompareOp::Less:
      return VK_COMPARE_OP_LESS;
    case GraphicsState::CompareOp::Equal:
      return VK_COMPARE_OP_EQUAL;
    case GraphicsState::CompareOp::LessOrEqual:
      return VK_COMPARE_OP_LESS_OR_EQUAL;
    case GraphicsState::CompareOp::Greater:
      return VK_COMPARE_OP_GREATER;
    case GraphicsState::CompareOp::NotEqual:
      return VK_COMPARE_OP_NOT_EQUAL;
    case GraphicsState::CompareOp::GreaterOrEqual:
      return VK_COMPARE_OP_GREATER_OR_EQUAL;
    case GraphicsState::CompareOp::Always:
      return VK_COMPARE_OP_ALWAYS;
    default:
      HANDLE_INVALID_FLAGS(compare_op, VK_COMPARE_OP_MAX_ENUM);
  }
}

VkPrimitiveTopology createVkPrimitiveTopology(
    GraphicsState::PrimitiveTopology primitive_topology) {
  switch (primitive_topology) {
    case GraphicsState::PrimitiveTopology::PointList:
      return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    case GraphicsState::PrimitiveTopology::LineList:
      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
    case GraphicsState::PrimitiveTopology::LineStrip:
      return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    case GraphicsState::PrimitiveTopology::TriangleList:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case GraphicsState::PrimitiveTopology::TriangleStrip:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    case GraphicsState::PrimitiveTopology::TriangleFan:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
    default:
      HANDLE_INVALID_FLAGS(primitive_topology, VK_PRIMITIVE_TOPOLOGY_MAX_ENUM);
  }
}

VkPolygonMode createVkPolygonMode(GraphicsState::PolygonMode polygon_mode) {
  switch (polygon_mode) {
    case GraphicsState::PolygonMode::Fill:
      return VK_POLYGON_MODE_FILL;
    case GraphicsState::PolygonMode::Line:
      return VK_POLYGON_MODE_LINE;
    case GraphicsState::PolygonMode::Point:
      return VK_POLYGON_MODE_POINT;
    default:
      HANDLE_INVALID_FLAGS(polygon_mode, VK_POLYGON_MODE_MAX_ENUM);
  }
}

VkCullModeFlags createVkCullMode(GraphicsState::CullMode cull_mode) {
  switch (cull_mode) {
    case GraphicsState::CullMode::None:
      return VK_CULL_MODE_NONE;
    case GraphicsState::CullMode::Front:
      return VK_CULL_MODE_FRONT_BIT;
    case GraphicsState::CullMode::Back:
      return VK_CULL_MODE_BACK_BIT;
    case GraphicsState::CullMode::Both:
      return VK_CULL_MODE_FRONT_AND_BACK;
    default:
      HANDLE_INVALID_FLAGS(cull_mode, VK_CULL_MODE_FLAG_BITS_MAX_ENUM);
  }
}

// VkStencilOp createVkStencilOp(GraphicsState::StencilOp stencil_op) {}

void createVkInputAssemblyState(const GraphicsState& graphics_state,
                                VkPipelineInputAssemblyStateCreateInfo* info) {
  const auto& state = graphics_state.input_assembly_state;

  info->sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  info->topology = createVkPrimitiveTopology(state.primitive_topology);
  info->primitiveRestartEnable = createVkBool(state.primitive_restart_enable);
}

void createVkRasterizationState(const GraphicsState& graphics_state,
                                VkPipelineRasterizationStateCreateInfo* info) {
  const auto& state = graphics_state.rasterization_state;

  info->sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  info->depthClampEnable = VK_FALSE;
  info->rasterizerDiscardEnable = VK_FALSE;
  info->polygonMode = createVkPolygonMode(state.polygon_mode);
  info->cullMode = createVkCullMode(state.cull_mode);
  info->frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  info->depthBiasEnable = VK_FALSE;
  info->lineWidth = 1.0f;
}

void createVkDepthStencilState(const GraphicsState& graphics_state,
                               VkPipelineDepthStencilStateCreateInfo* info) {
  const auto& depth_state = graphics_state.depth_state;

  info->sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  info->depthTestEnable = createVkBool(depth_state.test_enable);
  info->depthWriteEnable = createVkBool(depth_state.write_enable);
  info->depthCompareOp = createVkCompareOp(depth_state.compare_op);
  info->depthBoundsTestEnable = VK_FALSE;
  info->stencilTestEnable = VK_FALSE;
}

////////////////////////////////////////////////////////////////////////////////
// Actual pipeline creation
////////////////////////////////////////////////////////////////////////////////
GpuPipeline::StateHash GpuPipeline::createPipeline(
    const GraphicsState& graphics_state) {
  log_zone;

  auto state_hash = getStateHash(graphics_state);

  auto iter = pipelines.find(state_hash);
  if (iter != pipelines.end()) return state_hash;

  log_inf("Creating pipeline: 0x%0lx", state_hash);

  VkPipeline pipeline_object;

  std::vector<VkPipelineShaderStageCreateInfo> shader_stages = {
      vertex_shader->getStageCreateInfo(),
      fragment_shader->getStageCreateInfo()};

  log_inf("%d %d", vertex_bindings.size(), attribute_descriptions.size());

  VkPipelineVertexInputStateCreateInfo vertex_input_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .vertexBindingDescriptionCount =
          static_cast<uint32_t>(vertex_bindings.size()),
      .pVertexBindingDescriptions = vertex_bindings.data(),
      .vertexAttributeDescriptionCount =
          static_cast<uint32_t>(attribute_descriptions.size()),
      .pVertexAttributeDescriptions = attribute_descriptions.data()};

  VkPipelineInputAssemblyStateCreateInfo input_assembly_info{};
  createVkInputAssemblyState(graphics_state, &input_assembly_info);

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

  VkPipelineRasterizationStateCreateInfo rasterization_info{};
  createVkRasterizationState(graphics_state, &rasterization_info);

  VkPipelineMultisampleStateCreateInfo multisample_info{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable = VK_FALSE,
      .minSampleShading = 1.0f,
      .pSampleMask = nullptr,
      .alphaToCoverageEnable = VK_FALSE,
      .alphaToOneEnable = VK_FALSE};

  VkPipelineDepthStencilStateCreateInfo depth_stencil_info{};
  createVkDepthStencilState(graphics_state, &depth_stencil_info);

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
      .layout = pipeline_layout,
      .renderPass = render_pass,
      .subpass = subpass_index,
      .basePipelineHandle = VK_NULL_HANDLE,
      .basePipelineIndex = -1};

  if (vkCreateGraphicsPipelines(gpu->device, VK_NULL_HANDLE, 1, &pipeline_info,
                                nullptr, &pipeline_object) != VK_SUCCESS) {
    log_ftl("Failed to create pipeline 0x%0lx", state_hash);
  }

  pipelines.emplace(state_hash, pipeline_object);
  return state_hash;
}

GpuPipeline::StateHash GpuPipeline::getStateHash(
    const GraphicsState& graphics_state) {
  log_zone;
  return XXH64(&graphics_state, sizeof(GraphicsState), 0);
}

void GpuPipeline::cmdBind(VkCommandBuffer command_buffer,
                          StateHash state_hash) {
  auto iter = pipelines.find(state_hash);
  if (iter == pipelines.end()) {
    log_ftl("Pipeline 0x%0lx not found", state_hash);
  }

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    iter->second);
}

}  // namespace mondradiko