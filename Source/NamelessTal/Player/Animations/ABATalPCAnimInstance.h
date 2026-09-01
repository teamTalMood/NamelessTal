// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ABATalPCAnimInstance.generated.h"

class ATalPlayerCharacter;
/**
 * 
 */
UCLASS()
class NAMELESSTAL_API UABATalPCAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public :
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Speed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsDodging = false;

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY()
	ATalPlayerCharacter* TalPlayerCharacter;

};
