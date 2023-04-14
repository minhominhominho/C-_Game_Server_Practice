#include "pch.h"
#include "ServerPacketHandler.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

// ������ �۾��ڰ� ���� �۾��ϴ� ����

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	// TODO : Log
	return false;
}

bool Handle_S_LOGIN(PacketSessionRef& session, Protocol::S_LOGIN& pkt)
{
	if (pkt.success() == false)
		return true;

	if (pkt.players().size() == 0)
	{
		// ĳ���� ����â
	}

	// ���� UI ��ư ������ ���� ����
	Protocol::C_ENTER_GAME enterGamePkt;
	enterGamePkt.set_playerindex(0); // ù��° ĳ���ͷ� �����Ѵٰ� ����
	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(enterGamePkt);
	session->Send(sendBuffer);

	return true;
}

bool Handle_S_ENTER_GAME(PacketSessionRef& session, Protocol::S_ENTER_GAME& pkt)
{
	// TODO 
	return true;
}

bool Handle_S_CHAT(PacketSessionRef& session, Protocol::S_CHAT& pkt)
{
	std::cout << "Client: " << pkt.msg() << endl;
	return true;
}
