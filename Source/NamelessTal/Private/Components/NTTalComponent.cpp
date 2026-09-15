// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/NTTalComponent.h"

#include "TalBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"

UNTTalComponent::UNTTalComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

ATalBase* UNTTalComponent::GetCurrentTal() const
{
	return IsValid(CurrentTal.Get()) ? CurrentTal.Get() : nullptr;
}

bool UNTTalComponent::EquipTal(ATalBase* NewTal)
{
	if (!IsValid(NewTal)) return false;
	if (GetCurrentTal() == NewTal) return true;
	if (GetCurrentTal()) return false;
	if (NewTal->GetOwner() && NewTal->GetOwner() != GetOwner()) return false;

	ACharacter* Character = Cast<ACharacter>(GetOwner());
	USkeletalMeshComponent* Mesh = Character ? Character->GetMesh() : nullptr;

	if (!IsValid(Mesh) || !Mesh->DoesSocketExist(TalSocketName))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Tal equip failed: check character mesh and socket '%s'."),
			*TalSocketName.ToString()
		);
		return false;
	}

	const bool bAttached = NewTal->AttachToComponent(
		Mesh,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		TalSocketName
	);

	if (!bAttached) return false;

	NewTal->SetOwner(GetOwner());
	CurrentTal = NewTal;

	OnTalChanged.Broadcast(CurrentTal);

	return true;
}

ATalBase* UNTTalComponent::UnequipTal()
{
	ATalBase* OldTal = GetCurrentTal();
	CurrentTal = nullptr;

	if (!OldTal) return nullptr;

	OldTal->DetachFromActor(
		FDetachmentTransformRules::KeepWorldTransform
	);
	OldTal->SetOwner(nullptr);

	OnTalChanged.Broadcast(nullptr);

	return OldTal;
}

bool UNTTalComponent::SpawnDefaultTal(TSubclassOf<ATalBase> TalClass)
{
	if (!TalClass || !GetWorld() || !IsValid(GetOwner()))
	{
		return false;
	}

	if (GetCurrentTal())
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ATalBase* NewTal = GetWorld()->SpawnActor<ATalBase>(
		TalClass,
		GetOwner()->GetActorLocation(),
		GetOwner()->GetActorRotation(),
		SpawnParams
	);

	if (!IsValid(NewTal))
	{
		return false;
	}

	if (!EquipTal(NewTal))
	{
		NewTal->Destroy();
		return false;
	}

	return true;
}
