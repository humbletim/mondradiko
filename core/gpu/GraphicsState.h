// Copyright (c) 2020-2021 the Mondradiko contributors.
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <cstdint>

namespace mondradiko {

struct GraphicsState {
  using FlagData = uint8_t;
  enum class BoolFlag : FlagData { False = (1 << 0), True = (1 << 1) };

  enum class CompareOp : FlagData {
    Never = (1 << 0),
    Less = (1 << 1),
    Equal = (1 << 2),
    LessOrEqual = (1 << 3),
    Greater = (1 << 4),
    NotEqual = (1 << 5),
    GreaterOrEqual = (1 << 6),
    Always = (1 << 7)
  };

  enum class PrimitiveTopology : FlagData {
    PointList = (1 << 0),
    LineList = (1 << 1),
    LineStrip = (1 << 2),
    TriangleList = (1 << 3),
    TriangleStrip = (1 << 4),
    TriangleFan = (1 << 5)
  };

  enum class PolygonMode : FlagData {
    Fill = (1 << 0),
    Line = (1 << 1),
    Point = (1 << 2),
  };

  enum class CullMode : FlagData {
    None = (1 << 0),
    Front = (1 << 1),
    Back = (1 << 2),
    Both = (1 << 3)
  };

  enum class StencilOp : FlagData {
    Keep = (1 << 0),
    Zero = (1 << 1),
    Replace = (1 << 2),
    IncrementAndClamp = (1 << 3),
    DecrementAndClamp = (1 << 4),
    Invert = (1 << 5),
    IncrementAndWrap = (1 << 6),
    DecrementAndWrap = (1 << 7)
  };

  struct InputAssemblyState {
    PrimitiveTopology primitive_topology;
    BoolFlag primitive_restart_enable;
  } input_assembly_state;

  struct RasterizatonState {
    PolygonMode polygon_mode;
    CullMode cull_mode;
  } rasterization_state;

  struct DepthState {
    BoolFlag test_enable;
    BoolFlag write_enable;
    CompareOp compare_op;
  } depth_state;
};

}  // namespace mondradiko
