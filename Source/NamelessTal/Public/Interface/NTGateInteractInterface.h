// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NTGateInteractInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNTGateInteractInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class NAMELESSTAL_API INTGateInteractInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void PlayGateOpenMontage(const FTransform& WarpTargetTransform, class UAnimMontage* Montage) = 0;
};
