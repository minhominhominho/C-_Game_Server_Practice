#pragma once

class Player
{
public:
	uint64					playerId = 0;
	string					name;
	Protocol::PlayerType	type = Protocol::Player_TYPE_NONE;
	GameSessionRef			ownerSession; // Cycle 문제가 있긴 함. 일단 넘어감
};

