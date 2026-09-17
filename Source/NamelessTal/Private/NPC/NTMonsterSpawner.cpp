// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NTMonsterSpawner.h"
#include "MCLOG.h"
#include "NPC/NTNPCCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Title/NTSavePointEnum.h"
#include "NamelessTalGameMode.h"


// Sets default values
ANTMonsterSpawner::ANTMonsterSpawner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

// Called when the game starts or when spawned
void ANTMonsterSpawner::BeginPlay()
{
	Super::BeginPlay();

	//게임모드에 플레이어 리스폰 델리게이트 연결
	if (ANamelessTalGameMode* GameMode = Cast<ANamelessTalGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->OnMonsterRespawn.AddDynamic(this, &ANTMonsterSpawner::SpawnNPC);
	}

	//SpawnNPC(StagePoint);
}

void ANTMonsterSpawner::SpawnNPC(ENTRespawnPointEnum InSavePoint)
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	if (MonsterClass == nullptr)
	{
		MC_LOG(LogMC, Error, TEXT("No MonsterClass!"));
		return;
	}

	FVector SpawnLocation = GetActorLocation() + SpawnOffset + FVector(0,0,100.f);
	GetWorld()->SpawnActor<ANTNPCCharacterBase>(MonsterClass, SpawnLocation, GetActorRotation());
}

// Called every frame
void ANTMonsterSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

