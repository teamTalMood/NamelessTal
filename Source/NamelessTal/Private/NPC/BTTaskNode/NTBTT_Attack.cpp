// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/BTTaskNode/NTBTT_Attack.h"
#include "MCLOG.h"
#include "AIController.h"
#include "NPC/Interface/NTNPCCharacterBaseInterface.h"

UNTBTT_Attack::UNTBTT_Attack()
{
}

EBTNodeResult::Type UNTBTT_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//MC_LOG(LogMC,Log, TEXT("Begin"));

	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	APawn* ControllingPawn = Cast<APawn>(OwnerComp.GetAIOwner()->GetPawn());
	if (nullptr == ControllingPawn)
	{
		return EBTNodeResult::Failed;
	}

	INTNPCCharacterBaseInterface* AIPawn = Cast<INTNPCCharacterBaseInterface>(ControllingPawn);
	if (nullptr == AIPawn)
	{
		return EBTNodeResult::Failed;
	}

	FAICharacterAttackFinished OnAttackFinished;
	OnAttackFinished.BindLambda(
		[&]()
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
	);

	AIPawn->SetAIAttackDelegate(OnAttackFinished);
	AIPawn->AttackByAI();

	//MC_LOG(LogMC, Log, TEXT("End"));

	return EBTNodeResult::InProgress;
}
