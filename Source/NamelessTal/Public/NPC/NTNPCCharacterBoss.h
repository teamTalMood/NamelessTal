// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NPC/NTNPCCharacterBase.h"
#include "NPC/NTNPCAttackPatternData.h"
#include "NTNPCCharacterBoss.generated.h"

UENUM(BlueprintType)
enum class EBossPhase : uint8
{
	First = 0,
	Second = 1,
	Third = 2
};

USTRUCT(BlueprintType)
struct FNPCAttackPatternList
{
	GENERATED_BODY()

	FNPCAttackPatternList() = default;
	FNPCAttackPatternList(const TArray<FNPCAttackPatternSelectData>& InPatterns) : Patterns(InPatterns) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FNPCAttackPatternSelectData> Patterns;
};

/**
 * 
 */
UCLASS()
class NAMELESSTAL_API ANTNPCCharacterBoss : public ANTNPCCharacterBase
{
	GENERATED_BODY()

public:
	ANTNPCCharacterBoss();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual float ApplyDamage(AActor* DamageCauser, const FNPCApplyDamageData InDamageData) override;

protected:
	virtual void SetAttackPatternDatas() override;
	virtual TArray<FNPCAttackPatternSelectData>& GetAttackPatternDatas() override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Phase")
	TMap<EBossPhase, FName> PatternsNames;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Phase")
	TMap<EBossPhase, FNPCAttackPatternList> PhasePatternDatas;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Phase")
	TMap<EBossPhase, int32> PhaseHP;

	EBossPhase CurrentPhase = EBossPhase::First;

protected:
	virtual void HandleDeath() override;

	UFUNCTION()
	void StartNextPhase(class UAnimMontage* TargetMontage, bool IsProperlyEnded);
};
