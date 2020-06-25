#include <KApplication.h>
#include "LocalPlayerController.hpp"

using namespace Krawler;

LocalPlayerController::LocalPlayerController()
	: KComponentBase(GET_APP()->getCurrentScene()->addEntityToScene())
{
	getEntity()->setTag(L"Player_Satellite");
}
