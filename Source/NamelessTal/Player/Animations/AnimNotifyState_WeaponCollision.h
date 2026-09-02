// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_WeaponCollision.generated.h"

/**
 * 
 */
UCLASS()
class NAMELESSTAL_API UAnimNotifyState_WeaponCollision : public UAnimNotifyState
{
	GENERATED_BODY()
	
public :
	// 구간 시작 시 한번 호출
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation, float TotalDuration, 
		const FAnimNotifyEventReference& EventReference) override;

	// 구간 끝날 시 한번 호출
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation, 
		const FAnimNotifyEventReference& EventReference) override;
};
