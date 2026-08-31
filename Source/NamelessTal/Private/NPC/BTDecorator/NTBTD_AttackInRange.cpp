// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/BTDecorator/NTBTD_AttackInRange.h"
#include "NPC/NTAIParams.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NPC/Interface/NTNPCCharacterBaseInterface.h"

UNTBTD_AttackInRange::UNTBTD_AttackInRange()
{
	NodeName = TEXT("CanAttack");
}

bool UNTBTD_AttackInRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	bool bResult = Super::CalculateRawConditionValue(OwnerComp, NodeMemory);

	APawn* ControllingPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (nullptr == ControllingPawn)
	{
		return false;
	}

	INTNPCCharacterBaseInterface* AIPawn = Cast<INTNPCCharacterBaseInterface>(ControllingPawn);
	if (nullptr == AIPawn)
	{
		return false;
	}

	APawn* Target = Cast<APawn>(OwnerComp.GetBlackboardComponent()->GetValueAsObject(BBKEY_TARGET));
	if (nullptr == Target)
	{
		return false;
	}

	float DistanceToTarget = ControllingPawn->GetDistanceTo(Target);
	float AttackRangeWithRadius = AIPawn->GetAIAttackRange();
	bResult = (DistanceToTarget <= AttackRangeWithRadius);
	return bResult;
}
