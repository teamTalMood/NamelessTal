// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NTNPCCharacterBoss.h"
#include "MCLOG.h"
#include "Game/NTGameDataSingleton.h"
#include "Stat/StatComponent.h"

//AI
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NPC/Components/NTLockOnTargetComponent.h"


ANTNPCCharacterBoss::ANTNPCCharacterBoss()
{
	PatternsNames.Add(EBossPhase::First, TEXT(""));
	PatternsNames.Add(EBossPhase::Second, TEXT(""));
	PatternsNames.Add(EBossPhase::Third, TEXT(""));

	PhasePatternDatas.Add(EBossPhase::First, FNPCAttackPatternList());
	PhasePatternDatas.Add(EBossPhase::Second, FNPCAttackPatternList());
	PhasePatternDatas.Add(EBossPhase::Third, FNPCAttackPatternList());

	PhaseHP.Add(EBossPhase::First, 260);
	PhaseHP.Add(EBossPhase::Second, 195);
	PhaseHP.Add(EBossPhase::Third, 195);
}

void ANTNPCCharacterBoss::BeginPlay()
{
	Super::BeginPlay();

	StatComponent->MaxHP = PhaseHP[CurrentPhase];
	StatComponent->RestoreFull();
	StatComponent->SetSoulRegenBlocked(true);
}

float ANTNPCCharacterBoss::ApplyDamage(AActor* DamageCauser, const FNPCApplyDamageData InDamageData)
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	float ResultDamage = Super::ApplyDamage(DamageCauser, InDamageData);

	return ResultDamage;
}


void ANTNPCCharacterBoss::SetAttackPatternDatas()
{
	const TArray<FNTNPCAttackPatternData> FindedPatternDatas = UNTGameDataSingleton::Get().FindNPCAttackPatternsByUser(PatternUserID);
	if (FindedPatternDatas.IsEmpty())
	{
		MC_LOG(LogMC, Warning, TEXT("No PatternData! %s"), *PatternUserID.ToString());

		PhasePatternDatas[EBossPhase::First].Patterns.Add({ TEXT("P_NormalMon1_Attack1_0"),  2.0f, 200.0f, 50, 0.0f });
		PhasePatternDatas[EBossPhase::Second].Patterns.Add({ TEXT("P_NormalMon1_Attack2_0"), 4.0f, 450.0f, 30, 0.0f });
		PhasePatternDatas[EBossPhase::Third].Patterns.Add({ TEXT("P_NormalMon1_Attack3_0"),   6.0f, 700.0f, 15, 0.0f });
	}
	else
	{
		for (FNTNPCAttackPatternData PatternData : FindedPatternDatas)
		{
			for (const TPair<EBossPhase, FName>& Pair : PatternsNames)
			{
				EBossPhase Phase = Pair.Key;
				FName PatternName = Pair.Value;

				if (PatternData.FirstStepID.ToString().StartsWith(PatternName.ToString()))
				{
					PhasePatternDatas[Phase].Patterns.Add(FNPCAttackPatternSelectData(PatternData.FirstStepID, PatternData.CoolDown, PatternData.PatternRange, PatternData.PatternWeight, 0.0f));
				}
			}
		}
	}
}

TArray<FNPCAttackPatternSelectData>& ANTNPCCharacterBoss::GetAttackPatternDatas()
{
	return PhasePatternDatas[CurrentPhase].Patterns;
}

void ANTNPCCharacterBoss::HandleDeath()
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	if (bIsDead)
	{
		MC_LOG(LogMC, Error, TEXT("bIsDead is Already"));
		return;
	}

	bIsDead = true;

	//락온 해제
	if (UNTLockOnTargetComponent* LockOnTarget = FindComponentByClass<UNTLockOnTargetComponent>())
	{
		LockOnTarget->SetLockOnEnabled(false);
	}

	SetCanBeDamaged(false);

	//공격 상태 종료
	bIsAttacking = false;
	//CurrentAttack = EAttackDistance::AttackNone;//상태변환
	SetActorEnableCollision(false);//갖고있는 콜리전 다 해제
	//DisableWeaponCollision();//콜리전해제
	OnAICharacterAttackFinished.Unbind();//바인드해제

	//재생중인 몽타주 종료 + 바인딩된 종료 델리게이트 해제 (사망 중 콤보 재호출 방지)
	StopCurrentPlayMontage();

	if (DeathMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			FOnMontageEnded EndDelegate;
			EndDelegate.BindUObject(this, &ANTNPCCharacterBoss::StartNextPhase);
			AnimInstance->Montage_SetEndDelegate(EndDelegate, DeathMontage);

			AnimInstance->Montage_Play(DeathMontage);
		}
	}
	else
	{
		MC_LOG(LogSY, Warning, TEXT("DeathMontage is not assigned : %s"), *GetName());
	}

	MC_LOG(LogSY, Warning, TEXT("NPC DEAD : %s"), *GetName());
}

void ANTNPCCharacterBoss::StartNextPhase(UAnimMontage* TargetMontage, bool IsProperlyEnded)
{
	switch (CurrentPhase)
	{
	case EBossPhase::First:
		CurrentPhase = EBossPhase::Second;
		break;
	case EBossPhase::Second:
		CurrentPhase = EBossPhase::Third;
		break;
	case EBossPhase::Third:
		break;
	default:
		break;
	}

	StatComponent->MaxHP = PhaseHP[CurrentPhase];
	StatComponent->RestoreFull();
	StatComponent->SetSoulRegenBlocked(true);
	bIsDead = false;

	FString EnumString = StaticEnum<EBossPhase>()->GetNameByValue(static_cast<int64>(CurrentPhase)).ToString();
	MC_LOG(LogMC, Log, TEXT("End : %s"), *EnumString);
}
