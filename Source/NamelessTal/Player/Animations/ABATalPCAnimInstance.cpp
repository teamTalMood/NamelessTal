// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Animations/ABATalPCAnimInstance.h"
#include "Player/TalPlayerCharacter.h"

void UABATalPCAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	TalPlayerCharacter = Cast<ATalPlayerCharacter>(TryGetPawnOwner());
}

void UABATalPCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (TalPlayerCharacter == nullptr)
	{
		TalPlayerCharacter = Cast<ATalPlayerCharacter>(TryGetPawnOwner());
	}

	if (TalPlayerCharacter != nullptr) {
		Speed = TalPlayerCharacter->GetVelocity().Size();
		bIsDodging = TalPlayerCharacter->IsDodging();
	}
}
