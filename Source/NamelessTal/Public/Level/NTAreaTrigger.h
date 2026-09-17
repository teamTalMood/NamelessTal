#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NTAreaTrigger.generated.h"

class UBoxComponent;
class UTexture2D;

/**
 * 지역 도착 알림 트리거.
 * 플레이어가 박스 안에 있는 동안 알림이 계속 떠 있고, 나가면 사라진다.
 * 한 번 뜨고 마는 ANTMessageTrigger와 달리 드나들 때마다 반복된다.
 */
UCLASS()
class NAMELESSTAL_API ANTAreaTrigger : public AActor
{
	GENERATED_BODY()

public:
	ANTAreaTrigger();

protected:
	// 지역 이름. 예: 사찰 입구
	UPROPERTY(EditAnywhere, Category = "Area")
	FText AreaName;

	// 지역 설명. 여러 줄 입력 가능
	UPROPERTY(EditAnywhere, Category = "Area", meta = (MultiLine = true))
	FText Description;

	// 지역마다 다른 그림. 비워두면 이미지 칸이 접힌다
	UPROPERTY(EditAnywhere, Category = "Area")
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(VisibleAnywhere, Category = "Area")
	TObjectPtr<UBoxComponent> TriggerBox;

	UFUNCTION()
	void OnAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnAreaEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};