// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_Parry.generated.h"

/**
 * 
 */
UCLASS()
class NAMELESSTAL_API UAnimNotify_Parry : public UAnimNotify
{
	GENERATED_BODY()

public :
	//패링이 성공할 경우 발생하는 느려지는 애니메이션을 위한 용도

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EvenetReference) override;



};
