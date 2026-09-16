// Copyright Epic Games, Inc. All Rights Reserved.

#include "NamelessTalGameMode.h"
#include "MCLOG.h"

#include "NPC/NTNPCCharacterBase.h"
#include "Kismet/GameplayStatics.h"

#include "MediaPlayer.h"
#include "MediaSoundComponent.h"
#include "GameFramework/PlayerController.h"
#include "Player/UI/NTCutsceneWidget.h"

ANamelessTalGameMode::ANamelessTalGameMode()
{
	// stub
	CutsceneSound = CreateDefaultSubobject<UMediaSoundComponent>(TEXT("CutsceneSound"));
}

void ANamelessTalGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (CutsceneSound && CutsceneMediaPlayer)
	{
		CutsceneSound->SetMediaPlayer(CutsceneMediaPlayer);
	}

	if (CutsceneSource)
	{
		//PlayCutscene(CutsceneSource);
	}
}

void ANamelessTalGameMode::PlayCutscene(UMediaSource* Source)
{
	if (!CutsceneMediaPlayer || !Source || !CutsceneWidgetClass)
	{
		MC_LOG(LogMC, Warning, TEXT("PlayCutscene missing MediaPlayer/Source/WidgetClass"));
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return;
	}

	if (!ActiveCutsceneWidget)
	{
		ActiveCutsceneWidget = CreateWidget<UNTCutsceneWidget>(PC, CutsceneWidgetClass);
		ActiveCutsceneWidget->OnSkipRequested.AddUObject(this, &ANamelessTalGameMode::EndCutscene);
	}

	ActiveCutsceneWidget->AddToViewport(100);

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(ActiveCutsceneWidget->TakeWidget());
	PC->SetInputMode(InputMode);
	PC->SetShowMouseCursor(false);

	CutsceneMediaPlayer->OnEndReached.AddUniqueDynamic(this, &ANamelessTalGameMode::EndCutscene);
	CutsceneMediaPlayer->OpenSource(Source);
	CutsceneMediaPlayer->Play();
}

void ANamelessTalGameMode::EndCutscene()
{
	CutsceneMediaPlayer->Close();
	CutsceneMediaPlayer->OnEndReached.RemoveDynamic(this, &ANamelessTalGameMode::EndCutscene);

	if (ActiveCutsceneWidget)
	{
		ActiveCutsceneWidget->RemoveFromParent();
	}

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(true);
	}
}

void ANamelessTalGameMode::RespawnAllMonsters()
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	TArray<AActor*> Monsters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANTNPCCharacterBase::StaticClass(), Monsters);

	for (AActor* Monster : Monsters)
	{
		Monster->Destroy();
	}

	OnMonsterRespawn.Broadcast(CurrentRespawnPoint);
}
