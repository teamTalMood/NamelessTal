// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NTMonsterSpawner.h"
#include "NPC/NTNPCCharacterBase.h"
#include "DdoKaebiFire/DdokaebiFire.h"
#include "Kismet/GameplayStatics.h"


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

	TArray<AActor*> Fires;

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADdokaebiFire::StaticClass(), Fires);

	for (AActor* Fire : Fires)
	{
		if (ADdokaebiFire* Bonfire = Cast<ADdokaebiFire>(Fire))
		{
			Bonfire->OnBonfireRested.AddDynamic(this, &ANTMonsterSpawner::RespawnMissing);
		}
	}

	SpawnAll();
	
}

void ANTMonsterSpawner::SpawnAll()
{
	for (FMonsterSpawnEntry& Entry : SpawnEntries)
	{
		Entry.SpawnedMonster = GetWorld()->SpawnActor<ANTNPCCharacterBase>(Entry.MonsterClass,
			Entry.SpawnTransform);
	}
}

void ANTMonsterSpawner::RespawnMissing()
{
	for (FMonsterSpawnEntry& Entry : SpawnEntries)
	{
		if (Entry.SpawnedMonster.IsValid())
		{
			Entry.SpawnedMonster->Destroy();
		}
		Entry.SpawnedMonster = GetWorld()->SpawnActor<ANTNPCCharacterBase>(Entry.MonsterClass,
			Entry.SpawnTransform);
	}
}

// Called every frame
void ANTMonsterSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

