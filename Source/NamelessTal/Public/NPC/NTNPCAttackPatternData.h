#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "NTNPCAttackPatternData.generated.h"

USTRUCT(BlueprintType)
struct FNTNPCAttackPatternData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 패턴의 고유 ID : 이부분은 DataTable의 RowName으로 사용할 것.
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pattern")
	//FName PatternID;

	// 해당 패턴을 사용하는 몬스터의 ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pattern")
	FName PatternUser;

	// 패턴 전체의 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pattern")
	FString PatternName;

	// 패턴 시작 시 실행할 첫 번째 스텝의 ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pattern")
	FName FirstStepID;

	// 동일 패턴을 다시 사용할 수 있기까지의 시간(초)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pattern")
	float CoolDown = 0.f;

	// AI가 패턴을 선택할 수 있는 몬스터와 플레이어 사이의 최대거리(m)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pattern")
	float PatternRange = 0.f;

	// 사용 가능한 패턴 중 해당 패턴을 선택하기 위한 가중치
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pattern")
	int32 PatternWeight = 0;

	// 패턴 전체의 기획 의도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pattern")
	FString Memo;
};