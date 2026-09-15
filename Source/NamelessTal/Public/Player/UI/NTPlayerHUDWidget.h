// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NTPlayerHUDWidget.generated.h"

class UProgressBar;
class UStatComponent;

UCLASS()
class NAMELESSTAL_API UNTPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeWithStatComponent(UStatComponent* InStatComponent);

	void ShowWidgetMessage(const FText& Title, const FText& Body, float Duration);

protected:
	virtual void NativeConstruct() override;

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> PB_HP;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> PB_Soul;

	UPROPERTY()
	TObjectPtr<class UNTMessageWidget> WDG_Message;

private:
	UPROPERTY()
	TObjectPtr<UStatComponent> StatComponent;

	UFUNCTION()
	void HandleHPChanged(float NewHP, float MaxHP);

	UFUNCTION()
	void HandleSoulChanged(float NewSoul, float MaxSoul);

	void RefreshAll();
};
