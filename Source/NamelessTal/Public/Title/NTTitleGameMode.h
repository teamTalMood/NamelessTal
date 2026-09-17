// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NTTitleGameMode.generated.h"

class UAudioComponent;
class UWorld;

UCLASS()
class NAMELESSTAL_API ANTTitleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANTTitleGameMode();

	void StartGame();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Level")
	TSoftObjectPtr<UWorld> StartLevel;

	UPROPERTY(VisibleAnywhere, Category = "Sound")
	TObjectPtr<UAudioComponent> BGMAudioComponent;
};
