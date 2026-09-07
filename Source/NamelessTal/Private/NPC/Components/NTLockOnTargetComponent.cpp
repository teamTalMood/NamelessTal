// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/Components/NTLockOnTargetComponent.h"

#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UNTLockOnTargetComponent::UNTLockOnTargetComponent()
{

	//일정한 크기로 표시
	SetWidgetSpace(EWidgetSpace::Screen);

	//위젯 원본 크기사용
	SetDrawAtDesiredSize(true);

	//중앙으로 컴포넌트 위치
	SetPivot(FVector2D(0.5f, 0.5f));

	//락온되기전엔 숨기기
	SetVisibility(false);

	//충돌은 Off
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetGenerateOverlapEvents(false);
}

bool UNTLockOnTargetComponent::IsLockOnAvailable() const
{
	const AActor* OwnerActor = GetOwner();

	return bLockOnEnabled
		&& IsValid(OwnerActor)
		&& !OwnerActor->IsActorBeingDestroyed();
}

FVector UNTLockOnTargetComponent::GetLockOnLocation() const
{
	return GetComponentLocation();
}

void UNTLockOnTargetComponent::SetLockOnEnabled(bool bEnabled)
{
	bLockOnEnabled = bEnabled;

	if (!bLockOnEnabled)
	{
		SetLockOnWidgetVisible(false);
	}
}

void UNTLockOnTargetComponent::SetLockOnWidgetVisible(bool bShouldBeVisible)
{
	SetVisibility(bShouldBeVisible && bLockOnEnabled);
}
