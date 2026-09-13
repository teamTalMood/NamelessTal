// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/Animations/NTNPCAnimInstance.h"
#include "MCLOG.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UNTNPCAnimInstance::UNTNPCAnimInstance()
{
	MovingThreshould = 3.0f;
	JumpingThreshould = 100.0f;
}

void UNTNPCAnimInstance::SetAnimIsExhausted(const uint8 InIsExhausted)
{
	bIsExhausted = InIsExhausted;
}

void UNTNPCAnimInstance::SetAnimDeaded(const FVector& InGroundNormal, const FVector& InForwardVector)
{
	DeathGroundNormal = InGroundNormal;
	DeathForwardVector = InForwardVector;

	bIsDead = true;

	//MC_LOG(LogMC, Log, TEXT("Begin"));
}

void UNTNPCAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Owner = Cast<ACharacter>(GetOwningActor());
	if (Owner)
	{
		Movement = Owner->GetCharacterMovement();
	}
}

void UNTNPCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (Movement)
	{
		Velocity = Movement->Velocity;
		GroundSpeed = Velocity.Size2D();
		bIsIdle = GroundSpeed < MovingThreshould;
		bIsFalling = Movement->IsFalling();
		bIsJumping = bIsFalling & (Velocity.Z > JumpingThreshould);
	}
}
