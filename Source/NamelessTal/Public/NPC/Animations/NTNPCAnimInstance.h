// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "NTNPCAnimInstance.generated.h"

/**
 *
 */
UCLASS()
class NAMELESSTAL_API UNTNPCAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UNTNPCAnimInstance();

	void SetAnimIsExhausted(const uint8 InIsExhausted);
	void SetAnimDeaded(const FVector& InGroundNormal, const FVector& InForwardVector);

protected:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<class ACharacter> Owner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Character)
	TObjectPtr<class UCharacterMovementComponent> Movement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	FVector Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	float GroundSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	uint8 bIsIdle : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	float MovingThreshould;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	uint8 bIsFalling : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	uint8 bIsJumping : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	float JumpingThreshould;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	uint8 bIsExhausted : 1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	uint8 bIsDead : 1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	FVector DeathGroundNormal = FVector::UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Character)
	FVector DeathForwardVector = FVector::ForwardVector;
};
