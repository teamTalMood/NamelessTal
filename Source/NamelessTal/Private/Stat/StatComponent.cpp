// Fill out your copyright notice in the Description page of Project Settings.


#include "Stat/StatComponent.h"
#include "MCLOG.h"



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

	OnHPChanged.Broadcast(CurrentHP, MaxHP);
	OnSoulChanged.Broadcast(CurrentSoul, MaxSoul);
}

void UStatComponent::ApplyDamage(float HitDamage)
{
	if (HitDamage <= 0.0f || bDeathBroadcasted) return; //예외처리
	
	const float PreviousHP = CurrentHP;

	//데미지 처리
	CurrentHP = FMath::Clamp(CurrentHP - HitDamage, 0.0f, MaxHP);
	MC_SUBLOG(LogMC, Log, TEXT("CurrentHP : %f"), CurrentHP);

	OnHPChanged.Broadcast(CurrentHP, MaxHP);

	//델리게이트
	if (PreviousHP > 0.0f && CurrentHP <= 0.0f)
	{
		bDeathBroadcasted = true;

		MC_SUBLOG(LogMC, Log, TEXT("아잇 죽었다(테스트용)"));
		OnDeath.Broadcast();
	}
}

void UStatComponent::HealHp(float HealAmount)
{
	CurrentHP = FMath::Clamp(CurrentHP + HealAmount, 0.0f, MaxHP);
	OnHPChanged.Broadcast(CurrentHP, MaxHP);
}


void UStatComponent::RestoreFull()
// 모닥불 처리
{
	bTalchiRecoveryPaused = false;
	TalchiRemainingGroggyTime = 0.0f;

	CurrentHP = MaxHP;
	CurrentSoul = MaxSoul;
	bIsSoulExhausted = false; // 고갈 패널티 제거 
	bDeathBroadcasted = false; // 사망 플래그 초기화

	bIsGroggy = false; // 그로기 해제
	SetSoulRegenBlocked(false); // 넋 회복 차단 해제

	OnHPChanged.Broadcast(CurrentHP, MaxHP);
	OnSoulChanged.Broadcast(CurrentSoul, MaxSoul);
}

void UStatComponent::RestoreSoul()
{
	if (bTalchiRecoveryPaused)
	{
		return;
	}
	CurrentSoul = MaxSoul;
	bIsSoulExhausted = false;
	MC_SUBLOG(LogMC, Log, TEXT("CurrentSoul : %f"), CurrentSoul);

	OnSoulChanged.Broadcast(CurrentSoul, MaxSoul);
}

void UStatComponent::RecoverSoul(float Amount)
{
	if (bTalchiRecoveryPaused)
	{
		return;
	}
	CurrentSoul = FMath::Clamp(CurrentSoul + Amount, 0.0f, MaxSoul);
	OnSoulChanged.Broadcast(CurrentSoul, MaxSoul);
}



void UStatComponent::DrainSoul(float Amount)
{
	CurrentSoul = FMath::Clamp(CurrentSoul - Amount, 0.0f, MaxSoul);

	if (CurrentSoul <= 0.0f)
	{
		bIsSoulExhausted = true; // 고갈 진입
		
		if (!bIsGroggy)
		{
			bIsGroggy = true; // 그로기 시작
			GroggyEndTime = GetWorld()->GetTimeSeconds() + GroggyDuration;
			SetSoulRegenBlocked(true);

			// 잠금 도중 새로 그로기에 진입해도 재개할 시간을 보관합니다.
			if (bTalchiRecoveryPaused)
			{
				TalchiRemainingGroggyTime = FMath::Max(0.0f, GroggyDuration);
			}
		}
	}


	UE_LOG(LogTemp, Warning, TEXT("현재 넋 %f"), CurrentSoul);
	MC_SUBLOG(LogMC, Log, TEXT("CurrentSoul : %f"), CurrentSoul);



	//언제 마지막으로 사용했는지 체크하기
	LastSoulDrainTime = GetWorld()->GetTimeSeconds();
	OnSoulChanged.Broadcast(CurrentSoul, MaxSoul);
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

bool UStatComponent::IsInCombat() const
{
	return GetWorld()->GetTimeSeconds() - LastSoulDrainTime < CombatCooldown;
}

void UStatComponent::NotifyHitDuringGroggy()
{
	if (bIsGroggy)
	{
		EndGroggy();
	}
}

void UStatComponent::NotifyCombatAction()
{
	LastSoulDrainTime=GetWorld()->GetTimeSeconds();
}

// 시간이 지나면 (1초) 넋이 시간초에 따라 자동으로 찬다
void UStatComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* PCSoulTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, PCSoulTickFunction);

	if (bTalchiRecoveryPaused || IsDead())
	{
		return;
	}

	if (bIsPlayer && bIsGroggy && GetWorld()->GetTimeSeconds() >= GroggyEndTime)
	{
		EndGroggy();
	}

	if (bSoulRegenBlocked)
	{
		return;
	}

	const float TimeSinceLastDrain = GetWorld()->GetTimeSeconds() - LastSoulDrainTime;
	if (TimeSinceLastDrain < SoulRegenDelay)
	{
		return;
	}

	CurrentSoul = FMath::Clamp(CurrentSoul + 
		GetSoulRegenRate() * DeltaTime,
		0.0f, MaxSoul);

	OnSoulChanged.Broadcast(CurrentSoul, MaxSoul);

	if (bIsSoulExhausted && CurrentSoul >= SoulExhaustedUnlock)
	{
		bIsSoulExhausted = false; // 고갈 해제
	}
}


float UStatComponent::GetSoulRegenRateForCurrentHP() const
{
	return bIsSoulExhausted ? SoulRegenRate * SoulExhaustedRegenMultiplier :
		SoulRegenRate;
}

float UStatComponent::GetSoulRegenRate() const
{
	return GetSoulRegenRateForCurrentHP();
}

void UStatComponent::EndGroggy()
{
	if (!bIsGroggy || bTalchiRecoveryPaused) return; 
	bIsGroggy = false;
	SetSoulRegenBlocked(false);
	CurrentSoul = FMath::Clamp(CurrentSoul + GroggyRecoverSoul, 0.0f, MaxSoul);

	OnSoulChanged.Broadcast(CurrentSoul, MaxSoul);
}

void UStatComponent::SetTalchiRecoveryPaused(bool bPaused)
{
	if (bTalchiRecoveryPaused == bPaused || !GetWorld())
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();

	if (bPaused)
	{
		TalchiRemainingGroggyTime = bIsGroggy
			? FMath::Max(0.0f, GroggyEndTime - Now)
			: 0.0f;

		bTalchiRecoveryPaused = true;
		return;
	}

	bTalchiRecoveryPaused = false;

	if (bIsGroggy)
	{
		GroggyEndTime = Now + TalchiRemainingGroggyTime;
	}

	TalchiRemainingGroggyTime = 0.0f;

	// 자동 회복 대기 시간은 탈취 종료 시점부터 다시 계산합니다.
	LastSoulDrainTime = Now;
}