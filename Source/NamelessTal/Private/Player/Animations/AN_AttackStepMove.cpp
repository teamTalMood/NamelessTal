// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Animations/AN_AttackStepMove.h"

#include "Player/TalPlayerCharacter.h"

void UAN_AttackStepMove::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp) return;

	if (ATalPlayerCharacter* Character = Cast<ATalPlayerCharacter>(MeshComp->GetOwner()))
	{
		Character->BeginAttackStepMove(OverrideDistance, OverrideDuration);
	}
}

FString UAN_AttackStepMove::GetNotifyName_Implementation() const
{
	return TEXT("AttackStepMove");
}