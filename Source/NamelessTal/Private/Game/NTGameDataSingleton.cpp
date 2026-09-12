// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/NTGameDataSingleton.h"
#include "MCLOG.h"
#include "Engine/DataTable.h"
#include "UObject/ConstructorHelpers.h"

UNTGameDataSingleton::UNTGameDataSingleton()
{
	// NPCAttackPatternTable
	static ConstructorHelpers::FObjectFinder<UDataTable> NPCAttackPatternTableRef(TEXT("/Script/Engine.DataTable'/Game/GameData/DT_NTNPCAttackPattern.DT_NTNPCAttackPattern'"));
	if (nullptr != NPCAttackPatternTableRef.Object)
	{
		const UDataTable* DataTable = NPCAttackPatternTableRef.Object;
		ensure(DataTable->GetRowMap().Num() > 0);

		for (const auto& RowPair : DataTable->GetRowMap())
		{
			NPCAttackPatternTable.Add(RowPair.Key, *reinterpret_cast<FNTNPCAttackPatternData*>(RowPair.Value));
		}
	}

	ensure(NPCAttackPatternTable.Num() > 0);

	// NPCAttackPatternStepTable
	static ConstructorHelpers::FObjectFinder<UDataTable> NPCAttackPatternStepTableRef(TEXT("/Script/Engine.DataTable'/Game/GameData/DT_NTNPCAttackPatternStep.DT_NTNPCAttackPatternStep'"));
	if (nullptr != NPCAttackPatternStepTableRef.Object)
	{
		const UDataTable* DataTable = NPCAttackPatternStepTableRef.Object;
		ensure(DataTable->GetRowMap().Num() > 0);

		for (const auto& RowPair : DataTable->GetRowMap())
		{
			NPCAttackPatternStepTable.Add(RowPair.Key, *reinterpret_cast<FNTNPCAttackPatternStepData*>(RowPair.Value));
		}
	}

	ensure(NPCAttackPatternStepTable.Num() > 0);

	// PlayerSpecialArtTable
	static ConstructorHelpers::FObjectFinder<UDataTable> PlayerSpecialArtTableRef(TEXT("/Script/Engine.DataTable'/Game/GameData/DT_NTPlayerSpecialArt.DT_NTPlayerSpecialArt'"));
	if (nullptr != PlayerSpecialArtTableRef.Object)
	{
		const UDataTable* DataTable = PlayerSpecialArtTableRef.Object;
		ensure(DataTable->GetRowMap().Num() > 0);

		for (const auto& RowPair : DataTable->GetRowMap())
		{
			PlayerSpecialArtTable.Add(RowPair.Key, *reinterpret_cast<FNTPlayerSpecialArtData*>(RowPair.Value));
		}
	}

	ensure(PlayerSpecialArtTable.Num() > 0);

	// PlayerSpecialArtStepTable
	static ConstructorHelpers::FObjectFinder<UDataTable> PlayerSpecialArtStepTableRef(TEXT("/Script/Engine.DataTable'/Game/GameData/DT_NTPlayerSpecialArtStep.DT_NTPlayerSpecialArtStep'"));
	if (nullptr != PlayerSpecialArtStepTableRef.Object)
	{
		const UDataTable* DataTable = PlayerSpecialArtStepTableRef.Object;
		ensure(DataTable->GetRowMap().Num() > 0);

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
		return *Singleton;
	}

	MC_LOG(LogMC, Error, TEXT("Invalid Game Singleton"));
	return *NewObject<UNTGameDataSingleton>();
}

