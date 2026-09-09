// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Animations/AnimNotify_EquipHeldTal.h"
#include "Player/TalPlayerCharacter.h"

void UAnimNotify_EquipHeldTal::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (!IsValid(MeshComp)) return;

	if (ATalPlayerCharacter* PlayerCharacter = Cast<ATalPlayerCharacter>(MeshComp->GetOwner()))
	{
		PlayerCharacter->EquipHeldTalToFace();
	}
}
