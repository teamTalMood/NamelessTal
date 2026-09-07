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

	// 가드 / 패링간 이동속도 받아오기
	float GetGuardWalkSpeed() const { return GuardWalkSpeed; }

	// 가드 소울 코스트 
	float GetGuardSoulCost() const { return GuardSoulCost; }

	// 가드 데미지 곱하는 함수 
	float GetGuardDamageMultiplier() const { return GuardDamageMultiplier;  }

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
	float GuardSoulCost = 12.0f; 
	// 가드 성공 시 넋 소모량

	UPROPERTY(EditAnywhere, Category = "Guard")
	float GuardWalkSpeed = 200.0f; // 가드 중 이동 속도


private:
	// 패링 중인지 
	bool bIsGuarding = false;

	// 가드 시작 시간
	float GuardStartTime = 0.0f; 
		
};
