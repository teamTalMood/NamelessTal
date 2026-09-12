// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/BTService/NTBTS_SelectPattern.h"
#include "MCLOG.h"
#include "NPC/NTAIParams.h"
#include "NPC/Interface/NTNPCCharacterBaseInterface.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UNTBTS_SelectPattern::UNTBTS_SelectPattern()
{
	NodeName = TEXT("SelectPattern");
	Interval = 1.0f;
}

void UNTBTS_SelectPattern::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	MC_LOG(LogMC, Log, TEXT("Begin"));

	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (nullptr == ControllingPawn)
	{
		return;
	}

	INTNPCCharacterBaseInterface* AIPawn = Cast<INTNPCCharacterBaseInterface>(ControllingPawn);
	if (nullptr == AIPawn)
	{
		return;
	}

	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	APawn* Target = Cast<APawn>(BB->GetValueAsObject(BBKEY_TARGET));
	if (nullptr == Target)
	{
		return;
	}

	const float DistanceToTarget = ControllingPawn->GetDistanceTo(Target);

	// 0. INTNPCCharacterBaseInterface로 패턴배열 갖고옴 + DeltaSeconds만큼 쿨타임 감소
	TArray<FNPCAttackPatternSelectData>& Patterns = AIPawn->GetAttackPatternDatas();
	for (FNPCAttackPatternSelectData& Pattern : Patterns)
	{
		Pattern.CurrentCoolDown = FMath::Max(0.f, Pattern.CurrentCoolDown - DeltaSeconds);
	}

	// [TEST] 패턴별 이름-현재쿨타임-요구거리 로그
	for (const FNPCAttackPatternSelectData& Pattern : Patterns)
	{
		MC_LOG(LogMC, Log, TEXT("Pattern %s | CurrentCoolDown=%.2f | PatternRange=%.1f"), *Pattern.FirstStepID.ToString(), Pattern.CurrentCoolDown, Pattern.PatternRange);
	}

	// 1~2. 거리 필터 + 쿨탐 필터
	TArray<FNPCAttackPatternSelectData*> AvailablePatterns;
	int32 TotalWeight = 0;
	for (FNPCAttackPatternSelectData& Pattern : Patterns)
	{
		if (DistanceToTarget > Pattern.PatternRange || Pattern.CurrentCoolDown > 0.f)
		{
			continue;
		}
		AvailablePatterns.Add(&Pattern);
		TotalWeight += Pattern.PatternWeight;
	}

	// 3. 가중치 랜덤 선택 / 4. 없으면 없는 걸로
	if (AvailablePatterns.Num() == 0 || TotalWeight <= 0)
	{
		BB->SetValueAsName(BBKEY_SELECTEDATTACK, BB_ATTACKPATTERN_NONE);
		MC_LOG(LogMC, Log, TEXT("Distance=%.1f | Selected=None"), DistanceToTarget);
		return;
	}

	int32 RandValue = FMath::RandRange(0, TotalWeight - 1);
	FNPCAttackPatternSelectData* SelectedPattern = AvailablePatterns[0];
	for (FNPCAttackPatternSelectData* Pattern : AvailablePatterns)
	{
		RandValue -= Pattern->PatternWeight;
		if (RandValue < 0)
		{
			SelectedPattern = Pattern;
			break;
		}
	}

	SelectedPattern->CurrentCoolDown = SelectedPattern->CoolDown;
	BB->SetValueAsName(BBKEY_SELECTEDATTACK, SelectedPattern->FirstStepID);
	BB->SetValueAsVector(BBKEY_ATTACKPOS, Target->GetActorLocation());

	MC_LOG(LogMC, Log, TEXT("Distance=%.1f | Selected=%s"), DistanceToTarget, *SelectedPattern->FirstStepID.ToString());
}

