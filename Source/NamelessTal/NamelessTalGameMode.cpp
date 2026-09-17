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

	CutsceneSources.Add(EBossPhase::First, nullptr);
	CutsceneSources.Add(EBossPhase::Second, nullptr);
	CutsceneSources.Add(EBossPhase::Third, nullptr);
	CutsceneSources.Add(EBossPhase::End, nullptr);
}

void ANamelessTalGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	MC_LOG(LogMC, Log, TEXT("Begin"));

	if (CutsceneSound && CutsceneMediaPlayer)
	{
		CutsceneSound->SetMediaPlayer(CutsceneMediaPlayer);
	}
}

void ANamelessTalGameMode::PlayCutscene(UMediaSource* Source)
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

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

void ANamelessTalGameMode::BossPhaseStarted(EBossPhase InPhase)
{
	switch (InPhase)
	{
		case EBossPhase::First:
		{
			TArray<AActor*> Monsters;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANTNPCCharacterBase::StaticClass(), Monsters);

			for (AActor* Monster : Monsters)
			{
				Monster->Destroy();
			}
			break;
		}
		case EBossPhase::Second:
			break;
		case EBossPhase::Third:
			break;
		case EBossPhase::End:
			break;
		default:
			break;
	}

	if (CutsceneSources[InPhase])
	{
		PlayCutscene(CutsceneSources[InPhase]);
	}
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
		PC->SetShowMouseCursor(false);
	}

	OnCutsceneSequenceEnded.Broadcast();
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
