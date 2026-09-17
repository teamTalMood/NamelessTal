#include "Title/NTTitleGameMode.h"

#include "Title/NTTitlePlayerController.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MCLOG.h"

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

void ANTTitleGameMode::StartGame()
{
	if (StartLevel.IsNull())
	{
		MC_LOG(LogMC, Warning, TEXT("StartLevel not set"));
		return;
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, StartLevel);
}
