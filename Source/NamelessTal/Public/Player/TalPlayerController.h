// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TalPlayerController.generated.h"


class UNTPlayerHUDWidget;

UCLASS()
class NAMELESSTAL_API ATalPlayerController : public APlayerController
{
	GENERATED_BODY()
	


protected:
	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<class UInputMappingContext*> DefaultMappingContexts;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UNTPlayerHUDWidget> PlayerHUDClass;
	UPROPERTY()
	TObjectPtr<UNTPlayerHUDWidget> PlayerHUD;
};
