// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NTPlayerApplyDamageInterface.generated.h"

struct FPlayerApplyDamageData
{
	uint32 HPDamage = 0;
	uint32 SoulDamage = 0;
	float GuardBack = 0.0f;
	uint8 bCriticalEnable : 1 = false;
	float PlayerRecoveryTime = 0.0f;
};


// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNTPlayerApplyDamageInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class NAMELESSTAL_API INTPlayerApplyDamageInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	//virtual float ApplyDamage(AActor* DamageCauser, const float InDamage, const float InAttackDrain, const float InGuardDrain, const bool bCanParry) =0;
	virtual float ApplyDamage(AActor* DamageCauser, const FPlayerApplyDamageData InDamageData) =0;
};
