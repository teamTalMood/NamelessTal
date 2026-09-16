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

USTRUCT()
struct FNPCApplyDamageData
{
	GENERATED_BODY()

	int32 HPDamage = 0;
	int32 SoulDamage = 0;
	int32 StaggerDamage = 0.0f;
	uint8 bIsParry : 1 = false;

	// 타격으로 죽일때 시체 날릴 impulse
	// 방향 * 세기 값 ZeroVector면 날리지 않고 몽타주 실행해서 제자리에서 죽게하기
	FVector LaunchImpulse = FVector::ZeroVector;

};

USTRUCT()
struct FNPCAttackPatternSelectData
{
	GENERATED_BODY()

	FName FirstStepID;
	float CoolDown;
	float PatternRange;
	int32 PatternWeight;
	float CurrentCoolDown;
};

USTRUCT()
struct FNPCTalchiData
{
	GENERATED_BODY()
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

	virtual float ApplyDamage(AActor* DamageCauser, const float InDamage, const float InDrain, const bool bIsParry) = 0; // Legacy
	virtual float ApplyDamage(AActor* DamageCauser, const FNPCApplyDamageData InDamageData) = 0;

	// 0 : 애니메이션 없음 | 1 : 시계방향 스윙 | 2 : 반시계방향 스윙 | 3 : 정면방향 스윙
	virtual uint8 GetAttackDirection() = 0;

	// AttackPatternDatas 받아오기
	virtual TArray<FNPCAttackPatternSelectData>& GetAttackPatternDatas() = 0;

	////////////////////////////////
	// 탈취 코드
	// 그로기 상태인지 확인
	virtual const uint8 GetIsExhausting() = 0;
	virtual void TalChiNPC(FNPCTalchiData InTalchiData) = 0;
};
