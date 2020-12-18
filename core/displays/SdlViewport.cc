/**
 * @file SdlViewport.cc
 * @author Marceline Cramer (cramermarceline@gmail.com)
 * @brief Implements access to an SDL window swapchain.
 * @date 2020-11-10
 *
 * @copyright Copyright (c) 2020 Marceline Cramer
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 */

#include "core/displays/SdlViewport.h"

#include "core/displays/SdlDisplay.h"
#include "core/gpu/GpuInstance.h"
#include "core/renderer/Renderer.h"
#include "log/log.h"

namespace mondradiko {

SdlViewport::SdlViewport(GpuInstance* gpu, SdlDisplay* display,
                         Renderer* renderer)
    : gpu(gpu), display(display), renderer(renderer) {
  log_zone;

  int window_width;
  int window_height;
  SDL_Vulkan_GetDrawableSize(display->window, &window_width, &window_height);

  // TODO(marceline-cramer) Better queue sharing
  VkSwapchainCreateInfoKHR swapchain_info{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = display->surface,
      .minImageCount = display->surface_capabilities.minImageCount + 2,
      .imageFormat = display->swapchain_format,
      .imageColorSpace = display->swapchain_color_space,
      .imageExtent = {.width = static_cast<uint32_t>(window_width),
                      .height = static_cast<uint32_t>(window_height)},
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .preTransform = display->surface_capabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = display->swapchain_present_mode,
      .clipped = VK_TRUE,
      .oldSwapchain = VK_NULL_HANDLE};

  if (vkCreateSwapchainKHR(gpu->device, &swapchain_info, nullptr, &swapchain) !=
      VK_SUCCESS) {
    log_ftl("Failed to create swapchain.");
  }

  image_width = window_width;
  image_height = window_height;

  uint32_t image_count;
  vkGetSwapchainImagesKHR(gpu->device, swapchain, &image_count, nullptr);
  std::vector<VkImage> swapchain_images(image_count);
  vkGetSwapchainImagesKHR(gpu->device, swapchain, &image_count,
                          swapchain_images.data());

  images.resize(image_count);

  for (uint32_t i = 0; i < image_count; i++) {
    images[i].image = swapchain_images[i];

    VkImageViewCreateInfo view_info{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = images[i].image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = display->getSwapchainFormat(),
        .components = {.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                       .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                       .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                       .a = VK_COMPONENT_SWIZZLE_IDENTITY},
        .subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                             .baseMipLevel = 0,
                             .levelCount = 1,
                             .baseArrayLayer = 0,
                             .layerCount = 1}};

    if (vkCreateImageView(gpu->device, &view_info, nullptr,
                          &images[i].image_view) != VK_SUCCESS) {
      log_ftl("Failed to create swapchain image view.");
    }

    VkFramebufferCreateInfo framebufferCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = renderer->composite_pass,
        .attachmentCount = 1,
        .pAttachments = &images[i].image_view,
        .width = image_width,
        .height = image_height,
        .layers = 1};

    if (vkCreateFramebuffer(gpu->device, &framebufferCreateInfo, nullptr,
                            &images[i].framebuffer) != VK_SUCCESS) {
      log_ftl("Failed to create framebuffer.");
    }
  }

  acquire_image_index = 0;
  on_image_acquire.resize(10);

  for (uint32_t i = 0; i < on_image_acquire.size(); i++) {
    VkSemaphoreCreateInfo semaphore_info{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

    if (vkCreateSemaphore(gpu->device, &semaphore_info, nullptr,
                          &on_image_acquire[i]) != VK_SUCCESS) {
      log_ftl("Failed to create image acquisition semaphore.");
    }
  }
}

SdlViewport::~SdlViewport() {
  log_zone;

  vkDeviceWaitIdle(gpu->device);

  for (auto semaphore : on_image_acquire) {
    vkDestroySemaphore(gpu->device, semaphore, nullptr);
  }

  for (ViewportImage& image : images) {
    vkDestroyImageView(gpu->device, image.image_view, nullptr);
    vkDestroyFramebuffer(gpu->device, image.framebuffer, nullptr);
  }

  if (swapchain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(gpu->device, swapchain, nullptr);
  }
}

VkSemaphore SdlViewport::acquire() {
  log_zone;

  acquire_image_index++;
  if (acquire_image_index >= on_image_acquire.size()) {
    acquire_image_index = 0;
  }

  vkAcquireNextImageKHR(gpu->device, swapchain, UINT64_MAX,
                        on_image_acquire[acquire_image_index], VK_NULL_HANDLE,
                        &current_image_index);

  return on_image_acquire[acquire_image_index];
}

void SdlViewport::beginRenderPass(VkCommandBuffer command_buffer,
                                  VkRenderPass render_pass) {
  log_zone;

  std::array<VkClearValue, 1> clear_values;

  clear_values[0].color = {0.2, 0.0, 0.0, 1.0};

  VkRenderPassBeginInfo renderPassInfo{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = render_pass,
      .framebuffer = images[current_image_index].framebuffer,
      .renderArea = {.offset = {0, 0}, .extent = {image_width, image_height}},
      .clearValueCount = static_cast<uint32_t>(clear_values.size()),
      .pClearValues = clear_values.data()};

  vkCmdBeginRenderPass(command_buffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  VkViewport viewport{.x = 0,
                      .y = 0,
                      .width = static_cast<float>(image_width),
                      .height = static_cast<float>(image_height),
                      .minDepth = 0.0f,
                      .maxDepth = 1.0f};

  VkRect2D scissor{.offset = {0, 0}, .extent = {image_width, image_height}};

  vkCmdSetViewport(command_buffer, 0, 1, &viewport);
  vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

void SdlViewport::writeUniform(ViewportUniform* uniform) {
  log_zone;

  // TODO(marceline-cramer) Add WASD+Mouse control
  uniform->view =
      glm::lookAt(glm::vec3(2.0, 2.0, 2.0), glm::vec3(0.0, 0.0, 0.0),
                  glm::vec3(0.0, 1.0, 0.0));
  uniform->projection = glm::perspective(
      glm::radians(90.0f),
      static_cast<float>(image_width) / static_cast<float>(image_height), 0.01f,
      1000.0f);

  // Fix GLM matrix to work with Vulkan
  uniform->projection[1][1] *= -1.0;
}

void SdlViewport::release(VkSemaphore on_render_finished) {
  log_zone;

  VkPresentInfoKHR present_info{.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                                .waitSemaphoreCount = 1,
                                .pWaitSemaphores = &on_render_finished,
                                .swapchainCount = 1,
                                .pSwapchains = &swapchain,
                                .pImageIndices = &current_image_index};

  // TODO(marceline-cramer) Better queue management
  vkQueuePresentKHR(gpu->graphics_queue, &present_info);
}

}  // namespace mondradiko