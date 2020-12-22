/**
 * @file NetworkServer.h
 * @author Marceline Cramer (cramermarceline@gmail.com)
 * @brief Hosts a port and communicates to connecting clients with scene events
 * and server metadata.
 * @date 2020-12-06
 *
 * @copyright Copyright (c) 2020 Marceline Cramer
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 */

#pragma once

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/network/NetworkShared.h"
#include "flatbuffers/flatbuffers.h"
#include "steam/isteamnetworkingsockets.h"

namespace mondradiko {

// Forward declarations
class Scene;

namespace protocol {
struct JoinRequest;
};

class NetworkServer {
 public:
  NetworkServer(Scene*, const char*, int);
  ~NetworkServer();

  void update();

 private:
  Scene* scene;

  ISteamNetworkingSockets* sockets;

  std::unordered_map<ClientId, HSteamNetConnection> connections;

  //
  // Client event receive methods
  //
  void receiveEvents();
  void onJoinRequest(const protocol::JoinRequest*);

  //
  // Server event send methods
  //
  void sendAnnouncement(std::string);
  ClientId createNewConnection(HSteamNetConnection);
  void sendEvent(flatbuffers::FlatBufferBuilder&, ClientId);
  void sendQueuedEvents();

  //
  // Connection status change callbacks
  //
  void onConnecting(std::string, HSteamNetConnection);
  void onConnected(std::string, HSteamNetConnection);
  void onProblemDetected(std::string, HSteamNetConnection);
  void onClosedByPeer(std::string, HSteamNetConnection);
  void onDisconnect(std::string, HSteamNetConnection);
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
