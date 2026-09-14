// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NTMonsterSpawner.generated.h"

class ANTNPCCharacterBase;

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
	TSubclassOf<class ANTNPCCharacterBase> MonsterClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	FVector SpawnOffset;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int32 StagePoint;

	UFUNCTION()
	void SpawnNPC(int32 InSavePoint);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
