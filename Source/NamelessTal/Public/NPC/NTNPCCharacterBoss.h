// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NPC/NTNPCCharacterBase.h"
#include "NPC/NTNPCAttackPatternData.h"
#include "NTNPCCharacterBoss.generated.h"

class ANTOkJangPan;

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
	virtual void Tick(float DeltaTime) override;

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
	TSubclassOf<ANTOkJangPan> OkJangPanClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Accessory")
	TObjectPtr<class UStaticMeshComponent> HeadringMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Accessory")
	TObjectPtr<class UStaticMeshComponent> HeadringHolderMesh;

	// HeadringMesh의 로컬 Z축(Yaw) 회전 속도(도/초). Phase 진입 시 PhaseHeadringRotationSpeed값으로 갱신됨
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Accessory")
	float HeadringRotationSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Accessory")
	TMap<EBossPhase, float> PhaseHeadringRotationSpeed;

protected:
	virtual void HandleDeath() override;

	UFUNCTION()
	void StartNextPhase(class UAnimMontage* TargetMontage, bool IsProperlyEnded);

	virtual void BeginAttack() override;

	// Top/Pants/Shoulder 다이나믹 머티리얼에 동일한 스칼라 파라미터를 한번에 적용
	void SetPartsScalarParameter(FName ParamName, float Value);
};
