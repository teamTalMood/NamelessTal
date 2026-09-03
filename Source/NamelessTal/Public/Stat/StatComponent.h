// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NAMELESSTAL_API UStatComponent : public UActorComponent
{
	GENERATED_BODY()



protected:
	virtual void BeginPlay() override;

public:	
	// Sets default values for this component's properties
	UStatComponent();

public:
	UPROPERTY(EditAnywhere, Category="Stat")
	float MaxHP = 100.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	float CurrentHP;

	void ApplyDamage(float HitDamage);
	void HealHp(float HealAmount);
	void RestoreFull(); // 화톳불용

	bool IsDead() const { return CurrentHP <= 0.0f; }
	

};
