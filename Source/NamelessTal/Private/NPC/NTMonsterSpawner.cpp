// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NTMonsterSpawner.h"

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
	
}

void ANTMonsterSpawner::SpawnAll()
{
}

void ANTMonsterSpawner::RespawnMissing()
{
}

// Called every frame
void ANTMonsterSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

