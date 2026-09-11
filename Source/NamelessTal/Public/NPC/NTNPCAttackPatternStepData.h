#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NTNPCAttackPatternStepData.generated.h"

USTRUCT(BlueprintType)
struct FNTNPCAttackPatternStepData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 스텝의 고유 ID. 기존 타격별 PatternID에 대응 : 이부분은 DataTable의 RowName으로 사용할 것.
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PatternStep")
	//FName StepID;

	// 이 스텝이 속한 패턴의 ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PatternStep")
	FName PatternID;

	// 개별 스텝의 이름. 관리용으로 필요할 때 사용
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PatternStep")
	FString StepName;

	// 패턴 내 타격 순번. 0부터 시작
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PatternStep")
	int32 ComboCount = 0;

	// 다음에 실행할 스텝의 ID. Null이면 마지막 스텝
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PatternStep")
	FName NextStepID;

	// 플레이어에게 가하는 HP 데미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PatternStep")
	int32 DamageHP = 0;

	// 플레이어에게 가하는 넋 데미지. 가드 시 70% 적용
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PatternStep")
	int32 DamageSoul = 0;

	// 가드 또는 패링 시 플레이어가 뒤로 밀려나는 거리(m)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PatternStep")
	float GuardBack = 0.f;

	// 이 타격을 패링했을 때 회심의 일격 연출이 가능한지 여부
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PatternStep")
	bool bCriticalEnable = false;

	// 해당 스텝 시작부터 공격판정 발생까지의 시간(초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PatternStep")
	float CastTime = 0.f;

	// 해당 스텝 진행 중 패턴을 취소시키기 위해 요구되는 경직 수치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PatternStep")
	int32 HitStun = 0;

	// 해당 스텝의 공격판정 종료 후 다음 스텝 또는 전투대기로 전환하기까지의 시간(초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PatternStep")
	float RecoveryTime = 0.f;

	// 해당 타격을 가드·패링·피격한 플레이어의 경직 유지시간(초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PatternStep")
	float PlayerRecoveryTime = 0.f;

	// 해당 스텝에 사용할 애니메이션 리소스 키
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PatternStep")
	FName AnimationClip;

	// 개별 스텝의 기획 의도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PatternStep")
	FString Memo;
};