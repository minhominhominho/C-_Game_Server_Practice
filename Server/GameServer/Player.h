#pragma once

class Player
{
public:
	uint64					playerId = 0;
	string					name;
	Protocol::PlayerType	type = Protocol::Player_TYPE_NONE;
	GameSessionRef			ownerSession; // Cycle ������ �ֱ� ��. �ϴ� �Ѿ
};

