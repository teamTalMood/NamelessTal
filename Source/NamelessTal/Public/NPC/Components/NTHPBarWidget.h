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
	// 위젯 트리의 "PB_HP" / "PB_Soul"과 이름이 일치해야 컴파일 타임에 자동 바인딩됨
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> PB_HP;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> PB_Soul;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Actor")
	TObjectPtr<AActor> OwningActor;
};
