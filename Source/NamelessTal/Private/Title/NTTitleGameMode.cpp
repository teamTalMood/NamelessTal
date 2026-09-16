#include "Title/NTTitleGameMode.h"

#include "Title/NTTitlePlayerController.h"

ANTTitleGameMode::ANTTitleGameMode()
{
	PlayerControllerClass = ANTTitlePlayerController::StaticClass();
}
