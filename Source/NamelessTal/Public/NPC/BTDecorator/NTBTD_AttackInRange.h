// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "NTBTD_AttackInRange.generated.h"

/**
 * 
 */
UCLASS()
class NAMELESSTAL_API UNTBTD_AttackInRange : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	UNTBTD_AttackInRange();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
};
