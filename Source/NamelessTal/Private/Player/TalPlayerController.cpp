// Copyright Epic Games, Inc. All Rights Reserved.


#include "Player/TalPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Player/UI/NTPlayerHUDWidget.h"
#include "Player/TalPlayerCharacter.h"
#include "Stat/StatComponent.h"
#include "Player/UI/NTMessageWidget.h"
#include "MCLOG.h"


void ATalPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (!IsLocalController()) return;

	// 메시지 위젯은 숨긴채로 미리 만들어두기!
	if (NoticeWidgetClass)
	{
		NoticeWidget = CreateWidget<UNTMessageWidget>(this, NoticeWidgetClass);
		if (IsValid(NoticeWidget))
		{
			NoticeWidget->AddToViewport();

			NoticeWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}


	if (TutorialWidgetClass) {
		TutorialWidget = CreateWidget<UNTMessageWidget>(this,
			TutorialWidgetClass);
		if (IsValid(TutorialWidget))
		{
			TutorialWidget->AddToViewport();

			TutorialWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

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
	PlayerHUD->InitializeWithTalComponent(TalCharacter->GetTalComponent());

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

void ATalPlayerController::ShowMessage(ENTMessageStyle Style, const FText& Title, const FText& Body, float Duration)
{
	if (PlayerHUD == nullptr)
	{
		return;
	}

	PlayerHUD->ShowWidgetMessage(Title, Body, Duration);
}
