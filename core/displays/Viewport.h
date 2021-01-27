/**
 * @file Viewport.h
 * @author Marceline Cramer (cramermarceline@gmail.com)
 * @brief Base class for cross-API render targets.
 * @date 2020-11-08
 *
 * @copyright Copyright (c) 2020 the Mondradiko contributors.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 */

#pragma once

#include <vector>

#include "core/common/glm_headers.h"
#include "core/common/vulkan_headers.h"

namespace mondradiko {

// Forward declarations
class DisplayInterface;
class GpuImage;
class GpuInstance;
class Renderer;

struct ViewportUniform {
  glm::mat4 view;
  glm::mat4 projection;
  alignas(16) glm::vec3 position;
};

struct ViewportImage {
  /**
   * @brief Put a color attachment here to generate this image's framebuffer.
   *
   */
  VkImage image;

  /**
   * @brief Generated by Viewport base class for its framebuffers.
   *
   */
  VkImageView image_view;

  /**
   * @brief Generated by Viewport base class for use in render passes.
   *
   */
  VkFramebuffer framebuffer;
};

class Viewport {
 public:
  Viewport(DisplayInterface* display, GpuInstance* gpu, Renderer* renderer)
      : display(display), gpu(gpu), renderer(renderer) {}
  virtual ~Viewport() { _destroyImages(); }

  /**
   * @brief Begins a viewport's final render pass on a command buffer.
   * @param command_buffer Command buffer to record on.
   * @param render_pass Render pass to be begin.
   *
   * @note This method is implemented in the Viewport base class.
   */
  void beginRenderPass(VkCommandBuffer, VkRenderPass);

  /**
   * @brief Acquires a Viewport swapchain's image.
   * @return A semaphore that is signaled on image acquire, or VK_NULL_HANDLE.
   */
  VkSemaphore acquire() {
    // Redirect to virtual image acquisition method
    return _acquireImage(&_current_image_index);
  }

  /**
   * @brief Writes this viewport's uniform data.
   * @param uniform Uniform structure to write to.
   *
   */
  virtual void writeUniform(ViewportUniform*) = 0;

  /**
   * @brief Tests if a Viewport requires signaling for finished renders.
   * Useful for SDL, for example, which should only present on a queue when
   * the graphics queue is finished doing its work.
   * Unnecessary for OpenXR, where swapchain image synchronization is done
   * directly through acquisition of the graphics queue.
   *
   * @return true This Viewport requires a semaphore on render finished.
   * @return false This Viewport does not require signaling on render finished.
   */
  virtual bool isSignalRequired() = 0;

  /**
   * @brief Releases a swapchain image.
   * @param on_render_finished Semaphore that is signaled when GPU commands are
   * complete. Presumably ignored if isSignalRequired() returns false.
   *
   */
  void release(VkSemaphore on_render_finished) {
    // Redirect to virtual image acquisition method
    _releaseImage(_current_image_index, on_render_finished);
  }

 protected:
  /**
   * @brief Creates this Viewport's framebuffers; needed to begin render passes.
   *
   * @note Set _images, _image_width, and _image_height before
   * calling.
   */
  void _createImages();

  /**
   * @brief Destroys created framebuffers. Should be called before swapchain
   * destruction.
   *
   */
  void _destroyImages();

  /**
   * @brief Acquires a swapchain image and sets up synchronization.
   * @param image_index Image index to use.
   *
   * @return on_image_acquired Semaphore signaled when image is acquired,
   * or VK_NULL_HANDLE if no semaphore is signaled.
   */
  virtual VkSemaphore _acquireImage(uint32_t*) = 0;

  /**
   * @brief Releases a swapchain image. (implementation)
   * @param current_image_index Swapchain image to release.
   * @param on_render_finished Semaphore that is signaled when GPU commands are
   * complete. Presumably ignored if isSignalRequired() returns false.
   */
  virtual void _releaseImage(uint32_t, VkSemaphore) = 0;

  /**
   * @brief Contains this Viewport's images.
   * Resize, write .image members, and call
   * createImages() to create framebuffers.
   *
   */
  std::vector<ViewportImage> _images;
  /**
   * @brief Required to construct framebuffers.
   *
   */
  uint32_t _image_width;

  /**
   * @brief Required to construct framebuffers.
   *
   */
  uint32_t _image_height;

 private:
  DisplayInterface* display;
  GpuInstance* gpu;
  Renderer* renderer;

  GpuImage* _depth_image;
  uint32_t _current_image_index = 0;
};

}  // namespace mondradiko
