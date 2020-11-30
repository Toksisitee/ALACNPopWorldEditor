#include "std.h"
#include "network.h"
#include "pop.h"
#include "engine.h"
#include "dialogs.h"
#include <tchar.h>

Network             net;
ENetAddress		address;
ENetHost		*server;
ENetEvent		  event;
ENetPacket		*packet;
ENetHost		*client;
ENetPeer		  *peer;

void Network::InitNetwork(bool bHosting)
{
	if (m_bENetInitialized)
		return;

	m_bENetInitialized = true;

	m_bIsHost = bHosting;
	InitConsole();
	Log(NET_DEBUG, "sizeof(Packet): ", sizeof(struct Packet));
	Log(NET_DEBUG, "sizeof(LandPacket): ", sizeof(struct LandPacket));

	if (bHosting)
	{
		m_thProcess = std::thread(&Network::ProcessHost, this);
		m_thProcess.detach();
	}
	else
	{
		m_thProcess = std::thread(&Network::ProcessClient, this);
		m_thProcess.detach();
	}
}

void Network::InitConsole()
{
	AllocConsole();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout); 
	freopen_s((FILE**)stdin,  "CONIN$",  "r", stdin); 
	freopen_s((FILE**)stderr, "CONOUT$", "w", stderr); 
	printf(">> Initialized console.\n");

	Network::Log(NET_DEBUG, "m_bIsHost: ", IsHost(), " m_bENetInitialized: ", IsInitialized(), " m_bConnected: ", IsConnected(), " m_nConnections: ", +GetConnections());

	if (m_bIsHost)
		SetConsoleTitle("[Host]");
	else
		SetConsoleTitle("[Client]");

	std::cout << "Known issues"<< std::endl;
	std::cout << "1. Timeout during synchronization requests of large maps. However, rest assured that everything is sent & received." << std::endl;
	std::cout << "2. ???\n" << std::endl;
	

#if _NET_DEBUG_CONNECTION
	#ifdef _DEBUG
		memcpy(m_szIP, NET_CONNECT_IP, strlen(NET_CONNECT_IP));
		m_nPort = NET_PORT;
	#endif
#else
		std::cout << "Enter IP:" << std::endl;
		std::cin >> m_szIP;
		std::cout << "Enter Port:" << std::endl;
		std::cin >> m_nPort;
#endif
}

bool Network::ProcessHost()
{
	if (enet_initialize() != 0) 
	{
		Network::Log(NET_ERROR, "Failed to initialize ENet!");
		std::this_thread::sleep_for(std::chrono::seconds(5));
		exit(EXIT_FAILURE);
	}
	else
	{
		Network::Log(NET_SUCCESS, "Initialized ENet!");
	}
	atexit(enet_deinitialize);

	address.host = ENET_HOST_ANY;
	address.port = m_nPort;
	server = enet_host_create(&address, NET_SERVER_MAX_CONNECTIONS, NET_MAX_CHANNELS, 0, 0);

	if (server == NULL) 
	{
		Network::Log(NET_ERROR, "Failed to create the ENet server!");
		std::this_thread::sleep_for(std::chrono::seconds(5));
		exit(EXIT_FAILURE);
	}
	else
	{
		Network::Log(NET_SUCCESS, "Created server!");
	}
	atexit(enet_deinitialize);

	while (1) 
	{
		if (enet_host_service(server, &event, 10) > 0)
		{
			switch(event.type) 
			{
				case ENET_EVENT_TYPE_CONNECT:
					Connected();
					Network::Log(NET_SERVER, "Client connected: ", event.peer->address.host, ":", event.peer->address.port, " Total: ", +GetConnections());
					enet_peer_ping_interval(event.peer, 50);
				break;

				case ENET_EVENT_TYPE_RECEIVE:
				{
					char szBuffer[254];
					sprintf(szBuffer, "%s", event.packet->data);
					sscanf(szBuffer, "%s", &szBuffer);
					
					if (strcmp(szBuffer, "msg") == 0)
						OnPacket(event, true);
					else if (m_nRecvWaitCount != 0)
						OnPacket(event, false, m_nSerializePacketType);
					else
						OnPacket(event);

					enet_packet_destroy(event.packet);
				}
				break;

				case ENET_EVENT_TYPE_DISCONNECT:
					Disconnected();
					Network::Log(NET_SERVER, event.peer->address.host, ":", event.peer->address.port, " disconnected", " Total: ", +GetConnections());
					event.peer->data = NULL;
				break;
			}
		}
	}
 }

bool Network::ProcessClient()
{
	if (enet_initialize() != 0) 
	{
		Network::Log(NET_ERROR, "Failed to initialize ENet!");
		exit(EXIT_FAILURE);
	}
	else
	{
		Network::Log(NET_SUCCESS, "Initialized ENet!");
	}
	atexit(enet_deinitialize);

	client = enet_host_create(NULL, NET_CLIENT_MAX_CONNECTIONS, NET_MAX_CHANNELS, 0, 0);
	if (client == NULL) 
	{
		Network::Log(NET_ERROR, "Failed to create a client!");
		std::this_thread::sleep_for(std::chrono::seconds(5));
		exit(EXIT_FAILURE);
	}
	else
	{
		Network::Log(NET_SUCCESS, "Created client!");
	}
	atexit(enet_deinitialize);

	enet_address_set_host(&address, m_szIP);
	address.port = m_nPort;

	peer = enet_host_connect(client, &address, NET_MAX_CHANNELS, 0);
	do 
	{
		if (enet_host_service(client, &event, NET_CONNECTION_RETRY_TIMER) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) 
		{
			Log(NET_CLIENT, "Connected to host!");
			m_bConnected = true;
		} 
		else 
		{
			Log(NET_CLIENT, "Connection to host failed. Retrying.");
			enet_peer_reset(peer);
			peer = enet_host_connect(client, &address, NET_MAX_CHANNELS, 0);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	} 
	while (!IsConnected());

	while (1) 
	{
		if (enet_host_service(client, &event, 10) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
				Connected();
				Network::Log(NET_CLIENT, "Connected to host: ", event.peer->address.host, ":", event.peer->address.port, " Total: ", +GetConnections());
				enet_peer_ping_interval(event.peer, 50);
			break;

			case ENET_EVENT_TYPE_RECEIVE:
			{
				char szBuffer[254];
				sprintf(szBuffer, "%s", event.packet->data);
				sscanf(szBuffer, "%s", &szBuffer);
					
				if (strcmp(szBuffer, "msg") == 0)
					OnPacket(event, true);
				else if (m_nRecvWaitCount != 0)
					OnPacket(event, false, m_nSerializePacketType);
				else
					OnPacket(event);

				enet_packet_destroy(event.packet);
			}
			break;

			case ENET_EVENT_TYPE_DISCONNECT:
				Disconnected();
				Network::Log(NET_CLIENT, event.peer->address.host, ":", event.peer->address.port, " has disconnected.");
				event.peer->data = NULL;
			break;
			}
		}
	}
}
 
void Network::SendPacket(WORD *p, size_t nSize)
{
	if (!IsConnected())
		return;

	Log(NET_DEBUG, "Network::SendPacket");

	ENetPacket *packet = enet_packet_create(p, nSize, ENET_PACKET_FLAG_RELIABLE);
	
	if (Network::IsHost())
		enet_host_broadcast(server, 0, packet);
	else
		enet_peer_send(peer, 0, packet);
}

void Network::SendPacket(const char* szMsg)
{
	if (!IsConnected())
		return;

	Log(NET_DEBUG, "Network::SendPacket");

	ENetPacket *packet = enet_packet_create(szMsg, strlen(szMsg) + 1, ENET_PACKET_FLAG_RELIABLE);
	
	if (Network::IsHost())
		enet_host_broadcast(server, 0, packet);
	else
		enet_peer_send(peer, 0, packet);
}

void Network::SendPacket(Packet *p, size_t nSize)
{
	if (!IsConnected())
		return;

	Log(NET_DEBUG, "Network::SendPacket");

	ENetPacket *packet = enet_packet_create(p, nSize, ENET_PACKET_FLAG_RELIABLE);
	
	if (Network::IsHost())
		enet_host_broadcast(server, 0, packet);
	else
		enet_peer_send(peer, 0, packet);
}

void Network::SendPacket(LandPacket *p, size_t nSize)
{
	if (!IsConnected())
		return;

	Log(NET_DEBUG, "Network::SendPacket");

	ENetPacket *packet = enet_packet_create(p, nSize, ENET_PACKET_FLAG_RELIABLE);
	
	if (Network::IsHost())
		enet_host_broadcast(server, 0, packet);
	else
		enet_peer_send(peer, 0, packet);
}

void Network::OnPacket(ENetEvent & event, bool bMsg, SBYTE nPacketType)
{
	Log(NET_DEBUG, "Network::OnPacket");

	if (bMsg)
	{
		char szBuffer[254];
		SBYTE nType = -1;

		sprintf(szBuffer, "%s", event.packet->data);
		Log(NET_DEBUG, "szBuffer: ", szBuffer);
		sscanf(szBuffer, "msg %hhd", &nType);

		switch (nType)
		{
		case PACKETTYPEMSG_DISCOVERY_MANA:
		{
			int nIdx = 0,
				nMana = 0;

			sscanf(szBuffer, "msg %hhd %d %d", &nType, &nIdx, &nMana);
			Network::OnDiscoveryMana(nIdx, nMana);
		}
			break;
		case PACKETTYPEMSG_SYNC_MISC:
		{
			DWORD dwSpellsAvailable = 0,
				dwBuildingsAvailable = 0,
				dwSpellsNotCharging = 0;

			UBYTE nAllies0 = 0,
				nAllies1 = 0,
				nAllies2 = 0,
				nAllies3 = 0,
				nNumPlayers = 0,
				nLevelFlags = 0,
				nCP0 = 0,
				nCP1 = 0,
				nCP2 = 0,
				nObjBank = 0,
				nMapBank = 0;

			sscanf(szBuffer, "msg %hhd %lu %lu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %lu", 
				&nType, &dwSpellsAvailable, &dwBuildingsAvailable, &nAllies0, &nAllies1, &nAllies2, &nAllies3, 
				&nNumPlayers, &nLevelFlags, &nCP0, &nCP1, &nCP2, &nObjBank, &nMapBank, &dwSpellsNotCharging);
			
			Network::OnSyncMisc(dwSpellsAvailable, dwBuildingsAvailable, nAllies0, nAllies1, nAllies2, 
				nAllies3, nNumPlayers, nLevelFlags, nCP0, nCP1, nCP2, nObjBank, nMapBank, dwSpellsNotCharging);
		}
			break;
		default:
			Log(ERROR, "Received unknown MSG packet! [", event.packet->data[0], "]");
			break;
		}

		return;
	}

	if (m_nRecvWaitCount > 0)
	{
		switch (nPacketType)
		{
		case PACKETTYPE_SYNC_MAP:
				Network::OnSyncMap(event.packet->data);
			break;
		default:
			Log(ERROR, "Received unknown serialization packet! [", event.packet->data[0], "]");
			break;
		}

		m_nRecvWaitCount--;
		return;
	}

	switch (event.packet->data[0])
	{
	case PACKETTYPE_CREATE_OBJECT:
		Network::OnCreateObject(event.packet->data[1], event.packet->data[2], event.packet->data[3], event.packet->data[4], event.packet->data[5]);
		break;
	case PACKETTYPE_LAND_MODIFY:
	{
		for (int i = 0; i <= ((LAND_PACKETS_BUFFER - 1) / 2); i++)
		{

#ifdef _DEBUG
#if _NET_DEBUG_LAND
			printf("[Recv] nLand(%i): %i, nHeight(%i): %i\n", i + i + 1, event.packet->data[i + i + 1], i + i + 2, event.packet->data[i + i + 2]);
#endif
#endif
			if ((event.packet->data[i + i + 1] == 0 && event.packet->data[i + i + 2] == 0))
			{
				EngineUpdateView();
				EngineUpdateMiniMap();
				break;
			}

			Network::OnEngineSetGroundHeight(event.packet->data[i + i + 1], event.packet->data[i + i + 2], i + 1);
		}
	}
		break;
	case PACKETTYPE_UPDATE_VIEW:
		Network::OnEngineUpdateView();
		break;
	case PACKETTYPE_DELETE_OBJECT:
		Network::OnObjectDelete(event.packet->data[1]);
		break;
	case PACKETTYPE_MOVE_OBJECT:
		Network::OnObjectMove(event.packet->data[1], event.packet->data[2], event.packet->data[3], event.packet->data[4]);
		break;
	case PACKETTYPE_OWNER_OBJECT:
		Network::OnObjectOwner(event.packet->data[1], event.packet->data[2]);
		break;
	case PACKETTYPE_TYPE_OBJECT:
		Network::OnObjectType(event.packet->data[1], event.packet->data[2], event.packet->data[3]);
		break;
	case PACKETTYPE_MODEL_OBJECT:
		Network::OnObjectModel(event.packet->data[1], event.packet->data[2], event.packet->data[3]);
		break;
	case PACKETTYPE_SWAP_TRIBE:
		Network::OnSwapTribe(event.packet->data[1], event.packet->data[2], event.packet->data[3], event.packet->data[4], event.packet->data[5]);
		break;
	case PACKETTYPE_ROTATE_OBJECT:
		Network::OnObjectRotate(event.packet->data[1], event.packet->data[2], event.packet->data[3]);
		break;
	case PACKETTYPE_NEW_MAP_ASK:
		Network::OnNewMapAsk();
		break;
	case PACKETTYPE_NEW_MAP:
		Network::OnNewMap();
		break;
	case PACKETTYPE_AVAILABLE_DISC:
		Network::OnAvailableDiscoveries(event.packet->data[1]);
		break;
	case PACKETTYPE_SPELL_RESTRICTIONS:
		Network::OnSpellRestrictions(event.packet->data[1]);
		break;
	case PACKETTYPE_ALLIES:
		Network::OnAllies(event.packet->data[1]);
		break;
	case PACKETTYPE_FOG_OF_WAR:
		Network::OnFogOfWar(event.packet->data[1]);
		break;
	case PACKETTYPE_OMNIPRESENCE:
		Network::OnOmnipresence(event.packet->data[1]);
		break;
	case PACKETTYPE_NUM_PLAYERS:
		Network::OnNumPlayers(event.packet->data[1]);
		break;
	case PACKETTYPE_NO_GUEST:
		Network::OnNoGuest(event.packet->data[1]);
		break;
	case PACKETTYPE_NO_REINCARNATE_TIME:
		Network::OnNoReincarnationTime(event.packet->data[1]);
		break;
	case PACKETTYPE_COMPUTER_PLAYER_AI:
		Network::OnComputerPlayerIndex(event.packet->data[1], event.packet->data[2]);
		break;
	case PACKETTYPE_OBJECT_BANK:
		Network::OnObjectBank(event.packet->data[1]);
		break;
	case PACKETTYPE_MAP_TYPE:
		Network::OnMapType(event.packet->data[1]);
		break;
	case PACKETTYPE_CREATE_MARKER:
		Network::OnCreateMarker(event.packet->data[1], event.packet->data[2], event.packet->data[3]);
		break;
	case PACKETTYPE_MOVE_MARKER:
		Network::OnMarkerMove(event.packet->data[1], event.packet->data[2], event.packet->data[3]);
		break;
	case PACKETTYPE_DELETE_MARKER:
		Network::OnDeleteMarker(event.packet->data[1]);
		break;
	case PACKETTYPE_TRIGGER_OPTIONS:
		Network::OnTriggerOptions(event.packet->data[1], event.packet->data[2], event.packet->data[3], event.packet->data[4], event.packet->data[5], 
			event.packet->data[6], event.packet->data[7], event.packet->data[8], event.packet->data[9]);
		break;
	case PACKETTYPE_DISCOVERY_OPTIONS:
		Network::OnDiscoveryOptions(event.packet->data[1], event.packet->data[2], event.packet->data[3], event.packet->data[4]);
		break;
	case PACKETTYPE_SYNC_MAP_UPDATE_USER:
		Network::OnSyncUpdateUser();
		break;
	case PACKETTYPESER_SYNC_MAP:
		Network::OnPrepareUpdateMap(event.packet->data[1], event.packet->data[2]);
		break;
	case PACKETTYPE_SYNC_OBJECTS:
		Network::OnSyncObjects(event.packet->data[1], event.packet->data[2], event.packet->data[3],	
			event.packet->data[4], event.packet->data[5], event.packet->data[6], event.packet->data[7], 
			event.packet->data[8], event.packet->data[9], event.packet->data[10], event.packet->data[11], 
			event.packet->data[12], event.packet->data[13]);
		break;
	case PACKETTYPE_TRIGGER_LINK:
		Network::OnTriggerLink(event.packet->data[1], event.packet->data[2], event.packet->data[3], event.packet->data[4]);
		break;
	case PACKETTYPE_DELETE_ALL_OBJECTS:
		Network::OnRemoveAllObjects();
		break;
	case PACKETTYPE_DELETE_ALL_MARKERS:
		Network::OnRemoveAllMarkers();
		break;
	default:
		Log(ERROR, "Received unknown packet! [", event.packet->data[0], "]");
		break;
	}
}

THING* Network::OnCreateObject(BYTE nType, BYTE nModel, BYTE nOwner, WORD x, WORD z)
{
	Log(NET_DEBUG, "Network::OnCreateObject");

	if (ObjectsCount >= MAX_THINGS) return nullptr;

	THING *pThing;
	pThing = new THING;
	memset(pThing, 0, sizeof(THING));

	pThing->Thing.Type = nType;
	pThing->Thing.Model = nModel;
	pThing->Thing.Owner = nOwner;
	pThing->x = (float)x + 0.5f;
	pThing->z = (float)z + 0.5f;
	pThing->Thing.PosX = (x * 2) << 8;
	pThing->Thing.PosZ = (z * 2) << 8;

	LINK(Things, pThing);
	ObjectsCount++;
	EngineUpdateMiniMap();
	DlgInfoUpdate(hDlgInfo);
	DlgSetThingIndex(pThing);
	EngineUpdateView();

	return pThing;
}

void Network::OnEngineSetGroundHeight(int x, WORD h, int nNewPacket)//(int x, int z, WORD h)
{
	if (nNewPacket == 1)
		Log(NET_DEBUG, "Network::OnEngineSetGroundHeight");

	wEngineGround[x] = h;
}

void Network::OnEngineUpdateView()
{
	Log(NET_DEBUG, "Network::OnEngineUpdateView");
	EngineUpdateView();
}

void Network::OnObjectDelete(WORD wThingIdx)
{
	Log(NET_DEBUG, "Network::OnDeleteObj");

	THING *pThing = DlgObjectFindIdx(wThingIdx);
	if (!pThing) return;
	if (pThing) pThing->flags &= ~TF_EDIT_LANDBRIDGE;

	ThingsIndices[pThing->Idx] = 0;
	EngineUpdateView();
	UNLINK(Things, pThing);
	DlgObjectUnlinkObj(pThing);
	delete pThing;
	ObjectsCount--;
	DlgObjectUpdateInfo(hDlgObject);
	EngineUpdateMiniMap();
	DlgInfoUpdate(hDlgInfo);
}

void Network::OnObjectMove(WORD wThingIdx, WORD x, WORD z, bool bIsLandbridge)
{
	Log(NET_DEBUG, "Network::OnMoveObject");

	THING *pThing = DlgObjectFindIdx(wThingIdx);
	if (!pThing) return;

	if (bIsLandbridge)
	{
		pThing->LandBridge.x = (float)x + 0.5f;
		pThing->LandBridge.z = (float)z + 0.5f;
		pThing->Thing.Bluff[0] = (x * 2) << 8;
		pThing->Thing.Bluff[1] = (z * 2) << 8;
	}
	else
	{
		pThing->x = (float)x + 0.5f;
		pThing->z = (float)z + 0.5f;
		pThing->Thing.PosX = (x * 2) << 8;
		pThing->Thing.PosZ = (z * 2) << 8;
	}

	EngineUpdateView();
}

void Network::OnObjectOwner(WORD wThingIdx, BYTE nOwner)
{
	Log(NET_DEBUG, "Network::OnObjectOwner");

	THING *pThing = DlgObjectFindIdx(wThingIdx);
	if (!pThing) return;

	pThing->Thing.Owner = nOwner;
	DlgInfoUpdate(hDlgInfo);
}

void Network::OnObjectType(WORD wThingIdx, BYTE nType, BYTE nModel)
{
	Log(NET_DEBUG, "Network::OnObjectType");

	THING *pThing = DlgObjectFindIdx(wThingIdx);
	if (!pThing) return;
	memset(&pThing->Thing.Bluff, 0, sizeof(pThing->Thing.Bluff));

	pThing->Thing.Type = nType;
	pThing->Thing.Model = nModel;
	DlgInfoUpdate(hDlgInfo);
}

void Network::OnObjectModel(WORD wThingIdx, BYTE nModel, BYTE nIslandNum)
{
	Log(NET_DEBUG, "Network::OnObjectModel");

	THING* t = DlgObjectFindIdx(wThingIdx);
	if (!t) return;
	memset(&t->Thing.Bluff, 0, sizeof(t->Thing.Bluff));
	t->Thing.Model = nModel;
	t->Thing.Scenery.IslandNum = nIslandNum;
	DlgInfoUpdate(hDlgInfo);
}

void Network::OnObjectRotate(WORD wThingIdx, BYTE nType,  WORD wAngle)
{
	Log(NET_DEBUG, "Network::OnObjectRotate");

	THING *pThing = DlgObjectFindIdx(wThingIdx);
	if (!pThing) return;

	if (nType == T_BUILDING)
		pThing->Thing.Building.Angle = wAngle;
	else
		pThing->Thing.Scenery.Angle = wAngle;
}

void Network::OnSwapTribe(BYTE t1, BYTE t2, BYTE t3, BYTE t4, bool f)
{
	THING *pThing = Things;

	do
	{
		if(IsNotNeutral(pThing))
		{
			if(t1 == pThing->Thing.Owner)
				pThing->Thing.Owner = t2;
			else if(f && t3 == pThing->Thing.Owner)
				pThing->Thing.Owner = t4;
		}

		pThing = pThing->Next;
	}
	while(pThing != Things);

	DlgInfoUpdate(hDlgInfo);
	EngineUpdateMiniMap();
}

void Network::OnNewMapAsk()
{
	Log(NET_DEBUG, "Network::OnNewMapAsk");
	if (ModalMsg(SZ_CONFIRM_NEW_LEVEL_NET, APPNAME, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDYES)
	{
		struct Packet *p = new Packet;
		p->wType = PACKETTYPE_NEW_MAP;
		net.SendPacket(p);
		EngineNewMap();
		p->del();
	}

	net.SetNewMap(false);
}

void Network::OnNewMap()
{
	Log(NET_DEBUG, "Network::OnNewMap");
	net.SetNewMap(false);
	EngineNewMap();
}

void Network::OnAvailableDiscoveries(WPARAM wParam)
{
	Log(NET_DEBUG, "Network::OnAvailableDiscoveries");
	DlgSpellsBuildingsProcModify(wParam, true);
}

void Network::OnSpellRestrictions(WPARAM wParam)
{
	Log(NET_DEBUG, "Network::OnSpellRestrictions");
	DlgSpellsNotChargingProcModify(wParam, true);
}

void Network::OnAllies(WPARAM wParam)
{
	Log(NET_DEBUG, "Network::OnAllies");
	DlgAlliesProcModify(wParam, true);
}

void Network::OnNumPlayers(int nPlayers)
{
	Log(NET_DEBUG, "Network::OnNumPlayers");
	leveldat->Header.v2.NumPlayers = nPlayers;
	DlgHeaderUpdate(hDlgHeader);
}

void Network::OnOmnipresence(bool bOmnipresence)
{
	Log(NET_DEBUG, "Network::OnOmnipresence");
	
	if(bOmnipresence)
		leveldat->Header.v2.LevelFlags |= LEVEL_FLAGS_SHAMAN_OMNI;
	else
		leveldat->Header.v2.LevelFlags &= ~LEVEL_FLAGS_SHAMAN_OMNI;

	DlgHeaderUpdate(hDlgHeader);
}

void Network::OnFogOfWar(bool bFog)
{
	Log(NET_DEBUG, "Network::OnFogOfWar");

	if(bFog)
		leveldat->Header.v2.LevelFlags |= LEVEL_FLAGS_USE_FOG;
	else
		leveldat->Header.v2.LevelFlags &= ~LEVEL_FLAGS_USE_FOG;

	DlgHeaderUpdate(hDlgHeader);
}

void Network::OnNoGuest(bool bGuestSpells)
{
	Log(NET_DEBUG, "Network::OnNoGuest");

	if(bGuestSpells)
		leveldat->Header.v2.LevelFlags |= LEVEL_FLAGS_NO_GUEST;
	else
		leveldat->Header.v2.LevelFlags &= ~LEVEL_FLAGS_NO_GUEST;

	DlgHeaderUpdate(hDlgHeader);
}

void Network::OnNoReincarnationTime(bool bReincarnationTime)
{
	Log(NET_DEBUG, "Network::OnNoReincarnationTime");

	if(bReincarnationTime)
		leveldat->Header.v2.LevelFlags |= LEVEL_NO_REINCARNATE_TIME;
	else
		leveldat->Header.v2.LevelFlags &= ~LEVEL_NO_REINCARNATE_TIME;

	DlgHeaderUpdate(hDlgHeader);
}

void Network::OnComputerPlayerIndex(BYTE nTribe, int nScriptIndex)
{
	Log(NET_DEBUG, "Network::OnComputerPlayerIndex");
	leveldat->Header.v2.ComputerPlayerIndex[nTribe-1] = nScriptIndex;
	DlgHeaderUpdate(hDlgHeader);
}

void Network::OnObjectBank(BYTE nBank)
{
	Log(NET_DEBUG, "Network::OnObjectBank");
	leveldat->Header.v2.ObjectsBankNum = nBank;
	DlgObjBankUpdate(hDlgObjBank);
	EngineSetTreeType();
}

void Network::OnMapType(BYTE nBank)
{
	Log(NET_DEBUG, "Network::OnMapType");
	leveldat->Header.v2.LevelType = nBank;
	DlgMapTypeUpdate(hDlgMapType);
}

void Network::OnCreateMarker(int nMarker, WORD x, WORD z)
{
	Log(NET_DEBUG, "Network::OnCreateMarker");
	Markers[nMarker].x = (float)x + 0.5f;
	Markers[nMarker].z = (float)z + 0.5f;
	leveldat->Header.v2.Markers[nMarker] = ((z * 2) << 8) | (x * 2);
	DlgMarkersUpdate(hDlgMarkers);
	EngineUpdateView();
}

void Network::OnMarkerMove(int nMarker, WORD x, WORD z)
{
	Log(NET_DEBUG, "Network::OnMarkerMove");
	Markers[nMarker].x = (float)x + 0.5f;
	Markers[nMarker].z = (float)z + 0.5f;
	leveldat->Header.v2.Markers[nMarker] = ((z * 2) << 8) | (x * 2);
	DlgMarkersUpdate(hDlgMarkers);
	EngineUpdateView();
}

void Network::OnDeleteMarker(int nMarker)
{
	Log(NET_DEBUG, "Network::OnDeleteMarker");
	leveldat->Header.v2.Markers[nMarker] = ((0) << 8) | (0);
	Markers[nMarker].x = 0.5f;
	Markers[nMarker].z = 0.5f;
	Markers[nMarker].ex = 0.5f;
	Markers[nMarker].ez = 0.5f;
	Markers[nMarker].ey = 0.0f;
	DlgMarkersUpdate(hDlgMarkers);
}

void Network::OnTriggerOptions(WORD wThingIdx, BYTE nType, BYTE nCellRadius, SBYTE nOccurences, WORD nTriggerCount, WORD nPrayTime, WORD nInactiveTime, bool bStartInactive, bool bCreateOwned)
{
	Log(NET_DEBUG, "Network::OnTriggerOptions");

	THING *pThing = DlgObjectFindIdx(wThingIdx);
	if (!pThing) return;

	pThing->Thing.Trigger.TriggerType = nType;
	pThing->Thing.Trigger.CellRadius = nCellRadius;
	pThing->Thing.Trigger.NumOccurences = nOccurences;
	pThing->Thing.Trigger.TriggerCount = nTriggerCount;
	pThing->Thing.Trigger.PrayTime = nPrayTime;
	pThing->Thing.Trigger.InactiveTime = nInactiveTime;
	pThing->Thing.Trigger.StartInactive = bStartInactive;
	pThing->Thing.Trigger.CreatePlayerOwned = bCreateOwned;
	DlgTriggerUpdateInfo(hDlgTrigger);
}

void Network::OnDiscoveryOptions(WORD wThingIdx, BYTE nAvailability, BYTE nType, BYTE nModel)
{
	Log(NET_DEBUG, "Network::OnDiscoveryOptions");

	THING *pThing = DlgObjectFindIdx(wThingIdx);
	if (!pThing) return;

	pThing->Thing.General.AvailabilityType = nAvailability;
	pThing->Thing.General.DiscoveryType = nType;
	pThing->Thing.General.DiscoveryModel = nModel;
	DlgDiscoveryUpdateInfo(hDlgDiscovery);
}

void Network::OnDiscoveryMana(WORD wThingIdx, int nManaAmount)
{
	Log(NET_DEBUG, "Network::OnDiscoveryMana");

	THING *pThing = DlgObjectFindIdx(wThingIdx);
	if (!pThing) return;

	pThing->Thing.General.ManaAmt = nManaAmount;
	DlgDiscoveryUpdateInfo(hDlgDiscovery);
}

void Network::OnSyncMapAsk()
{
	if (net.IsInitialized())
	{
		if (ModalMsg(SZ_CONFIRM_SYNC_LEVEL_ASK, APPNAME, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDYES)
		{
			struct Packet *p = new Packet;
			p->wType = PACKETTYPE_SYNC_MAP_UPDATE_USER;
			net.SendPacket(p);
			p->del();
		}
	}
}

void Network::OnSyncUpdateUser()
{
	Log(NET_DEBUG, "Network::OnSyncUpdateUser");

	if (ModalMsg(SZ_CONFIRM_SYNC_LEVEL, APPNAME, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDYES)
	{
		ModalMsg(SZ_SYNC_LEVEL_WARNING, APPNAME, MB_ICONSTOP | MB_OK | MB_DEFBUTTON2);
		struct Packet *pPSer = new Packet;
		pPSer->wType = PACKETTYPESER_SYNC_MAP;
		pPSer->wData[0] = 1;
		pPSer->wData[1] = PACKETTYPE_SYNC_MAP;
		net.SendPacket(pPSer);
		pPSer->del();

		WORD* wLandData = reinterpret_cast<WORD*>(&wEngineGround);
		net.SendPacket(wLandData, sizeof(wEngineGround));

		THING *pThing = Things;
		struct Packet *pPObjs = new Packet;

		int nDiscoveries = 0;
		int nDiscoveryIdx[MAX_V2_THINGS];
		int nDiscoveryMana[MAX_V2_THINGS];

		if (pThing)
		{
			do
			{
				pPObjs->wType = PACKETTYPE_SYNC_OBJECTS;
				pPObjs->wData[0] = pThing->Thing.Type;
				pPObjs->wData[1] = pThing->Thing.Model;
				pPObjs->wData[2] = pThing->Thing.Owner;
				pPObjs->wData[3] = (WORD)(pThing->x - 0.5f);
				pPObjs->wData[4] = (WORD)(pThing->z - 0.5f);

				switch (pThing->Thing.Type)
				{
				case T_BUILDING:
				{
					pPObjs->wData[5] = pThing->Thing.Building.Angle;
				}
				break;
				case T_SCENERY:
				{
					pPObjs->wData[5] = pThing->Thing.Scenery.Angle;
					pPObjs->wData[6] = pThing->Thing.Scenery.IslandNum;
				}
				break;
				case T_GENERAL:
				{
					if (pThing->Thing.Model == M_GENERAL_TRIGGER)
					{
						pPObjs->wData[5] = pThing->Thing.Trigger.TriggerType;
						pPObjs->wData[6] = pThing->Thing.Trigger.CellRadius;
						pPObjs->wData[12] = pThing->Thing.Trigger.NumOccurences;
						pPObjs->wData[7] = pThing->Thing.Trigger.TriggerCount;
						pPObjs->wData[8] = pThing->Thing.Trigger.PrayTime;
						pPObjs->wData[9] = pThing->Thing.Trigger.InactiveTime;
						pPObjs->wData[10] = pThing->Thing.Trigger.StartInactive;
						pPObjs->wData[11] = pThing->Thing.Trigger.CreatePlayerOwned;
					}
					else if (pThing->Thing.Model == M_GENERAL_DISCOVERY)
					{
						pPObjs->wData[5] = pThing->Thing.General.AvailabilityType;
						pPObjs->wData[6] = pThing->Thing.General.DiscoveryType;
						pPObjs->wData[7] = pThing->Thing.General.DiscoveryModel;
						nDiscoveryIdx[nDiscoveries] = pThing->Idx;
						nDiscoveryMana[nDiscoveries] = pThing->Thing.General.ManaAmt;
						nDiscoveries++;
					}
				}
				break;
				case T_EFFECT:
				{
					// TODO Landbrige bs
				}
				break;
				}

				net.SendPacket(pPObjs);
				pThing = pThing->Next;

			} while (pThing != Things);

			pPObjs->del();

			for (int i = 0; i <= nDiscoveries; i++)
			{
				char szPacket[100];
				sprintf(szPacket, "msg %d %d %d", PACKETTYPEMSG_DISCOVERY_MANA, nDiscoveryIdx[i], nDiscoveryMana[i]);
				net.SendPacket(szPacket);
			}
		}

		for (int i = 0; i < 256; i++)
		{
			struct Packet *p = new Packet;
			p->wType = PACKETTYPE_CREATE_MARKER;
			p->wData[0] = i;
			p->wData[1] = (WORD)(Markers[i].x - 0.5f);
			p->wData[2] = (WORD)(Markers[i].z - 0.5f);
			net.SendPacket(p);
			p->del();
		}

		char szPacket[1024];
		sprintf(szPacket, "msg %hhd %lu %lu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %hhu %lu", PACKETTYPEMSG_SYNC_MISC, 
				leveldat->Header.v2.DefaultThings.SpellsAvailable, leveldat->Header.v2.DefaultThings.BuildingsAvailable,
				leveldat->Header.v2.DefaultAllies[0], leveldat->Header.v2.DefaultAllies[1], leveldat->Header.v2.DefaultAllies[2],
				leveldat->Header.v2.DefaultAllies[3], leveldat->Header.v2.NumPlayers, leveldat->Header.v2.LevelFlags, 
				leveldat->Header.v2.ComputerPlayerIndex[0], leveldat->Header.v2.ComputerPlayerIndex[1], leveldat->Header.v2.ComputerPlayerIndex[2],
				leveldat->Header.v2.ObjectsBankNum, leveldat->Header.v2.LevelType, leveldat->Header.v2.DefaultThings.SpellsNotCharging);
		net.SendPacket(szPacket);

		THING *Trigger = Things;
		do
		{
			if (Trigger)
			{
				if (Trigger->Thing.Type == T_GENERAL && Trigger->Thing.Model == M_GENERAL_TRIGGER)
				{
					for (int i = 0; i < 10; i++)
					{
						if (Trigger->Links[i])
						{
							struct Packet *p = new Packet;
							p->wType = PACKETTYPE_TRIGGER_LINK;
							p->wData[0] = true;
							p->wData[1] = Trigger->Idx;
							p->wData[2] = Trigger->Links[i]->Idx;
							p->wData[3] = i;
							net.SendPacket(p);
							p->del();
						}
					}
				}

				Trigger = Trigger->Next;
			}
		} while (Trigger != Things);
	}

}

void Network::OnPrepareUpdateMap(BYTE nRecvWaitCount, BYTE nSerializePacketType)
{
	Log(NET_DEBUG, "Network::OnPrepareUpdateMap");
	m_nRecvWaitCount = nRecvWaitCount;
	m_nSerializePacketType = nSerializePacketType;
	EngineNewMap();
}

void Network::OnSyncMap(WORD* wLandData)
{
	Log(NET_DEBUG, "Network::OnSyncMap");

	for (int i = 0; i < GROUND_X_SIZE * GROUND_Z_SIZE; i++)
	{
		wEngineGround[i] = wLandData[i];

#ifdef _DEBUG
	#if _NET_DEBUG_LAND
		printf("Block: %i, Height: %hu\n", i, wEngineGround[i]);
	#endif
#endif
	}

	EngineUpdateView();
	EngineUpdateMiniMap();
}

void Network::OnSyncObjects(BYTE nType, BYTE nModel, BYTE nOwner, WORD x, WORD z, WORD wData5, WORD wData6, WORD wData7, WORD wData8, WORD wData9, WORD wData10, WORD wData11, SBYTE wData12)
{
	THING *pThing;
	pThing = Network::OnCreateObject(nType, nModel, nOwner, x, z);
	if (!pThing) return;

	switch (nType)
	{
	case T_PERSON:
	{
		
	}
		break;
	case T_BUILDING:
	{
		pThing->Thing.Building.Angle = wData5;
	}
		break;
	case T_SCENERY:
	{
		pThing->Thing.Scenery.Angle = wData5;
		pThing->Thing.Scenery.IslandNum = wData6;
	}
		break;
	case T_GENERAL:
	{
		if (pThing->Thing.Model == M_GENERAL_TRIGGER)
		{
			OnTriggerOptions(pThing->Idx, wData5, wData6, wData12, wData7, wData8, wData9, wData10, wData11);		
		}

		if (pThing->Thing.Model == M_GENERAL_DISCOVERY)
		{
			OnDiscoveryOptions(pThing->Idx, wData5, wData6, wData7);
		}
	}
		break;
	case T_EFFECT:
	{
		if (pThing->Thing.Model == M_EFFECT_LAND_BRIDGE)
		{
			pThing->LandBridge.x = (float)(((pThing->Thing.Bluff[0] & 0xFFFF) >> 8) / 2) + 0.5f;
			pThing->LandBridge.z = (float)(((pThing->Thing.Bluff[1] & 0xFFFF) >> 8) / 2) + 0.5f;
		}
	}
		break;
	}
}

void Network::OnSyncMisc(ULONG dwAvailableSpells, ULONG dwBuildingsAvailable, UBYTE nAllies0, UBYTE nAllies1, UBYTE nAllies2, UBYTE nAllies3, UBYTE nNumPlayers, UBYTE nLevelFlags, UBYTE nCP0, UBYTE nCP1, UBYTE nCP2, UBYTE nObjBank, UBYTE nMapBank, DWORD dwSpellsRestrictions)
{
	leveldat->Header.v2.DefaultThings.SpellsAvailable = dwAvailableSpells;
	leveldat->Header.v2.DefaultThings.BuildingsAvailable = dwBuildingsAvailable;
	leveldat->Header.v2.DefaultAllies[0] = nAllies0;
	leveldat->Header.v2.DefaultAllies[1] = nAllies1;
	leveldat->Header.v2.DefaultAllies[2] = nAllies2;
	leveldat->Header.v2.DefaultAllies[3] = nAllies3;
	leveldat->Header.v2.NumPlayers = nNumPlayers;
	leveldat->Header.v2.LevelFlags = nLevelFlags;
	leveldat->Header.v2.ComputerPlayerIndex[0] = nCP0;
	leveldat->Header.v2.ComputerPlayerIndex[1] = nCP1;
	leveldat->Header.v2.ComputerPlayerIndex[2] = nCP2;
	leveldat->Header.v2.ObjectsBankNum = nObjBank;
	leveldat->Header.v2.LevelType = nMapBank;
	leveldat->Header.v2.DefaultThings.SpellsNotCharging = dwSpellsRestrictions;

	EngineSetTreeType();
}

void Network::OnTriggerLink(bool link, WORD wSourceIdx, WORD wTargetIdx, BYTE nSlot)
{
	ThingLink = DlgObjectFindIdx(wSourceIdx);
	if (!ThingLink) return;
	THING *pThingTarget = DlgObjectFindIdx(wTargetIdx);
	if (!pThingTarget) return;

	if (!link)
	{
		ThingLink->Links[nSlot] = 0;
		DlgLinkFixLinks();
	}
	else
		ThingLink->Links[nSlot] = pThingTarget;
}

void Network::OnRemoveAllObjects()
{
	Log(NET_DEBUG, "Network::OnRemoveAllObjects");
	const int nResult = MessageBox(NULL, _T("User asks to delete all objects, do you accept?"), _T("Delete Objects"), MB_YESNO);
	if (nResult == IDNO) return;
	PurgeObjects();
}

void Network::OnRemoveAllMarkers()
{
	Log(NET_DEBUG, "Network::OnRemoveAllMarkers");
	const int nResult = MessageBox(NULL, _T("User asks to reset all markers, do you accept?"), _T("Reset Markers"), MB_YESNO);
	if (nResult == IDNO) return;
	ResetMarkers();
}

template<class... Args>
void Network::Log(BYTE nLogType, Args... args)
{

	if (nLogType == NET_DEBUG)
	{
#if !_DEBUG
		return;
#endif
	}

	std::string strType = std::string();

	switch (nLogType)
	{
	case NET_CLIENT:
		strType = SZ_NET_CLIENT;
		break;
	case NET_SERVER:
		strType = SZ_NET_SERVER;
		break;
	case NET_ERROR:
		strType = SZ_NET_ERROR;
	case NET_SUCCESS:
		strType = SZ_NET_SUCCESS;
		break;
	case NET_DEBUG:
		strType = SZ_NET_DEBUG;
		break;
	}

	std::cout << strType << " ";
	(std::cout << ... << args) << std::endl;
}