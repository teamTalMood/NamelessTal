// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NTTitlePlayerController.generated.h"

class UNTTitleWidget;

UCLASS()
class NAMELESSTAL_API ANTTitlePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UNTTitleWidget> TitleWidgetClass;

	UPROPERTY()
	TObjectPtr<UNTTitleWidget> TitleWidget;
};
