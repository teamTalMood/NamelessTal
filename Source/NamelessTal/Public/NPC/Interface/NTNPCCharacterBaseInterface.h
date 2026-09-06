// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NTNPCCharacterBaseInterface.generated.h"

DECLARE_DELEGATE(FAICharacterAttackFinished);

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNTNPCCharacterBaseInterface : public UInterface
{
	GENERATED_BODY()
};

UENUM()
enum class EAttackDistance : uint8
{
	AttackNone = 0,
	AttackShort = 10,
	AttackMiddle = 20,
	AttackLong = 30
};

/**
 *
 */
class NAMELESSTAL_API INTNPCCharacterBaseInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual float GetAIPatrolRadius() = 0;
	virtual float GetAIDetectRange() = 0;
	virtual float GetAIAttackRange() = 0;
	virtual float GetAITurnSpeed() = 0;
	virtual void AttackByAI() = 0;
	virtual void SetAIAttackDelegate(const FAICharacterAttackFinished& InOnAttackFinished) = 0;

	virtual float ApplyDamage(const float InDamage, const float InDrain, const bool bIsParry) = 0;

	// 0 : 애니메이션 없음 | 1 : 시계방향 스윙 | 2 : 반시계방향 스윙 | 3 : 정면방향 스윙
	virtual uint8 GetAttackDirection() = 0;
};
