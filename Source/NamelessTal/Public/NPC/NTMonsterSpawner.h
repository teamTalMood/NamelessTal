// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NTMonsterSpawner.generated.h"

class ANTNPCCharacterBase;
class ADdokaebiFire;

USTRUCT()
struct FMonsterSpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<ANTNPCCharacterBase> MonsterClass;

	UPROPERTY(EditAnywhere, meta = (MakeEditWidget = true))
	FTransform SpawnTransform;

	TWeakObjectPtr<ANTNPCCharacterBase> SpawnedMonster;

};


UCLASS()
class NAMELESSTAL_API ANTMonsterSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANTMonsterSpawner();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<FMonsterSpawnEntry> SpawnEntries;

	void SpawnAll();

	UFUNCTION()
	void RespawnMissing();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
