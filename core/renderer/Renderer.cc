// Copyright (c) 2020-2021 the Mondradiko contributors.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "core/renderer/Renderer.h"

#include "core/cvars/CVarScope.h"
#include "core/displays/DisplayInterface.h"
#include "core/displays/Viewport.h"
#include "core/gpu/GpuDescriptorPool.h"
#include "core/gpu/GpuDescriptorSet.h"
#include "core/gpu/GpuDescriptorSetLayout.h"
#include "core/gpu/GpuInstance.h"
#include "core/gpu/GpuVector.h"
#include "core/renderer/OverlayPass.h"
#include "core/renderer/RenderPass.h"
#include "log/log.h"

#include "c++20-polyfills.h"

namespace mondradiko {

void Renderer::initCVars(CVarScope* cvars) {
  CVarScope* renderer = cvars->addChild("renderer");

  OverlayPass::initCVars(renderer);
}

Renderer::Renderer(const CVarScope* cvars, DisplayInterface* display,
                   GpuInstance* gpu)
    : cvars(cvars->getChild("renderer")), display(display), gpu(gpu) {
  log_zone;

  {
    log_zone_named("Create render pass");

    auto swapchain_attachment_description = with(VkAttachmentDescription, 
        $.format = display->getSwapchainFormat(),
        $.samples = VK_SAMPLE_COUNT_1_BIT,
        $.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        $.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        $.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        $.finalLayout = display->getFinalLayout());

    auto depth_attachment_description = with(VkAttachmentDescription, 
        $.format = display->getDepthFormat(),
        $.samples = VK_SAMPLE_COUNT_1_BIT,
        $.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        $.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        $.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        $.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    auto swapchain_attachment_reference = with(VkAttachmentReference, 
        $.attachment = 0, $.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    auto depth_attachment_reference = with(VkAttachmentReference, 
        $.attachment = 1,
        $.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    std::vector<VkAttachmentDescription> attachments = {
        swapchain_attachment_description, depth_attachment_description};

    auto composite_subpass_description = with(VkSubpassDescription, 
        $.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        $.colorAttachmentCount = 1,
        $.pColorAttachments = &swapchain_attachment_reference,
        $.pDepthStencilAttachment = &depth_attachment_reference);

    auto swapchain_dependency = with(VkSubpassDependency,
        $.srcSubpass = VK_SUBPASS_EXTERNAL,
        $.dstSubpass = 0,
        $.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        $.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        $.srcAccessMask = 0,
        $.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        $.dependencyFlags = 0);

    auto compositePassCreateInfo = with(VkRenderPassCreateInfo, 
        $.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        $.attachmentCount = (uint32_t)attachments.size(),
        $.pAttachments = attachments.data(),
        $.subpassCount = 1,
        $.pSubpasses = &composite_subpass_description,
        $.dependencyCount = 1,
        $.pDependencies = &swapchain_dependency);

    if (vkCreateRenderPass(gpu->device, &compositePassCreateInfo, nullptr,
                           &composite_pass) != VK_SUCCESS) {
      log_ftl("Failed to create Renderer composite render pass.");
    }
  }

  {
    log_zone_named("Create descriptor set layout");

    viewport_layout = new GpuDescriptorSetLayout(gpu);
    viewport_layout->addDynamicUniformBuffer(sizeof(ViewportUniform));
  }

  {
    log_zone_named("Create frame data");

    // Pipeline two frames
    frames_in_flight.resize(2);
    current_frame = 0;

    for (auto& frame : frames_in_flight) {
      auto alloc_info = with(VkCommandBufferAllocateInfo, 
          $.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
          $.commandPool = gpu->command_pool,
          $.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
          $.commandBufferCount = 1);

      if (vkAllocateCommandBuffers(gpu->device, &alloc_info,
                                   &frame.command_buffer) != VK_SUCCESS) {
        log_ftl("Failed to allocate frame command buffers.");
      }

      auto semaphore_info = with(VkSemaphoreCreateInfo, 
          $.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);

      if (vkCreateSemaphore(gpu->device, &semaphore_info, nullptr,
                            &frame.on_render_finished) != VK_SUCCESS) {
        log_ftl("Failed to create frame render finished semaphore.");
      }

      auto fenceInfo = with(VkFenceCreateInfo, $.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                                  $.flags = VK_FENCE_CREATE_SIGNALED_BIT);

      if (vkCreateFence(gpu->device, &fenceInfo, nullptr, &frame.is_in_use) !=
          VK_SUCCESS) {
        log_ftl("Failed to create frame fence.");
      }

      frame.descriptor_pool = new GpuDescriptorPool(gpu);

      frame.viewports = new GpuVector(
          // TODO(marceline-cramer) Better descriptor management
          gpu, sizeof(ViewportUniform), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
    }
  }
}

Renderer::~Renderer() {
  log_zone;

  destroyFrameData();

  if (viewport_layout != nullptr) delete viewport_layout;

  if (composite_pass != VK_NULL_HANDLE)
    vkDestroyRenderPass(gpu->device, composite_pass, nullptr);
}

void Renderer::addRenderPass(RenderPass* render_pass) {
  log_zone;

  render_pass->createFrameData(frames_in_flight.size());
  render_passes.push_back(render_pass);
}

void Renderer::destroyFrameData() {
  log_zone;

  vkDeviceWaitIdle(gpu->device);

  for (auto& render_pass : render_passes) {
    render_pass->destroyFrameData();
  }

  for (auto& frame : frames_in_flight) {
    if (frame.viewports != nullptr) delete frame.viewports;
    if (frame.on_render_finished != VK_NULL_HANDLE)
      vkDestroySemaphore(gpu->device, frame.on_render_finished, nullptr);
    if (frame.is_in_use != VK_NULL_HANDLE)
      vkDestroyFence(gpu->device, frame.is_in_use, nullptr);
    if (frame.descriptor_pool != nullptr) delete frame.descriptor_pool;
  }

  render_passes.clear();
  frames_in_flight.clear();
}

void Renderer::renderFrame() {
  log_zone;

  current_frame++;
  if (current_frame >= frames_in_flight.size()) {
    current_frame = 0;
  }

  auto& frame = frames_in_flight[current_frame];

  {
    log_zone_named("Fence frame");

    vkWaitForFences(gpu->device, 1, &frame.is_in_use, VK_TRUE, UINT64_MAX);
  }

  {
    log_zone_named("Clean up last frame");

    frame.descriptor_pool->reset();
  }

  std::vector<Viewport*> viewports;
  std::vector<VkSemaphore> on_viewport_acquire(0);
  bool viewports_require_signal = false;

  {
    log_zone_named("Acquire viewports");

    display->acquireViewports(&viewports);

    for (uint32_t viewport_index = 0; viewport_index < viewports.size();
         viewport_index++) {
      VkSemaphore on_image_acquire = viewports[viewport_index]->acquire();
      if (on_image_acquire != VK_NULL_HANDLE) {
        on_viewport_acquire.push_back(on_image_acquire);
      }

      if (viewports[viewport_index]->isSignalRequired()) {
        viewports_require_signal = true;
      }
    }
  }

  GpuDescriptorSet* viewport_descriptor;

  {
    log_zone_named("Allocate descriptors");

    viewport_descriptor = frame.descriptor_pool->allocate(viewport_layout);
    viewport_descriptor->updateDynamicBuffer(0, frame.viewports);

    for (auto& render_pass : render_passes) {
      render_pass->allocateDescriptors(current_frame, frame.descriptor_pool);
    }
  }

  {
    log_zone_named("Begin command buffers");

    auto beginInfo = with(VkCommandBufferBeginInfo, 
        $.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        $.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    vkBeginCommandBuffer(frame.command_buffer, &beginInfo);
  }

  {
    log_zone_named("Run pre-render commands");

    for (auto& render_pass : render_passes) {
      render_pass->preRender(current_frame, frame.command_buffer);
    }
  }

  {
    log_zone_named("Render frame");

    for (uint32_t viewport_index = 0; viewport_index < viewports.size();
         viewport_index++) {
      viewport_descriptor->updateDynamicOffset(
          0, viewport_index * sizeof(ViewportUniform));
      viewports[viewport_index]->beginRenderPass(frame.command_buffer,
                                                 composite_pass);

      for (auto& render_pass : render_passes) {
        render_pass->render(current_frame, frame.command_buffer,
                            viewport_descriptor);
      }

      vkCmdEndRenderPass(frame.command_buffer);
    }

    vkEndCommandBuffer(frame.command_buffer);
  }

  {
    log_zone_named("Write viewport uniforms");

    for (uint32_t i = 0; i < viewports.size(); i++) {
      ViewportUniform uniform;
      viewports[i]->writeUniform(&uniform);
      frame.viewports->writeElement(i, uniform);
    }
  }

  {
    log_zone_named("Submit to queue");

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    submitInfo.waitSemaphoreCount = on_viewport_acquire.size();
    submitInfo.pWaitSemaphores = on_viewport_acquire.data();

    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &frame.command_buffer;

    if (viewports_require_signal) {
      submitInfo.signalSemaphoreCount = 1;
      submitInfo.pSignalSemaphores = &frame.on_render_finished;
    } else {
      submitInfo.signalSemaphoreCount = 0;
      submitInfo.pSignalSemaphores = nullptr;
    }

    vkResetFences(gpu->device, 1, &frame.is_in_use);

    if (vkQueueSubmit(gpu->graphics_queue, 1, &submitInfo, frame.is_in_use) !=
        VK_SUCCESS) {
      log_ftl("Failed to submit primary frame command buffer.");
    }
  }

  {
    log_zone_named("Release viewports");

    for (uint32_t viewportIndex = 0; viewportIndex < viewports.size();
         viewportIndex++) {
      viewports[viewportIndex]->release(frame.on_render_finished);
    }
  }
}

}  // namespace mondradiko
