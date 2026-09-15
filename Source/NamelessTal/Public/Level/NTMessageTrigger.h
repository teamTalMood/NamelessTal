// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Player/TalPlayerController.h"
#include "NTMessageTrigger.generated.h"

class UBoxComponent;

UCLASS()
class NAMELESSTAL_API ANTMessageTrigger : public AActor
{
	GENERATED_BODY()

public:
	ANTMessageTrigger();

protected:
	UPROPERTY(EditAnywhere, Category = "Message")
	ENTMessageStyle Style = ENTMessageStyle::Notice;

	UPROPERTY(EditAnywhere, Category = "Message")
	FText Title;

	UPROPERTY(EditAnywhere, Category = "Message", meta = (MultiLine = true))
	FText Body;

	UPROPERTY(EditAnywhere, Category = "Message", meta = (ClampMin = "0.1"))
	float Duration = 3.0f;

	// 한번 띄운뒤 콜리전을 꺼버릴지
	UPROPERTY(EditAnywhere, Category = "Message")
	bool bOnce = true;

	UPROPERTY(VisibleAnywhere, Category = "Message")
	TObjectPtr<UBoxComponent> TriggerBox;

	UFUNCTION()
	void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
