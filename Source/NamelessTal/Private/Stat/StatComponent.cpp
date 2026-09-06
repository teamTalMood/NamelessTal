// Fill out your copyright notice in the Description page of Project Settings.


#include "Stat/StatComponent.h"




// Sets default values for this component's properties
UStatComponent::UStatComponent()
{
	// 넋을 채우는데 쓰이는 Tick
	PrimaryComponentTick.bCanEverTick = true;
}

void UStatComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHP = MaxHP;
	CurrentSoul = MaxSoul;
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
	CurrentSoul = MaxSoul;
}



void UStatComponent::DrainSoul(float Amount)
{
	CurrentSoul = FMath::Clamp(CurrentSoul - Amount, 0.0f, MaxSoul);

	if (CurrentSoul <= 0.0f)
	{
		bIsSoulExhausted = true; // 고갈 진입
	}
	UE_LOG(LogTemp, Warning, TEXT("현재 넋 %f"), CurrentSoul);

	//언제 마지막으로 사용했는지 체크하기
	LastSoulDrainTime = GetWorld()->GetTimeSeconds();
}

void UStatComponent::SetSoulRegenBlocked(bool bBlocked)
{
	//막혀있다가 풀리는 순간
	if (bSoulRegenBlocked && !bBlocked)
	{
		LastSoulDrainTime = GetWorld()->GetTimeSeconds();
	}
	bSoulRegenBlocked = bBlocked;
}

// 시간이 지나면 (1초) 넋이 시간초에 따라 자동으로 찬다
void UStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, 
	FActorComponentTickFunction* PCSoulTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, PCSoulTickFunction);
	
	if (bSoulRegenBlocked)
	{
		return;
	}

	const float TimeSinceLastDrain = GetWorld()->GetTimeSeconds() - LastSoulDrainTime;
	if (TimeSinceLastDrain < SoulRegenDelay)
	{
		return;
	}

	CurrentSoul = FMath::Clamp(CurrentSoul + GetSoulRegenRateForCurrentHP() * DeltaTime,
		0.0f, MaxSoul);

	if (bIsSoulExhausted && CurrentSoul >= SoulExhaustedUnlock)
	{
		bIsSoulExhausted = false; // 고갈 해제 
	}
}

float UStatComponent::GetSoulRegenRateForCurrentHP() const
{
	const float HPPercent = MaxHP > 0.0f ? (CurrentHP / MaxHP) * 100.0f : 0.0f;

	float BaseRate;
	if (HPPercent >= 75.0f)	     BaseRate = SoulRegenRate_Over75;
	else if (HPPercent >= 50.0f) BaseRate = SoulRegenRate_50to75;
	else if (HPPercent >= 25.0f) BaseRate = SoulRegenRate_25to50;
	else						 BaseRate = SoulRegenRate_Under25;


	return bIsSoulExhausted ? BaseRate * SoulExhaustedRegenMultiplier : BaseRate;
}


