// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "NTBTT_Attack.generated.h"

/**
 * 
 */
UCLASS()
class NAMELESSTAL_API UNTBTT_Attack : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UNTBTT_Attack();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

};
