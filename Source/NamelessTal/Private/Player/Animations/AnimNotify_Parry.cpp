// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Animations/AnimNotify_Parry.h"
#include "Player/TalPlayerCharacter.h"


void UAnimNotify_Parry::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, 
	const FAnimNotifyEventReference& EvenetReference)
{
	if (ATalPlayerCharacter* Character = Cast<ATalPlayerCharacter>(MeshComp->GetOwner()))
	{
		Character->TriggerParrySlowMo();
	}
}
