// Fill out your copyright notice in the Description page of Project Settings.


#include "Guard/GuardComponent.h"

// Sets default values for this component's properties
UGuardComponent::UGuardComponent()
{
	// 틱 안씁니다
	PrimaryComponentTick.bCanEverTick = false;
}

void UGuardComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UGuardComponent::StartGuard()
{
	bIsGuarding = true;
	GuardStartTime = GetWorld()->GetTimeSeconds();
}

void UGuardComponent::StopGuard()
{
	bIsGuarding = false;
}

bool UGuardComponent::TryParry() const
{
	// 가드 안하고 잇을 경우
	if (!bIsGuarding) 
	{ 
		// 가드 실패
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("패링성공"));
	// 지금 시간 - 가드 시작 시간 <= 0.1초 보다 작거나 같을 경우에만 True 반환
	const float TimeSinceGuard = GetWorld()->GetTimeSeconds() - GuardStartTime;
	return TimeSinceGuard <= ParryWindow;

}

