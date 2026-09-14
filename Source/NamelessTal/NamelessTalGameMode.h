// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NamelessTalGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMonsterRespawn, int32, SavePoint);

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

	void SetCurrentSavePoint(int32 InCurrentSavePoint) { CurrentSavePoint = InCurrentSavePoint; }
	const int32 GetCurrentSavePoint() { return CurrentSavePoint; }

protected:
	int32 CurrentSavePoint = 0;

};



