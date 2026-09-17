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

	// 몬스터에게 가하는 HP 데미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArtStep")
	int32 HPDamage = 0;

	// 몬스터에게 가하는 넋 데미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArtStep")
	int32 SoulDamage = 0;

	// 몬스터에게 가하는 경직 수치. 누적되어 HitStun을 터뜨릴 수 있다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArtStep")
	int32 StaggerDamage = 0;

	// 이 타격이 NPC를 밀어내는 속도 0이면 밀지않기
	// 타격 순간 플레이어가 계산하기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArtStep")
	float PushBackSpeed = 0.f;
};
