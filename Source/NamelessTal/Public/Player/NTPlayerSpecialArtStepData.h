#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NTPlayerSpecialArtStepData.generated.h"

USTRUCT(BlueprintType)
struct FNTPlayerSpecialArtStepData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 타격 데이터의 고유 ID : 이부분은 DataTable의 RowName으로 사용할 것.
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArtStep")
	//FName StepID;

	// 이 타격이 속한 스페셜아츠의 ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArtStep")
	FName SpecialArtID;

	// 기술 내 명시적 타격 순번. 0부터 시작
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArtStep")
	int32 ComboCount = 0;

	// 이어서 실행할 타격의 StepID. Null이면 기술 종료
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArtStep")
	FName NextStepID;

	// 몬스터에게 가하는 HP 데미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArtStep")
	int32 HPDamage = 0;

	// 몬스터에게 가하는 넋 데미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArtStep")
	int32 SoulDamage = 0;

	// 몬스터에게 가하는 경직 수치. 누적되어 HitStun을 터뜨릴 수 있다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArtStep")
	int32 StaggerDamage = 0;

	// 이 단계 시작부터 공격판정 발생까지의 시간(초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArtStep")
	float CastTime = 0.f;

	// 공격판정 종료 후 다음 단계 또는 조작 가능한 상태로 넘어가기까지의 시간(초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArtStep")
	float RecoveryTime = 0.f;

	// 사용중 플레이어가 바라보는 방향으로 이동하는 거리(m). 제자리 공격이면 0
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArtStep")
	float MoveDistance = 0.f;

	// 플레이어가 사용할 몽타주 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArtStep")
	FName MontageName;

	// 해당 타격의 기획 의도 및 참고사항
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArtStep")
	FString Memo;
};
