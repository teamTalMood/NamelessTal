// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/NTTalComponent.h"

#include "TalBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "UObject/UnrealType.h"

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

	ApplyTalWearScale();

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

void UNTTalComponent::ApplyTalWearScale()
{
	ATalBase* Tal = GetCurrentTal();
	USceneComponent* TalRoot = IsValid(Tal) ? Tal->GetRootComponent() : nullptr;

	if (!TalRoot) return;

	// 탈 자체 크기가 아니라 착용자 기준으로 통일합니다.
	TalRoot->SetRelativeScale3D(TalWearScale);
}

#if WITH_EDITOR
void UNTTalComponent::PostEditChangeProperty(
	FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 디테일 패널에서 값을 바꾸면 쓰고 있는 탈에 바로 반영합니다.
	if (PropertyChangedEvent.GetMemberPropertyName()
		== GET_MEMBER_NAME_CHECKED(UNTTalComponent, TalWearScale))
	{
		ApplyTalWearScale();
	}
}
#endif
