// Copyright Epic Games, Inc. All Rights Reserved.


#include "Player/TalPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Player/UI/NTPlayerHUDWidget.h"
#include "Player/TalPlayerCharacter.h"
#include "Stat/StatComponent.h"
#include "Player/UI/NTMessageWidget.h"
#include "Player/UI/NTAreaNoticeWidget.h"
#include "NamelessTalGameMode.h"
#include "MCLOG.h"
#include "EnhancedInputComponent.h"
#include "Player/UI/NTPauseMenuWidget.h"
#include "Engine/Texture2D.h"


void ATalPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//MC_LOG(LogMC, Log, TEXT("Begin"));

	SetControllerSettings(nullptr);
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

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (PauseAction)
		{
			EIC->BindAction(PauseAction, ETriggerEvent::Started,
				this, &ATalPlayerController::TogglePauseMenu);
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

void ATalPlayerController::ShowAreaNotice(const FText& AreaName, const FText& Description, UTexture2D* Icon)
{
	if (!IsValid(AreaNoticeWidget))
	{
		return;
	}

	AreaNoticeWidget->ShowArea(AreaName, Description, Icon);
}

void ATalPlayerController::HideAreaNotice()
{
	if (!IsValid(AreaNoticeWidget))
	{
		return;
	}

	AreaNoticeWidget->HideArea();
}
void ATalPlayerController::SetControllerSettings(ATalPlayerCharacter* InPawn)
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	if (!IsLocalController()) return;

	if (bISControllerSetted) return;

	SetInputMode(FInputModeGameOnly());
	SetShowMouseCursor(false);

	// 지역 알림 위젯은 숨긴채로 미리 만들어두기!
	if (AreaNoticeWidgetClass)
	{
		AreaNoticeWidget = CreateWidget<UNTAreaNoticeWidget>(this, AreaNoticeWidgetClass);
		if (IsValid(AreaNoticeWidget))
		{
			AreaNoticeWidget->AddToViewport();

			AreaNoticeWidget->SetVisibility(ESlateVisibility::Collapsed);
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

	ATalPlayerCharacter* TalCharacter = InPawn ? InPawn : Cast<ATalPlayerCharacter>(GetPawn());
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
	PlayerHUD->InitializeWithPlayerCharacter(TalCharacter);

	if (ANamelessTalGameMode* GameMode = Cast<ANamelessTalGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->OnStartBossGame.BindUObject(this, &ATalPlayerController::HandleStartBossGame);
		MC_LOG(LogMC, Log, TEXT("OnStartBossGame Binded"));
	}

	bISControllerSetted = true;
}

void ATalPlayerController::HandleStartBossGame()
{
	if (!IsValid(PlayerHUD)) return;

	MC_LOG(LogMC, Log, TEXT("Begin"));

	PlayerHUD->ShowBossHPBar();
}

void ATalPlayerController::UpdateBossHP(float CurrentHP, float MaxHP)
{
	if (!IsValid(PlayerHUD)) return;

	PlayerHUD->UpdateBossHPBar(CurrentHP, MaxHP);
}

void ATalPlayerController::UpdateBossSoul(float CurrentSoul, float MaxSoul)
{
	if (!IsValid(PlayerHUD)) return;

	PlayerHUD->UpdateBossSoulBar(CurrentSoul, MaxSoul);
}

void ATalPlayerController::TogglePauseMenu()
{
	if (PauseMenu && PauseMenu->IsInViewport())
	{
		PauseMenu->CloseMenu();
		return;
	}

	if (!PauseMenu)
	{
		if (!PauseMenuClass)
		{
			return;
		}
		PauseMenu = CreateWidget<UNTPauseMenuWidget>(this, PauseMenuClass);
	}

	if (PauseMenu)
	{
		PauseMenu->OpenMenu();
	}
}

