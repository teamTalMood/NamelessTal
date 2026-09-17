#include "Title/NTTitleGameMode.h"

#include "Title/NTTitlePlayerController.h"
#include "Components/AudioComponent.h"

ANTTitleGameMode::ANTTitleGameMode()
{
	PlayerControllerClass = ANTTitlePlayerController::StaticClass();

	BGMAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BGMAudioComponent"));
	BGMAudioComponent->bAutoActivate = false;
}

void ANTTitleGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (BGMAudioComponent)
	{
		BGMAudioComponent->Play();
	}
}
