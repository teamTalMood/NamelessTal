// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/NTGameDataSingleton.h"
#include "MCLOG.h"
#include "Engine/DataTable.h"

UNTGameDataSingleton::UNTGameDataSingleton()
{

}

void UNTGameDataSingleton::LoadGameDataTables()
{
	if (bTablesLoaded)
	{
		return;
	}
	bTablesLoaded = true;

	// NPCAttackPatternTable
	if (const UDataTable* DataTable = NPCAttackPatternTableAsset.LoadSynchronous())
	{
		for (const auto& RowPair : DataTable->GetRowMap())
		{
			NPCAttackPatternTable.Add(RowPair.Key, *reinterpret_cast<FNTNPCAttackPatternData*>(RowPair.Value));
		}
	}

	ensure(NPCAttackPatternTable.Num() > 0);

	// NPCAttackPatternStepTable
	if (const UDataTable* DataTable = NPCAttackPatternStepTableAsset.LoadSynchronous())
	{
		for (const auto& RowPair : DataTable->GetRowMap())
		{
			NPCAttackPatternStepTable.Add(RowPair.Key, *reinterpret_cast<FNTNPCAttackPatternStepData*>(RowPair.Value));
		}
	}

	ensure(NPCAttackPatternStepTable.Num() > 0);

	// PlayerSpecialArtTable
	if (const UDataTable* DataTable = PlayerSpecialArtTableAsset.LoadSynchronous())
	{
		for (const auto& RowPair : DataTable->GetRowMap())
		{
			PlayerSpecialArtTable.Add(RowPair.Key, *reinterpret_cast<FNTPlayerSpecialArtData*>(RowPair.Value));
		}
	}

	ensure(PlayerSpecialArtTable.Num() > 0);

	// PlayerSpecialArtStepTable
	if (const UDataTable* DataTable = PlayerSpecialArtStepTableAsset.LoadSynchronous())
	{
		for (const auto& RowPair : DataTable->GetRowMap())
		{
			PlayerSpecialArtStepTable.Add(RowPair.Key, *reinterpret_cast<FNTPlayerSpecialArtStepData*>(RowPair.Value));
		}
	}

	ensure(PlayerSpecialArtStepTable.Num() > 0);
}

TArray<FNTNPCAttackPatternData> UNTGameDataSingleton::FindNPCAttackPatternsByUser(FName InPatternUser) const
{
	TArray<FNTNPCAttackPatternData> Result;

	for (const auto& RowPair : NPCAttackPatternTable)
	{
		if (RowPair.Value.PatternUser == InPatternUser)
		{
			Result.Add(RowPair.Value);
		}
	}

	return Result;
}

UNTGameDataSingleton& UNTGameDataSingleton::Get()
{
	UNTGameDataSingleton* Singleton = CastChecked<UNTGameDataSingleton>(GEngine->GameSingleton);
	if (Singleton)
	{
		Singleton->LoadGameDataTables();
		return *Singleton;
	}

	MC_LOG(LogMC, Error, TEXT("Invalid Game Singleton"));
	return *NewObject<UNTGameDataSingleton>();
}

