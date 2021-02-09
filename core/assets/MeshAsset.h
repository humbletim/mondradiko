// Copyright (c) 2020-2021 the Mondradiko contributors.
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <array>
#include <cstddef>

#include "core/assets/AssetPool.h"
#include "core/gpu/GpuPipeline.h"
#include "lib/include/glm_headers.h"
#include "lib/include/vulkan_headers.h"

#include "c++20-polyfills.h"

namespace mondradiko {

// Forward declarations
class GpuInstance;
class GpuBuffer;

struct MeshVertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec3 color;
  glm::vec2 tex_coord;

  static GpuPipeline::VertexBindings getVertexBindings() {
    GpuPipeline::VertexBindings bindings(1);

    bindings[0] = with(GpuPipeline::VertexBindings::value_type, $.binding = 0,
                   $.stride = sizeof(MeshVertex),
                   $.inputRate = VK_VERTEX_INPUT_RATE_VERTEX);

    return bindings;
  }

  static GpuPipeline::AttributeDescriptions getAttributeDescriptions() {
    GpuPipeline::AttributeDescriptions descriptions(4);

    descriptions[0] = with(GpuPipeline::AttributeDescriptions::value_type, $.location = 0,
                       $.binding = 0,
                       $.format = VK_FORMAT_R32G32B32_SFLOAT,
                       $.offset = offsetof(MeshVertex, position));

    descriptions[1] = with(GpuPipeline::AttributeDescriptions::value_type, $.location = 1,
                       $.binding = 0,
                       $.format = VK_FORMAT_R32G32B32_SFLOAT,
                       $.offset = offsetof(MeshVertex, normal));

    descriptions[2] = with(GpuPipeline::AttributeDescriptions::value_type, $.location = 2,
                       $.binding = 0,
                       $.format = VK_FORMAT_R32G32B32_SFLOAT,
                       $.offset = offsetof(MeshVertex, color));

    descriptions[3] = with(GpuPipeline::AttributeDescriptions::value_type, $.location = 3,
                       $.binding = 0,
                       $.format = VK_FORMAT_R32G32_SFLOAT,
                       $.offset = offsetof(MeshVertex, tex_coord));

    return descriptions;
  }
};

using MeshIndex = uint32_t;

class MeshAsset : public Asset {
 public:
  DECL_ASSET_TYPE(assets::AssetType::MeshAsset);

  // Asset lifetime implementation
  explicit MeshAsset(GpuInstance* gpu) : gpu(gpu) {}
  void load(const assets::SerializedAsset*) final;
  ~MeshAsset();

  GpuBuffer* vertex_buffer = nullptr;
  GpuBuffer* index_buffer = nullptr;
  size_t index_count = 0;

 private:
  GpuInstance* gpu;
};

}  // namespace mondradiko
