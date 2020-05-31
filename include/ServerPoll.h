#pragma once

#include <iostream>
#include <vector>
#include <thread>
#include <SFML\Network.hpp>
#include <chrono>

#include <queue>
#include <atomic>

#include <Krawler.h>

#include "GameClient.h"
#include "ServerPackets.h"

#define SERVER_LISTENING_PORT 32000
#define SERVER_DELTA_TIME 16000
#define EXPECTED_VERSION_NUM "0.0.1"
#define MAX_CLIENTS 5
#define CLIENT_TIMEOUT_IN_SECONDS 5

class ServerPoll
{
public:

	ServerPoll();
	~ServerPoll() = default;

	bool loadServer();
	void runServer();
	void closeServer();

	const std::atomic_bool& hasFinishedClosingConnections() const { return m_bFinishedClosing; }

private:
	struct InboundMessage
	{
		sf::Packet receivedPacket;
		sf::IpAddress senderIp;
		unsigned short senderPort;
	};

	void pollConnected();
	int handleReceived();
	void processQueue();

	void processInboundMessage(InboundMessage& message);
	MessageType getMessageType(InboundMessage& message);

	void handleEstablish(InboundMessage& message);
	void handleMove(InboundMessage& message);

	std::vector<GameClient>::iterator findGameClientFromInbound(InboundMessage& message);
	std::vector<GameClient> m_connectedClients;

	using MessageSenderPair = std::pair<GameClient*, ServerClientMessage*>;
	std::queue<MessageSenderPair> m_messages;

	sf::UdpSocket m_commsSocket;

	std::atomic_bool m_bIsRunning = true;
	std::atomic_bool m_bFinishedClosing = false;

	bool m_bLogExeTime = false;
	int m_frames = 0;
};