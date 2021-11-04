

#include "../player/Player.h"
#include "../player/PlayerMgr.h"

#include "../log/Log.h"

extern PlayerMgr playerMgr;

void msgHandle(int messageId, int sessionId, const char* name, char* data, size_t lens)
{
	switch (messageId)
	{
	case 100:
	{
		break;
	}
	default:
		
	}
}

