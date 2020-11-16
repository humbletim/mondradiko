/**
 * @file MeshShader.h
 * @author Marceline Cramer (cramermarceline@gmail.com)
 * @brief Contains pipeline state for the MeshPipeline.
 * @date 2020-10-24
 *
 * @copyright Copyright (c) 2020 Marceline Cramer
 *
 * This file is part of Mondradiko.
 *
 * Mondradiko is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Mondradiko is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Mondradiko.  If not, see <https: //www.gnu.org/licenses/>.
 *
 */

#ifndef SRC_RENDERER_MESHSHADER_H_
#define SRC_RENDERER_MESHSHADER_H_

#include "renderer/ShaderModule.h"

namespace mondradiko {

class MeshShader {
 public:
  explicit MeshShader(GpuInstance*);
  ~MeshShader();

  ShaderStages getStages();

 private:
  GpuInstance* gpu;

  ShaderModule vert_shader;
  ShaderModule frag_shader;
};

}  // namespace mondradiko

#endif  // SRC_RENDERER_MESHSHADER_H_