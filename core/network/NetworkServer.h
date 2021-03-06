// Copyright (c) 2020-2021 the Mondradiko contributors.
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/network/NetworkShared.h"
#include "core/world/World.h"
#include "lib/include/flatbuffers_headers.h"
#include "lib/include/gamenetworkingsockets_include.h"

namespace mondradiko {

// Forward declarations
class Filesystem;
class WorldEventSorter;

namespace protocol {
struct JoinRequest;
}  // namespace protocol

class NetworkServer {
 public:
  NetworkServer(Filesystem*, WorldEventSorter*, const char*, int);
  ~NetworkServer();

  void sendTestEvent(EntityId);

  void updateWorld();
  void update();

 private:
  Filesystem* fs;
  WorldEventSorter* world_event_sorter;

  ISteamNetworkingSockets* sockets;

  std::unordered_map<ClientId, HSteamNetConnection> connections;

  //
  // Client event receive methods
  //
  void onJoinRequest(ClientId, const protocol::JoinRequest*);

  //
  // Server event send methods
  //
  void sendAnnouncement(std::string);
  void setClientId(ClientId);

  //
  // Connection status change callbacks
  //
  void onConnecting(std::string, HSteamNetConnection);
  void onConnected(std::string, HSteamNetConnection);
  void onProblemDetected(std::string, HSteamNetConnection);
  void onClosedByPeer(std::string, HSteamNetConnection);
  void onDisconnect(std::string, HSteamNetConnection);

  //
  // Helper methods
  //
  void receiveEvents();
  void sendWorldUpdates();
  void sendEvent(flatbuffers::FlatBufferBuilder&, ClientId);
  void sendQueuedEvents();
  static void callback_ConnectionStatusChanged(
      SteamNetConnectionStatusChangedCallback_t*);

  HSteamListenSocket listen_socket = k_HSteamListenSocket_Invalid;

  HSteamNetPollGroup poll_group = k_HSteamNetPollGroup_Invalid;

  struct QueuedEvent {
    ClientId destination;
    std::vector<uint8_t> data;
  };

  std::deque<QueuedEvent> event_queue;
};

}  // namespace mondradiko
