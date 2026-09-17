// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NTTitleGameMode.generated.h"

class UAudioComponent;

UCLASS()
class NAMELESSTAL_API ANTTitleGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANTTitleGameMode();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Sound")
	TObjectPtr<UAudioComponent> BGMAudioComponent;
};
