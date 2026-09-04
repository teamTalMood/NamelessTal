// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GuardComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NAMELESSTAL_API UGuardComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UGuardComponent();

	void StartGuard();
	void StopGuard();

	//패링 판정에 들어가는지 체크(TakeDamage에서 호출)
	bool TryParry() const;
	bool IsGuarding() const { return bIsGuarding; }


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Guard")
	float ParryWindow = 0.1f; 
	//가드 시작 이 후 이 시간안에 맞으면 패링으로 처리 

	UPROPERTY(EditAnywhere, Category = "Guard")
	float GuardDamageMultiplier = 0.3f; 
	// 일반 가드 시 받는 데미지(비율)

	UPROPERTY(EditAnywhere, Category = "Guard")
	float GuardSoulCost = 30.0f; 
	// 가드 성공 시 넋 소모량

private:
	// 패링 중인지 
	bool bIsGuarding = false;

	// 가드 시작 시간
	float GuardStartTime = 0.0f; 
		
};
