// Copyright Epic Games, Inc. All Rights Reserved.

#include "NamelessTalGameMode.h"
#include "MCLOG.h"

#include "NPC/NTNPCCharacterBase.h"
#include "Kismet/GameplayStatics.h"

ANamelessTalGameMode::ANamelessTalGameMode()
{
	// stub
}

void ANamelessTalGameMode::RespawnAllMonsters()
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	TArray<AActor*> Monsters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANTNPCCharacterBase::StaticClass(), Monsters);

	for (AActor* Monster : Monsters)
	{
		Monster->Destroy();
	}

	OnMonsterRespawn.Broadcast(CurrentSavePoint);
}
