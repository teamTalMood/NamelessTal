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

	void RespawnAllMonsters();

	void SetCurrentSavePoint(ENTRespawnPointEnum InCurrentSavePoint) { CurrentRespawnPoint = InCurrentSavePoint; }
	const ENTRespawnPointEnum GetCurrentSavePoint() { return CurrentRespawnPoint; }

	void SetIsEpicKilled(uint8 InIsEpicKilled) { bIsEpicKilled = InIsEpicKilled; }
	const uint8 GetIsEpicKilled() { return bIsEpicKilled; }

	/** 렌더된 시퀀스 영상을 소리와 함께 전체화면 재생한다 */
	void PlayCutscene(UMediaSource* Source);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void EndCutscene();

protected:
	ENTRespawnPointEnum CurrentRespawnPoint = ENTRespawnPointEnum::StartPoint;

	uint8 bIsEpicKilled : 1 = false;

	UPROPERTY(EditDefaultsOnly, Category = "Cutscene")
	TObjectPtr<UMediaPlayer> CutsceneMediaPlayer;

	UPROPERTY(VisibleAnywhere, Category = "Cutscene")
	TObjectPtr<UMediaSoundComponent> CutsceneSound;

	UPROPERTY(EditDefaultsOnly, Category = "Cutscene")
	TSubclassOf<UNTCutsceneWidget> CutsceneWidgetClass;

	UPROPERTY()
	TObjectPtr<UNTCutsceneWidget> ActiveCutsceneWidget;

};



