// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NTHPBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class NAMELESSTAL_API UNTHPBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UNTHPBarWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

public:
	UFUNCTION()
	void UpdateHpBar(float NewCurrentHp, float InMaxHP);

	UFUNCTION()
	void UpdateSoulBar(float NewCurrentSoul, float InMaxSoul);

	FORCEINLINE void SetOwiningActor(AActor* NewOwner) { OwningActor = NewOwner; }

protected:
	UPROPERTY()
	TObjectPtr<class UProgressBar> HpProgressBar;

	UPROPERTY()
	TObjectPtr<class UProgressBar> SoulProgressBar;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Actor")
	TObjectPtr<AActor> OwningActor;
};
