// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NTTitleWidget.generated.h"

class UButton;

UCLASS()
class NAMELESSTAL_API UNTTitleWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Start;

private:
	UFUNCTION()
	void HandleStartClicked();
};
