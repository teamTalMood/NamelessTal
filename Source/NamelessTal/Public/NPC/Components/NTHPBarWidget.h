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
	FORCEINLINE void SetMaxHp(float NewMaxHp) { MaxHp = NewMaxHp; }

	UFUNCTION()
	void UpdateHpBar(float NewCurrentHp, float MaxHP);

	FORCEINLINE void SetOwiningActor(AActor* NewOwner) { OwningActor = NewOwner; }

protected:
	UPROPERTY()
	TObjectPtr<class UProgressBar> HpProgressBar;

	UPROPERTY()
	float MaxHp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Actor")
	TObjectPtr<AActor> OwningActor;
};
