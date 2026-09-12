// Copyright Epic Games, Inc. All Rights Reserved.


#include "Player/TalPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Player/UI/NTPlayerHUDWidget.h"
#include "Player/TalPlayerCharacter.h"
#include "Stat/StatComponent.h"


void ATalPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) return;
	if (!PlayerHUDClass) return;

	ATalPlayerCharacter* TalCharacter = Cast<ATalPlayerCharacter>(GetPawn());
	if (!IsValid(TalCharacter)) return;

	UStatComponent* StatComponent = TalCharacter->GetStatComponent();
	if (!IsValid(StatComponent)) return;

	PlayerHUD = CreateWidget<UNTPlayerHUDWidget>(
		this,
		PlayerHUDClass
	);
	if (!IsValid(PlayerHUD)) return;


	PlayerHUD->AddToViewport();
	PlayerHUD->InitializeWithStatComponent(StatComponent);

}

void ATalPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Add Input Mapping Contexts
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
		{
			Subsystem->AddMappingContext(CurrentContext, 0);
		}
	}
	
}

