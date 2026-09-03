// Fill out your copyright notice in the Description page of Project Settings.


#include "Stat/StatComponent.h"




// Sets default values for this component's properties
UStatComponent::UStatComponent()
{
	// Tick disabled by default
	PrimaryComponentTick.bCanEverTick = false;
}

void UStatComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHP = MaxHP;
}

void UStatComponent::ApplyDamage(float HitDamage)
{

	CurrentHP = FMath::Clamp(CurrentHP - HitDamage, 0.0f, MaxHP);
}

void UStatComponent::HealHp(float HealAmount)
{
	CurrentHP = FMath::Clamp(CurrentHP + HealAmount, 0.0f, MaxHP);
}


void UStatComponent::RestoreFull()
// 모닥불 처리
{
	CurrentHP = MaxHP;
}

