#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TalBase.h"
#include "NTPlayerSpecialArtData.generated.h"

USTRUCT(BlueprintType)
struct FNTPlayerSpecialArtData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 스페셜아츠의 고유 ID : 이부분은 DataTable의 RowName으로 사용할 것.
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArt")
	//FName SpecialArtID;

	// 플레이어에게 표시할 기술 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArt")
	FString SpecialArtName;

	// 기술 설명. 문자열 테이블을 사용한다면 해당 키값으로 대체
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArt")
	FString Description;
	
	// 이 스페셜아츠를 사용할 때 플레이어가 사용할 무기 ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArt")
	ETalStyle WeaponID;
	
	// 사용 시 최초로 실행할 타격 데이터의 ID. SpecialArtStepTable을 참조
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArt")
	TSoftObjectPtr<UAnimMontage> Montage;

	// 사용할 수 있는 횟수(탈취시마다 초기화)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArt")
	int32 UseLimit = 0;

	// 기술 1회 사용 시 넋 소모량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArt")
	int32 SoulCost = 0;

	// 장착창·HUD 등에 사용할 아이콘 리소스 키
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArt")
	FName Icon;

	// 기획 의도 및 참고사항
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialArt")
	FString Memo;
};
