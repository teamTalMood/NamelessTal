// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Title/NTSavePointEnum.h"
#include "NamelessTalGameMode.generated.h"

class UMediaPlayer;
class UMediaSource;
class UMediaSoundComponent;
class UNTCutsceneWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterRespawn, ENTRespawnPointEnum, OnMonsterRespawn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCutsceneSequenceEnded);
DECLARE_DELEGATE(FOnStartBossGame);

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class ANamelessTalGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	/** Constructor */
	ANamelessTalGameMode();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnMonsterRespawn OnMonsterRespawn;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnCutsceneSequenceEnded OnCutsceneSequenceEnded;

	// 보스방 입장 시퀀스 종료 시 호출. ATalPlayerController에서 바인딩
	FOnStartBossGame OnStartBossGame;

	void RespawnAllMonsters();

	// 보스 HP 변경을 ATalPlayerController로 전달
	void NotifyBossHPChanged(float CurrentHP, float MaxHP);

	// 보스 넋 변경을 ATalPlayerController로 전달
	void NotifyBossSoulChanged(float CurrentSoul, float MaxSoul);

	void SetCurrentSavePoint(ENTRespawnPointEnum InCurrentSavePoint) { CurrentRespawnPoint = InCurrentSavePoint; }
	const ENTRespawnPointEnum GetCurrentSavePoint() { return CurrentRespawnPoint; }

	void SetIsEpicKilled(uint8 InIsEpicKilled) { bIsEpicKilled = InIsEpicKilled; }
	const uint8 GetIsEpicKilled() { return bIsEpicKilled; }

	void SetBossRoomEntered(uint8 InBossRoomEntered) { bBossRoomEntered = InBossRoomEntered; }
	const uint8 GetBossRoomEntered() { return bBossRoomEntered; }

	/** 렌더된 시퀀스 영상을 소리와 함께 전체화면 재생한다 */
	void PlayCutscene(UMediaSource* Source);

	void BossPhaseStarted(EBossPhase InPhase);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void EndCutscene();

protected:
	ENTRespawnPointEnum CurrentRespawnPoint = ENTRespawnPointEnum::StartPoint;

	uint8 bIsEpicKilled : 1 = false;
	uint8 bBossRoomEntered : 1 = false;
	uint8 bIsBossStartSequence : 1 = false;
	uint8 bIsEnding : 1 = false;

	UPROPERTY(EditDefaultsOnly, Category = "Cutscene")
	TObjectPtr<UMediaPlayer> CutsceneMediaPlayer;

	UPROPERTY(VisibleAnywhere, Category = "Cutscene")
	TObjectPtr<UMediaSoundComponent> CutsceneSound;

	UPROPERTY(EditDefaultsOnly, Category = "Cutscene")
	TSubclassOf<UNTCutsceneWidget> CutsceneWidgetClass;

	UPROPERTY()
	TObjectPtr<UNTCutsceneWidget> ActiveCutsceneWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Cutscene")
	TMap<EBossPhase, TObjectPtr<UMediaSource>> CutsceneSources;

	UPROPERTY(EditDefaultsOnly, Category = "Ending")
	FName TitleLevelName;

};



