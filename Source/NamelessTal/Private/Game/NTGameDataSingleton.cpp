// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/NTGameDataSingleton.h"
#include "MCLOG.h"
#include "Engine/DataTable.h"
#include "UObject/ConstructorHelpers.h"

UNTGameDataSingleton::UNTGameDataSingleton()
{
	static ConstructorHelpers::FObjectFinder<UDataTable> NPCAttackPatternTableRef(TEXT("/Script/Engine.DataTable'/Game/GameData/DT_NTNPCAttackPattern.DT_NTNPCAttackPattern'"));
	if (nullptr != NPCAttackPatternTableRef.Object)
	{
		const UDataTable* DataTable = NPCAttackPatternTableRef.Object;
		check(DataTable->GetRowMap().Num() > 0);

		for (const auto& RowPair : DataTable->GetRowMap())
		{
			NPCAttackPatternTable.Add(RowPair.Key, *reinterpret_cast<FNTNPCAttackPatternData*>(RowPair.Value));
		}
	}

	ensure(NPCAttackPatternTable.Num() > 0);
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

