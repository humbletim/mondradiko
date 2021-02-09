// Copyright (c) 2020-2021 the Mondradiko contributors.
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "core/displays/Viewport.h"

#include "core/displays/DisplayInterface.h"
#include "core/gpu/GpuImage.h"
#include "core/gpu/GpuInstance.h"
#include "core/renderer/Renderer.h"
#include "log/log.h"

#include "c++20-polyfills.h"

namespace mondradiko {

void Viewport::beginRenderPass(VkCommandBuffer command_buffer,
                               VkRenderPass render_pass) {
  log_zone;

  std::array<VkClearValue, 2> clear_values;

  clear_values[0].color = {0.2, 0.0, 0.0, 1.0};
  clear_values[1].depthStencil = {1.0f};

  auto renderPassInfo = with(VkRenderPassBeginInfo, 
      $.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      $.renderPass = render_pass,
      $.framebuffer = _images[_current_image_index].framebuffer,
      $.renderArea = with(VkRect2D, $.offset = {0, 0}, $.extent = {_image_width, _image_height}),
      $.clearValueCount = static_cast<uint32_t>(clear_values.size()),
      $.pClearValues = clear_values.data());

  vkCmdBeginRenderPass(command_buffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  auto viewport = with(VkViewport, $.x = 0,
                      $.y = 0,
                      $.width = static_cast<float>(_image_width),
                      $.height = static_cast<float>(_image_height),
                      $.minDepth = 0.0f,
                      $.maxDepth = 1.0f);

  auto scissor = with(VkRect2D, $.offset = {0, 0}, $.extent = {_image_width, _image_height});

  vkCmdSetViewport(command_buffer, 0, 1, &viewport);
  vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

void Viewport::_createImages() {
  _depth_image = new GpuImage(
      gpu, display->getDepthFormat(), _image_width, _image_height,
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

  for (uint32_t i = 0; i < _images.size(); i++) {
    auto view_info = with(VkImageViewCreateInfo, 
        $.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        $.image = _images[i].image,
        $.viewType = VK_IMAGE_VIEW_TYPE_2D,
        $.format = display->getSwapchainFormat(),
        $.components = with(VkComponentMapping, $.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                       $.g = VK_COMPONENT_SWIZZLE_IDENTITY,
                       $.b = VK_COMPONENT_SWIZZLE_IDENTITY,
                       $.a = VK_COMPONENT_SWIZZLE_IDENTITY),
        $.subresourceRange = with(VkImageSubresourceRange, $.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                             $.baseMipLevel = 0,
                             $.levelCount = 1,
                             $.baseArrayLayer = 0,
                             $.layerCount = 1));

    if (vkCreateImageView(gpu->device, &view_info, nullptr,
                          &_images[i].image_view) != VK_SUCCESS) {
      log_ftl("Failed to create swapchain image view.");
    }

    std::array<VkImageView, 2> framebuffer_attachments = {_images[i].image_view,
                                                          _depth_image->view};

    auto framebuffer_info = with(VkFramebufferCreateInfo, 
        $.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        $.renderPass = renderer->getCompositePass(),
        $.attachmentCount = framebuffer_attachments.size(),
        $.pAttachments = framebuffer_attachments.data(),
        $.width = _image_width,
        $.height = _image_height,
        $.layers = 1);

    if (vkCreateFramebuffer(gpu->device, &framebuffer_info, nullptr,
                            &_images[i].framebuffer) != VK_SUCCESS) {
      log_ftl("Failed to create framebuffer.");
    }
  }
}

void Viewport::_destroyImages() {
  for (ViewportImage& image : _images) {
    vkDestroyImageView(gpu->device, image.image_view, nullptr);
    vkDestroyFramebuffer(gpu->device, image.framebuffer, nullptr);
  }
  _images.resize(0);

  if (_depth_image != nullptr) delete _depth_image;
  _depth_image = nullptr;
}

}  // namespace mondradiko
