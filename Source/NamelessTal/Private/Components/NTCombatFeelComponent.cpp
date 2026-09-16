// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/NTCombatFeelComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Engine/World.h"

#include "Camera/CameraShakeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

// Sets default values for this component's properties
UNTCombatFeelComponent::UNTCombatFeelComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 히트스톱 걸릴 시 걸린 동안 틱 돌기, 평소 비용 = 0
	PrimaryComponentTick.bStartWithTickEnabled = false;
}


// Called when the game starts
void UNTCombatFeelComponent::BeginPlay()
{
	Super::BeginPlay();
}

float UNTCombatFeelComponent::GetDurationFor(ENTHitStopKind Kind) const
{
	switch (Kind)
	{
	case ENTHitStopKind::Normal:
		return HitStopDuration_NormalAttack;
	case ENTHitStopKind::SpecialArt:
		return HitStopDuration_SpecialAttack;
	case ENTHitStopKind::Guard:
		return HitStopDuration_Guard;
	case ENTHitStopKind::Kill:
		return HitStopDuration_Kill;
	default : 
		return 0.0f;
	}
}

void UNTCombatFeelComponent::ApplyDilation(float Value)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	//시간 배율 만지는곳은 여기뿐
	UGameplayStatics::SetGlobalTimeDilation(World, Value);
}


TSubclassOf<UCameraShakeBase> UNTCombatFeelComponent::GetShakeFor(ENTHitStopKind Kind) const
{
	switch (Kind)
	{
	case ENTHitStopKind::Normal: 
		return Shake_NormalAttack;
	case ENTHitStopKind::SpecialArt: 
		return Shake_SpecialAttack;
	case ENTHitStopKind::Guard: 
		return Shake_Guard;
	case ENTHitStopKind::Kill:
		return Shake_Kill;
	default : 
		return nullptr;
	}
}

void UNTCombatFeelComponent::PlayShake(TSubclassOf<UCameraShakeBase> ShakeClass)
{
	if (!ShakeClass || CameraShakeScale <= 0.0f)
	{
		return;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) {
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
	{
		PC->ClientStartCameraShake(ShakeClass, CameraShakeScale);
	}
}

// Called every frame
void UNTCombatFeelComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Now = World->GetRealTimeSeconds();

	if (bParrySlowMoActive)
	{
		if (Now >= ParrySlowMoEndRealTime)
		{
			StopParrySlowMo();
		}
	}
	else if (bHitStopActive)
	{
		if (Now >= HitStopEndRealTime)
		{
			ApplyDilation(1.0f);
			bHitStopActive = false;
		}
	}

	if (!bParrySlowMoActive && !bHitStopActive)
	{
		SetComponentTickEnabled(false);
	}
}

void UNTCombatFeelComponent::TriggerHitStop(ENTHitStopKind Kind)
{
	// 셰이크는 패링중에도 나감 히트스톱만 막힘
	PlayShake(GetShakeFor(Kind));

	if (bParrySlowMoActive)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Duration = GetDurationFor(Kind);

	if (Duration <= 0.0f)
	{
		return;
	}

	ApplyDilation(HitStopTimeDilation);
	bHitStopActive = true;

	// 연속 타격이면 더 긴쪽 늘리기
	HitStopEndRealTime = FMath::Max(HitStopEndRealTime,
		World->GetRealTimeSeconds() + Duration);

	SetComponentTickEnabled(true);
}

void UNTCombatFeelComponent::StartParrySlowMo()
{


	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	PlayShake(Shake_Parry);
	
	// 패링이 우선 -> 진행중인 히트스탑 멈추기
	bHitStopActive = false;

	ApplyDilation(ParrySlowMoTimeDilation);
	bParrySlowMoActive = true;
	ParrySlowMoEndRealTime = World->GetRealTimeSeconds()
		+ ParrySlowMoRealDuration;

	SetComponentTickEnabled(true);
}

void UNTCombatFeelComponent::StopParrySlowMo()
{
	if (!bParrySlowMoActive)
	{
		return;
	}

	bParrySlowMoActive = false;
	ApplyDilation(1.0f);
}

