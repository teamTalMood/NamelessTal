// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "NTAIController.generated.h"

/**
 * 
 */
UCLASS()
class NAMELESSTAL_API ANTAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	ANTAIController();

protected:
	virtual void OnPossess(APawn* InPawn) override;

public:
	void RunAI();
	void StopAI();

protected:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UBlackboardData> BBAsset;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UBehaviorTree> BTAsset;
};
