// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NPC/NTNPCCharacterBase.h"
#include "NPC/NTNPCAttackPatternData.h"
#include "Title/NTSavePointEnum.h"
#include "NTNPCCharacterBoss.generated.h"

class ANTOkJangPan;

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

	// BossRing 소켓에 붙는 빈 피벗. HeadringMesh는 피벗의 자식으로 붙고, 회전은 이 피벗에서 함
	// (HeadringMesh 자체의 피봇이 메시 중심이 아니라 메시를 직접 돌리면 어긋남)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Accessory")
	TObjectPtr<class USceneComponent> HeadringPivotComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Accessory")
	TObjectPtr<class UStaticMeshComponent> HeadringMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Accessory")
	TObjectPtr<class UStaticMeshComponent> HeadringHolderMesh;

	// HeadringPivotComponent의 로컬 Y축(Pitch) 회전 속도(도/초). Phase 진입 시 PhaseHeadringRotationSpeed값으로 갱신됨
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Accessory")
	float HeadringRotationSpeed = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Accessory")
	TMap<EBossPhase, float> PhaseHeadringRotationSpeed;

protected:
	virtual void HandleDeath() override;

	UFUNCTION()
	void StartNextPhase();

	// StatComponent->OnHPChanged 바인딩. 보스 HP 변경을 게임모드로 전달
	UFUNCTION()
	void NotifyGameModeHPChanged(float NewHP, float MaxHP);

	// StatComponent->OnSoulChanged 바인딩. 보스 넋 변경을 게임모드로 전달
	UFUNCTION()
	void NotifyGameModeSoulChanged(float NewSoul, float MaxSoul);

	virtual void BeginAttack() override;

	// Top/Pants/Shoulder 다이나믹 머티리얼에 동일한 스칼라 파라미터를 한번에 적용
	void SetPartsScalarParameter(FName ParamName, float Value);

	void StartBossSequence(UAnimMontage* TargetMontage, bool IsProperlyEnded);

	virtual void SetupCharacterWidget(class UNTHPBarWidget* InUserWidget) override;

	FTimerHandle PhaseStartDelayTimer;
};
