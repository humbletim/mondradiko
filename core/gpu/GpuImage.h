/**
 * @file GpuImage.h
 * @author Marceline Cramer (cramermarceline@gmail.com)
 * @brief Creates and manages a Vulkan image and its allocation.
 * @date 2020-10-24
 *
 * @copyright Copyright (c) 2020 the Mondradiko contributors.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 */

#pragma once

#include "lib/include/vulkan_headers.h"

namespace mondradiko {

// Forward declarations
class GpuBuffer;
class GpuInstance;

class GpuImage {
 public:
  GpuImage(GpuInstance*, VkFormat, uint32_t, uint32_t, VkImageUsageFlags,
           VmaMemoryUsage);
  ~GpuImage();

  void writeData(const void*);
  void transitionLayout(VkImageLayout);

  VkFormat format;
  VkImageLayout layout;
  uint32_t width;
  uint32_t height;
  VmaAllocation allocation = nullptr;
  VmaAllocationInfo allocation_info;
  VkImage image = VK_NULL_HANDLE;

  VkImageView view = VK_NULL_HANDLE;

 private:
  GpuInstance* gpu;

  void createView();
};

}  // namespace mondradiko
