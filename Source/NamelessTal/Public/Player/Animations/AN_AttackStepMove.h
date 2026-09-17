// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_AttackStepMove.generated.h"

/**
 * 
 */
UCLASS()
class NAMELESSTAL_API UAN_AttackStepMove : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	// 0보다 크면 캐릭터의 타별 기본 거리 대신 이 값을 사용
	UPROPERTY(EditAnywhere, Category = "AttackStep")
	float OverrideDistance = -1.0f;

	// 0보다 크면 캐릭터의 기본 지속시간 대신 이 값을 사용
	UPROPERTY(EditAnywhere, Category = "AttackStep")
	float OverrideDuration = -1.0f;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;
};
