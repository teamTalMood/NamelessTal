// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NPC/NTNPCCharacterBase.h"
#include "NTNPCCharacterEpic.generated.h"

/**
 * 
 */
UCLASS()
class NAMELESSTAL_API ANTNPCCharacterEpic : public ANTNPCCharacterBase
{
	GENERATED_BODY()

protected:
	virtual void HandleDeath() override;
};
