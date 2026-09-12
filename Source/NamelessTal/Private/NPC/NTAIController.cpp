// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NTAIController.h"
#include "MCLOG.h"
#include "NPC/NTAIParams.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"

ANTAIController::ANTAIController()
{
}

void ANTAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (BBAsset != nullptr && BTAsset != nullptr)
	{
		RunAI();
	}
	else
	{
		MC_LOG(LogMC, Error, TEXT("No BBAsset or BTAsset!!"));
	}

}

void ANTAIController::RunAI()
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	UBlackboardComponent* BlackboardPtr = Blackboard.Get();
	if (UseBlackboard(BBAsset, BlackboardPtr))
	{
		Blackboard->SetValueAsVector(BBKEY_HOMEPOS, GetPawn()->GetActorLocation());
		bool RunResult = RunBehaviorTree(BTAsset);
		ensure(RunResult);
	}
}

void ANTAIController::StopAI()
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	UBehaviorTreeComponent* BTComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (BTComponent)
	{
		BTComponent->StopTree();
	}
}


