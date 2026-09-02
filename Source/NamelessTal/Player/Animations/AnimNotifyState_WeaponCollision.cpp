// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Animations/AnimNotifyState_WeaponCollision.h"
#include "player/TalPlayerCharacter.h"

void UAnimNotifyState_WeaponCollision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if(ATalPlayerCharacter* Character = Cast<ATalPlayerCharacter>(MeshComp->GetOwner()))
	{
		Character->EnableWeaponCollision();
	}
}

void UAnimNotifyState_WeaponCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if(ATalPlayerCharacter* Character = Cast<ATalPlayerCharacter>(MeshComp->GetOwner()))
	{
		Character->DisableWeaponCollision();
	}
}
