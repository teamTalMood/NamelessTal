// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NPC/NTNPCAttackPatternData.h"
#include "NPC/NTNPCAttackPatternStepData.h"
#include "Player/NTPlayerSpecialArtData.h"
#include "Player/NTPlayerSpecialArtStepData.h"
#include "NTGameDataSingleton.generated.h"

class UDataTable;

/**
 *
 */
UCLASS(Blueprintable)
class NAMELESSTAL_API UNTGameDataSingleton : public UObject
{
	GENERATED_BODY()

public:
	UNTGameDataSingleton();
	static UNTGameDataSingleton& Get();

// NPC Attack Pattern Data Section
public:
	FORCEINLINE const FNTNPCAttackPatternData* GetNPCAttackPattern(FName InPatternID) const { return NPCAttackPatternTable.Find(InPatternID); }
	TArray<FNTNPCAttackPatternData> FindNPCAttackPatternsByUser(FName InPatternUser) const;
	FORCEINLINE const FNTNPCAttackPatternStepData* GetNPCAttackPatternStep(FName InStepID) const { return NPCAttackPatternStepTable.Find(InStepID); }
	FORCEINLINE const FNTPlayerSpecialArtData* GetPlayerSpecialArt(FName InSpecialArtID) const { return PlayerSpecialArtTable.Find(InSpecialArtID); }
	FORCEINLINE const FNTPlayerSpecialArtStepData* GetPlayerSpecialArtStep(FName InStepID) const { return PlayerSpecialArtStepTable.Find(InStepID); }
	const FNTPlayerSpecialArtStepData* FindPlayerSpecialArtStep(FName InSpecialArtID, int32 InComboCount) const;

	TArray<FName> FindPlayerSpecialArtIDsByWeapon(ETalStyle InWeaponStyle) const;
	
private:
	void LoadGameDataTables();

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> NPCAttackPatternTableAsset;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> NPCAttackPatternStepTableAsset;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> PlayerSpecialArtTableAsset;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> PlayerSpecialArtStepTableAsset;

	bool bTablesLoaded = false;

	TMap<FName, FNTNPCAttackPatternData> NPCAttackPatternTable;

	TMap<FName, FNTNPCAttackPatternStepData> NPCAttackPatternStepTable;

	TMap<FName, FNTPlayerSpecialArtData> PlayerSpecialArtTable;

	TMap<FName, FNTPlayerSpecialArtStepData> PlayerSpecialArtStepTable;
};
