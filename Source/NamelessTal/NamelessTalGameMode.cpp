// Copyright Epic Games, Inc. All Rights Reserved.

#include "NamelessTalGameMode.h"
#include "MCLOG.h"

#include "NPC/NTNPCCharacterBase.h"
#include "Kismet/GameplayStatics.h"

#include "MediaPlayer.h"
#include "MediaSoundComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/UI/NTCutsceneWidget.h"
#include "Player/TalPlayerController.h"
#include "Components/AudioComponent.h"

ANamelessTalGameMode::ANamelessTalGameMode()
{
	// stub
	CutsceneSound = CreateDefaultSubobject<UMediaSoundComponent>(TEXT("CutsceneSound"));

	BGMAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BGMAudioComponent"));
	BGMAudioComponent->bAutoActivate = false;

	CutsceneSources.Add(EBossPhase::First, nullptr);
	CutsceneSources.Add(EBossPhase::Second, nullptr);
	CutsceneSources.Add(EBossPhase::Third, nullptr);
	CutsceneSources.Add(EBossPhase::End, nullptr);
}

void ANamelessTalGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	if (CutsceneSound && CutsceneMediaPlayer)
	{
		CutsceneSound->SetMediaPlayer(CutsceneMediaPlayer);
	}

	if (BGMAudioComponent)
	{
		BGMAudioComponent->Play();
	}
}

void ANamelessTalGameMode::PlayCutscene(UMediaSource* Source)
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	if (!CutsceneMediaPlayer || !Source || !CutsceneWidgetClass)
	{
		MC_LOG(LogMC, Warning, TEXT("PlayCutscene missing MediaPlayer/Source/WidgetClass"));
		return;
	}

	if (BGMAudioComponent)
	{
		BGMAudioComponent->Stop();
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return;
	}

	PC->FlushPressedKeys();

	if (ACharacter* PlayerCharacter = Cast<ACharacter>(PC->GetPawn()))
	{
		PlayerCharacter->GetCharacterMovement()->StopMovementImmediately();
		PlayerCharacter->GetCharacterMovement()->DisableMovement();
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
	//MC_LOG(LogMC, Log, TEXT("Begin"));

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

			bBossRoomEntered = true;
			CurrentRespawnPoint = ENTRespawnPointEnum::Boss;
			bIsBossStartSequence = true;
			break;
		}
		case EBossPhase::Second:
			break;
		case EBossPhase::Third:
			break;
		case EBossPhase::End:
			bIsEnding = true;
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
	if (bIsEnding)
	{
		if (!TitleLevelName.IsNone())
		{
			UGameplayStatics::OpenLevel(this, TitleLevelName);
		}
		else
		{
			MC_LOG(LogMC, Warning, TEXT("TitleLevelName not set"));
		}
	}

	CutsceneMediaPlayer->Close();
	CutsceneMediaPlayer->OnEndReached.RemoveDynamic(this, &ANamelessTalGameMode::EndCutscene);

	if (ActiveCutsceneWidget)
	{
		ActiveCutsceneWidget->RemoveFromParent();
	}

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		if (ACharacter* PlayerCharacter = Cast<ACharacter>(PC->GetPawn()))
		{
			PlayerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		}

		PC->FlushPressedKeys();
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
	}

	OnCutsceneSequenceEnded.Broadcast();

	if (bIsBossStartSequence)
	{
		OnMonsterRespawn.Broadcast(ENTRespawnPointEnum::Boss);
		bIsBossStartSequence = false;

		if (BGMAudioComponent && BossBGM)
		{
			BGMAudioComponent->SetSound(BossBGM);
		}

		OnStartBossGame.ExecuteIfBound();
	}

	if (BGMAudioComponent)
	{
		BGMAudioComponent->Play();
	}
}

void ANamelessTalGameMode::NotifyBossHPChanged(float CurrentHP, float MaxHP)
{
	if (ATalPlayerController* PC = Cast<ATalPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		PC->UpdateBossHP(CurrentHP, MaxHP);
	}
}

void ANamelessTalGameMode::NotifyBossSoulChanged(float CurrentSoul, float MaxSoul)
{
	if (ATalPlayerController* PC = Cast<ATalPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
	{
		PC->UpdateBossSoul(CurrentSoul, MaxSoul);
	}
}

void ANamelessTalGameMode::RespawnAllMonsters()
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	TArray<AActor*> Monsters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANTNPCCharacterBase::StaticClass(), Monsters);

	for (AActor* Monster : Monsters)
	{
		Monster->Destroy();
	}

	OnMonsterRespawn.Broadcast(CurrentRespawnPoint);
}
