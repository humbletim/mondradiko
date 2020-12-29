/**
 * @file WorldEventSorter.cc
 * @author Marceline Cramer (cramermarceline@gmail.com)
 * @brief Determines which WorldEvents go to which clients.
 * @date 2020-12-23
 *
 * @copyright Copyright (c) 2020 the Mondradiko contributors.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 */

#include "core/world/WorldEventSorter.h"

#include <utility>
#include <vector>

#include "core/components/TransformComponent.h"
#include "core/world/World.h"
#include "protocol/WorldEvent_generated.h"

namespace mondradiko {

WorldEventSorter::WorldEventSorter(World* world) : world(world) {}

WorldEventSorter::~WorldEventSorter() {}

void WorldEventSorter::processEvent(
    std::unique_ptr<protocol::WorldEventT>& event) {
  global_events.push_back(std::move(event));
}

// Helper function to generate component update events
template <class ComponentType>
flatbuffers::Offset<protocol::WorldEvent> updateComponents(
    flatbuffers::FlatBufferBuilder& builder, EntityRegistry& registry) {
  static_assert(std::is_base_of<Component, ComponentType>(),
                "ComponentType must inherit from Component");

  using ProtocolComponentType = typename ComponentType::SerializedType;

  auto component_view = registry.view<ComponentType>();

  // Prepass to find dirty components
  std::vector<EntityId> entities_data;
  entities_data.reserve(component_view.size());

  for (auto& entity : component_view) {
    auto& component = component_view.get(entity);

    if (component.isDirty()) {
      entities_data.push_back(entity);
    }
  }

  // Allocate and copy dirty component data
  ProtocolComponentType* components_data;
  auto components_offset = builder.CreateUninitializedVectorOfStructs(
      entities_data.size(), &components_data);

  for (uint32_t i = 0; i < entities_data.size(); i++) {
    auto& component = component_view.get(entities_data[i]);
    components_data[i] = component.getData();
    // TODO(marceline-cramer) Keep track of client-relative dirtiness
    // component.markClean();
  }

  // Assemble outgoing event
  auto entities_offset = builder.CreateVector(entities_data);

  protocol::UpdateComponentsBuilder update_components(builder);
  update_components.add_entities(entities_offset);
  buildUpdateComponents(update_components, components_offset);
  auto update_components_offset = update_components.Finish();

  protocol::WorldEventBuilder world_event(builder);
  world_event.add_type(protocol::WorldEventType::UpdateComponents);
  world_event.add_update_components(update_components_offset);
  auto world_event_offset = world_event.Finish();

  return world_event_offset;
}

WorldEventSorter::WorldUpdateOffset WorldEventSorter::broadcastGlobalEvents(
    flatbuffers::FlatBufferBuilder& builder) const {
  std::vector<flatbuffers::Offset<protocol::WorldEvent>> update_offsets;

  for (auto& event : global_events) {
    auto event_offset = protocol::CreateWorldEvent(builder, event.get());
    update_offsets.push_back(event_offset);
  }

  std::vector<flatbuffers::Offset<protocol::WorldEvent>> component_updates = {
      updateComponents<TransformComponent>(builder, world->registry)};

  update_offsets.insert(update_offsets.end(), component_updates.begin(),
                        component_updates.end());

  return builder.CreateVector(update_offsets);
}

bool WorldEventSorter::isOutOfDate() { return global_events.size() > 0; }

void WorldEventSorter::clearQueue() { global_events.clear(); }

}  // namespace mondradiko
