// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/Components/NTTalChiTargetComponent.h"

// Sets default values for this component's properties
UNTTalChiTargetComponent::UNTTalChiTargetComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

FTransform UNTTalChiTargetComponent::GetTalchiTransform() const
{
	return GetComponentTransform();
}

bool UNTTalChiTargetComponent::IsTalchiAvailable() const
{
	const AActor* OwnerActor = GetOwner();

	return bTalchiEnabled
		&& IsActive()
		&& IsValid(OwnerActor)
		&& !OwnerActor->IsActorBeingDestroyed()
		&& IsValid(PlayerTalchiMontage)
		&& IsValid(VictimTalchiMontage);
}

void UNTTalChiTargetComponent::SetTalchiEnabled(bool bEnabled)
{
	bTalchiEnabled = bEnabled;
}

UAnimMontage* UNTTalChiTargetComponent::GetPlayerTalchiMontage() const
{
	return PlayerTalchiMontage;
}

UAnimMontage* UNTTalChiTargetComponent::GetVictimTalchiMontage() const
{
	return VictimTalchiMontage;
}