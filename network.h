#pragma once
#include "pop.h"

extern ENetAddress	  address;
extern ENetHost		  *server;
extern ENetEvent	    event;
extern ENetPacket	  *packet;
extern ENetHost		  *client;
extern ENetPeer			*peer;

#ifdef _DEBUG

#define _NET_DEBUG_CONNECTION			1
#define NET_CONNECT_IP					"localhost"
#define	NET_PORT						(1234)

#define	_NET_DEBUG_LAND					1

#endif

#define LAND_PACKETS_BUFFER				4096 // LAND_PACKETS / 2. 
#define NET_CONNECTION_RETRY_TIMER		3000
#define NET_SERVER_MAX_CONNECTIONS		1
#define NET_CLIENT_MAX_CONNECTIONS		1
#define NET_MAX_CHANNELS				1
#define NET_CLIENT						(0)
#define NET_SERVER						(1)
#define NET_ERROR						(2)
#define NET_SUCCESS						(3)
#define NET_DEBUG						(4)

#define SZ_NET_CLIENT					"[Client]"
#define SZ_NET_SERVER					"[Server]"
#define SZ_NET_ERROR					"[Error]"
#define SZ_NET_SUCCESS					"[Success]"
#define SZ_NET_DEBUG					"[Debug]"

#define PACKETTYPE_CREATE_OBJECT		(0)
#define PACKETTYPE_DELETE_OBJECT		(1)
#define PACKETTYPE_MOVE_OBJECT			(2)
#define PACKETTYPE_OWNER_OBJECT			(3)
#define PACKETTYPE_TYPE_OBJECT			(4)
#define	PACKETTYPE_MODEL_OBJECT			(5)
#define PACKETTYPE_ROTATE_OBJECT		(6)
#define PACKETTYPE_SWAP_TRIBE			(7)
#define PACKETTYPE_NEW_MAP_ASK			(8)
#define PACKETTYPE_NEW_MAP				(9)
#define PACKETTYPE_LAND_MODIFY			(10)
#define PACKETTYPE_UPDATE_VIEW			(11)
#define PACKETTYPE_AVAILABLE_DISC		(12)
#define PACKETTYPE_SPELL_RESTRICTIONS	(13)
#define PACKETTYPE_ALLIES				(14)
#define PACKETTYPE_NUM_PLAYERS			(15)
#define	PACKETTYPE_FOG_OF_WAR			(16)
#define PACKETTYPE_OMNIPRESENCE			(17)
#define	PACKETTYPE_NO_GUEST				(18)
#define	PACKETTYPE_NO_REINCARNATE_TIME	(19)
#define PACKETTYPE_COMPUTER_PLAYER_AI	(20)
#define	PACKETTYPE_OBJECT_BANK			(21)
#define PACKETTYPE_MAP_TYPE				(22)
#define PACKETTYPE_CREATE_MARKER		(23)
#define PACKETTYPE_MOVE_MARKER			(24)
#define PACKETTYPE_DELETE_MARKER		(25)
#define PACKETTYPE_TRIGGER_OPTIONS		(26)
#define PACKETTYPE_DISCOVERY_OPTIONS	(27)
#define PACKETTYPEMSG_DISCOVERY_MANA	(28)
#define PACKETTYPESER_SYNC_MAP			(29)
#define PACKETTYPE_SYNC_MAP_ASK			(30)
#define	PACKETTYPE_SYNC_MAP				(31)
#define PACKETTYPE_SYNC_MAP_UPDATE_USER	(32)
#define PACKETTYPE_SYNC_OBJECTS			(33)
#define PACKETTYPEMSG_SYNC_MISC			(34)
#define PACKETTYPE_TRIGGER_LINK			(35)
#define PACKETTYPE_DELETE_ALL_OBJECTS   (36)
#define PACKETTYPE_DELETE_ALL_MARKERS   (37)

struct Packet
{
	Packet()
	{
	
		
	}

	void del() 
	{
		delete this;
	}

	WORD wType;
	WORD wData[13];
};

struct LandPacket
{
	LandPacket()
	{
		clear();
	}

	WORD wType;
	WORD wData[LAND_PACKETS_BUFFER];
	void clear() { memset(this->wData, 0, sizeof(this->wData)); }
};


class Network 
{
public:
	Network(): m_bIsHost { false }, m_bENetInitialized { false }, 
		m_bConnected { false }, m_nConnections { 0 }, m_bNewMap { false }, 
		m_LandPackets { 0 }, m_nRecvWaitCount { 0 }, m_szIP { 0 }, m_nPort { 0 }
    {

    }
	void InitNetwork(bool bHosting);
	void SendPacket(WORD* p, size_t nSize);
	void SendPacket(const char* szMsg);
	void SendPacket(Packet *p, size_t nSize = sizeof(struct Packet));
	void SendPacket(LandPacket *p, size_t nSize = sizeof(struct LandPacket));
	bool IsHost(void) { return this->m_bIsHost; }
	bool IsInitialized(void) { return this->m_bENetInitialized; }
	bool IsConnected(void) { return this->m_bConnected; }
	BYTE GetConnections(void) { return this->m_nConnections; }
	void Connected(void) { this->m_nConnections++; this->m_bConnected = this->m_nConnections; }
	void Disconnected(void) { this->m_nConnections--; this->m_bConnected = this->m_nConnections; }
	bool GetNewMap() { return this->m_bNewMap; }
	void SetNewMap(bool accept) { this->m_bNewMap = accept; }
	void OnSyncMapAsk();
	int m_LandPackets;
private:
	template<class... Args>
	void Log(BYTE nLogType, Args... args);

	void InitConsole(void);
	bool ProcessHost(void);
	bool ProcessClient(void);
	bool m_bIsHost;
	bool m_bENetInitialized;
	bool m_bConnected;
	BYTE m_nConnections;
	std::thread m_thProcess;
	bool m_bNewMap;
	BYTE m_nRecvWaitCount;
	BYTE m_nSerializePacketType;
	char m_szIP[255];
	int m_nPort;

	void OnPacket(ENetEvent &event, bool bMsg = false, SBYTE nPacketType = -1);
	THING* OnCreateObject(BYTE nType, BYTE nModel, BYTE nOwner, WORD x, WORD z);
	void OnObjectDelete(WORD wThingIdx);
	void OnObjectMove(WORD wThingIdx, WORD x, WORD z, bool bIsLandbridge);
	void OnObjectOwner(WORD wThingIdx, BYTE nOwner);
	void OnObjectType(WORD wThingIdx, BYTE nType, BYTE nModel);
	void OnObjectModel(WORD wThingIdx, BYTE nModel, BYTE nIslandNum);
	void OnSwapTribe(BYTE t1, BYTE t2, BYTE t3, BYTE t4, bool f);
	void OnObjectRotate(WORD wThingIdx, BYTE nType, WORD wAngle);
	void OnEngineSetGroundHeight(int x, WORD h, int nNewPacket);
	void OnEngineUpdateView(void);
	void OnNewMapAsk(void);
	void OnNewMap(void);
	void OnAvailableDiscoveries(WPARAM wParam);
	void OnSpellRestrictions(WPARAM wParam);
	void OnAllies(WPARAM wParam);
	void OnNumPlayers(int nPlayers);
	void OnFogOfWar(bool bFog);
	void OnOmnipresence(bool bOmnipresence);
	void OnNoGuest(bool bGuestSpells);
	void OnNoReincarnationTime(bool bReincarnationTime);
	void OnComputerPlayerIndex(BYTE nTribe, int nScriptIndex);
	void OnObjectBank(BYTE nBank);
	void OnMapType(BYTE nBank);
	void OnDeleteMarker(int nMarker);
	void OnMarkerMove(int nMarker, WORD x, WORD z);
	void OnCreateMarker(int nMarker, WORD x, WORD z);
	void OnTriggerOptions(WORD wThingIdx, BYTE nType, BYTE nCellRadius, SBYTE nOccurences, WORD wTriggerCount, WORD wPrayTime, WORD wInactiveTime, bool bStartInactive, bool bCreateOwned);
	void OnDiscoveryOptions(WORD wThingIdx, BYTE nAvailability, BYTE nType, BYTE nModel);
	void OnDiscoveryMana(WORD wThingIdx, int nManaAmount);
	void OnPrepareUpdateMap(BYTE nRecvWaitCount, BYTE nSerializePacketType );
	void OnSyncMap(WORD* wLandData);
	void OnSyncUpdateUser(void);
	void OnSyncObjects(BYTE nType, BYTE nModel, BYTE nOwner, WORD x, WORD z, WORD wData5, WORD wData6, WORD wData7, WORD wData8, WORD wData9, WORD wData10, WORD wData11, SBYTE wData12);
	void OnSyncMisc(ULONG dwAvailableSpells, ULONG dwBuildingsAvailable, UBYTE nAllies0, UBYTE nAllies1, UBYTE nAllies2, UBYTE nAllies3, UBYTE nNumPlayers, UBYTE nLevelFlags, UBYTE nCP0, UBYTE nCP1, UBYTE nCP2, UBYTE nObjBank, UBYTE nMapBank, DWORD dwSpellsRestrictions);
	void OnTriggerLink(bool link, WORD wSourceIdx, WORD wTargetIdx, BYTE nSlot);
	void OnRemoveAllObjects(void);
	void OnRemoveAllMarkers(void);
}; 

extern Network net;