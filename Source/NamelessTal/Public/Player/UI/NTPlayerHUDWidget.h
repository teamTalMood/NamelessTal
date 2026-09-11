// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NTPlayerHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class NAMELESSTAL_API UNTPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> PB_HP;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> PB_Soul;
};
