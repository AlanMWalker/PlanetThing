#include "LocalPlayerController.h"

#include <KApplication.h>

using namespace Krawler;

LocalPlayerController::LocalPlayerController()
	: KComponentBase(GET_APP()->getCurrentScene()->addEntityToScene())
{
	getEntity()->setTag(L"Player_Satellite");
}
