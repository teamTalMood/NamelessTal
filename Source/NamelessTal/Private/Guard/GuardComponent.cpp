// Fill out your copyright notice in the Description page of Project Settings.


#include "Guard/GuardComponent.h"
#include "TimerManager.h"

// Sets default values for this component's properties
UGuardComponent::UGuardComponent()
{
	// 틱 안씁니다
	PrimaryComponentTick.bCanEverTick = false;
}

bool UGuardComponent::IsStaggered() const
{
	return GetWorld()->GetTimeSeconds() < StaggerEndTime;
}

void UGuardComponent::StartStagger(float Duration)
{
	StaggerEndTime = GetWorld()->GetTimeSeconds() + Duration;
}


void UGuardComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UGuardComponent::ActivateGuard()
{
	bIsGuarding = true;
	GuardStartTime = GetWorld()->GetTimeSeconds();

	const float MinParryWindow = ParryWindowBase - ParryWindowStackDec * MaxParryStackCount;

	CurrentParryWindow = FMath::Max(ParryWindowBase - ParryWindowStackDec * ParryWindowStackCount,
		MinParryWindow);
}


void UGuardComponent::StartGuard()
{
	bIsGuardInputHeld = true;
	
	// 가드 해제 후 짧은 시간 내 재입력시 패링 윈도우 스택 누적
	const float TimeSinceLastStop = GetWorld()->GetTimeSeconds() - LastGuardStopTime;

	if (TimeSinceLastStop <= ParryWindowRetryLimit)
	{ 
		// 5까지 현재 스택 누적
		ParryWindowStackCount = FMath::Min(ParryWindowStackCount + 1, MaxParryStackCount);
	} 
	else { // 시간 지나면 리셋
		ParryWindowStackCount = 0;
	}
	
	//실제 가드 활성화는 딜레이 이후
	GetWorld()->GetTimerManager().SetTimer(
		GuardActivationTimerHandle, this, &UGuardComponent::ActivateGuard,
		GuardActivationDelay, false);


}

void UGuardComponent::StopGuard()
{
	bIsGuardInputHeld = false;

	// 딜레이 중 버튼 뗄 시 , 활성화 안되도록 타이머 취소
	GetWorld()->GetTimerManager().ClearTimer(GuardActivationTimerHandle);
	bIsGuarding = false;
	LastGuardStopTime = GetWorld()->GetTimeSeconds();
}

bool UGuardComponent::TryParry() const
{
	// 가드 안하고 잇을 경우
	if (!bIsGuarding) 
	{ 
		// 가드 실패
		return false;
	}
	
	// 지금 시간 - 가드 시작 시간 <= 이번 가드의 패링 윈도우보다 작거나 같을 시 True 반환
	const float TimeSinceGuard = GetWorld()->GetTimeSeconds() - GuardStartTime;
	const bool bParried = TimeSinceGuard <= CurrentParryWindow;

	if (bParried)
	{
		UE_LOG(LogTemp, Warning, TEXT("패링성공"));
	}

	return bParried;
}

