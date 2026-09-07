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
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;

private:
	// FlowAbortMode(Both/LowerPriority)가 실제로 동작하려면 매 틱 조건을 재검사해서
	// 값이 바뀔 때 OwnerComp.RequestExecution을 호출해줘야 함 (Observer Aborts는 자동으로 안 걸림)
	struct FNTBTD_AttackInRangeMemory
	{
		bool bLastRawCondition = false;
	};
};
