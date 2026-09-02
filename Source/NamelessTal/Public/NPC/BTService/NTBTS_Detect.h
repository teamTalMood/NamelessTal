// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "NTBTS_Detect.generated.h"

/**
 * 
 */
UCLASS()
class NAMELESSTAL_API UNTBTS_Detect : public UBTService
{
	GENERATED_BODY()

public:
	UNTBTS_Detect();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
};
