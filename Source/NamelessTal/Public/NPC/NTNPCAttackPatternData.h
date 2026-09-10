#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NTNPCAttackPatternData.generated.h"

USTRUCT(BlueprintType)
struct FNTNPCAttackPatternData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackPattern")
	FString PatternUser;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackPattern")
	FString PatternName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackPattern")
	int32 ComboCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackPattern")
	FString NextPattern;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackPattern")
	int32 DamageHP = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackPattern")
	int32 DamageSoul = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackPattern")
	float GuardBack = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackPattern")
	bool CriticalEnable = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackPattern")
	float CastTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackPattern")
	int32 HitStun = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackPattern")
	float RecoveryTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackPattern")
	float PlayerRecoveryTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackPattern")
	int32 CoolDown = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackPattern")
	float PatternRange = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackPattern")
	int32 PatternWeight = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackPattern")
	FString AnimationClip;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AttackPattern")
	FString Memo;
};