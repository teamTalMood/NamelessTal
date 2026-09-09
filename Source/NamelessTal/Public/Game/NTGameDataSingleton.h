// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NPC/NTNPCAttackPatternData.h"
#include "NTGameDataSingleton.generated.h"

/**
 *
 */
UCLASS()
class NAMELESSTAL_API UNTGameDataSingleton : public UObject
{
	GENERATED_BODY()

public:
	UNTGameDataSingleton();
	static UNTGameDataSingleton& Get();

// NPC Attack Pattern Data Section
public:
	FORCEINLINE const FNTNPCAttackPatternData* GetNPCAttackPattern(FName InPatternID) const { return NPCAttackPatternTable.Find(InPatternID); }

private:
	TMap<FName, FNTNPCAttackPatternData> NPCAttackPatternTable;
};
