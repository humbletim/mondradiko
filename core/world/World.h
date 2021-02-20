// Copyright (c) 2020-2021 the Mondradiko contributors.
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <unordered_map>

#include "core/assets/AssetPool.h"
#include "core/scripting/ScriptEnvironment.h"
#include "core/world/Entity.h"
#include "lib/include/flatbuffers_headers.h"

namespace mondradiko {

// Forward declarations
class Filesystem;
class GpuInstance;

namespace protocol {
struct WorldEvent;

struct SpawnEntity;
struct UpdateComponents;
struct UpdateScripts;
}  // namespace protocol

class World {
 public:
  World(Filesystem*, GpuInstance*);
  ~World();

  void initializePrefabs();

  //
  // World event callbacks
  //
  void onSpawnEntity(const protocol::SpawnEntity*);
  void onUpdateComponents(const protocol::UpdateComponents*);
  void onUpdateScripts(const protocol::UpdateScripts*);

  //
  // Helper methods
  //
  bool update(double);
  void processEvent(const protocol::WorldEvent*);

  template <class ComponentType, class ProtocolComponentType>
  void updateComponents(
      const flatbuffers::Vector<EntityId>*,
      const flatbuffers::Vector<const ProtocolComponentType*>*);

  Filesystem* fs;
  GpuInstance* gpu;

  // private:
  EntityRegistry registry;
  AssetPool asset_pool;
  ScriptEnvironment scripts;
};

}  // namespace mondradiko
