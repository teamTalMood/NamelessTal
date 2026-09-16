// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Title/NTSavePointEnum.h"
#include "NamelessTalGameMode.generated.h"

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

protected:
	ENTRespawnPointEnum CurrentRespawnPoint = ENTRespawnPointEnum::StartPoint;

};



