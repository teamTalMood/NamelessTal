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
	bool TryParry() const;

	//패링 판정에 들어가는지 체크(TakeDamage에서 호출)
	bool IsGuarding() const { return bIsGuarding; }

	// 버튼을 누르고 있는지 (애니메이션용)
	bool IsGuardInputHeld() const { return bIsGuardInputHeld; }

	// 가드 / 패링간 이동속도 받아오기
	float GetGuardWalkSpeed() const { return GuardWalkSpeed; }

	// 가드 소울 코스트 
	float GetGuardSoulCost() const { return GuardSoulCostPersent; }

	// 가드 성공시 공격, 구르기 스프린트 차단하기
	bool IsStaggered() const;
	void StartStagger(float Duration);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// 가드 입력 후 실제 가드상태가 되기까지 딜레이 시간
	UPROPERTY(EditAnywhere, Category = "Guard")
	float GuardActivationDelay = 0.1f;

	// 패링 판정 기본 시간 ( 재입력 패널티 적용 전 기본 시간)
	UPROPERTY(EditAnywhere, Category = "Guard")
	float ParryWindowBase= 0.2f; 

	// 재입력 1회당 줄어드는 패링 윈도우 시간
	UPROPERTY(EditAnywhere, Category = "Guard")
	float ParryWindowStackDec = 0.02f;

	// 최대 패링 재입력 중첩 (0.2 - 0.02*5 = 0.1이 최대하한선)
	UPROPERTY(EditAnywhere, Category =  "Guard")
	int32 MaxParryStackCount = 5;

	// 가드 해제 후 이 시간 이내 재입력 시 패링 윈도우 단축 스택 쌓임
	UPROPERTY(EditAnywhere, Category = "Guard")
	float ParryWindowRetryLimit = 0.5f;

	// 가드 성공 시 넋 소모량 ( 적 공격(넋값) * 이 값)
	UPROPERTY(EditAnywhere, Category = "Guard")
	float GuardSoulCostPersent = 0.7f;

	UPROPERTY(EditAnywhere, Category = "Guard")
	float GuardWalkSpeed = 50.0f; // 가드 중 이동 속도 (0.5m/s)


private:
	void ActivateGuard();

	// 패링 중인지 
	bool bIsGuarding = false;

	// 가드 시작 시간
	float GuardStartTime = 0.0f; 
	
	// 이번 가드에 적용되는 실제 패링 윈도우(스택 반영값)
	float CurrentParryWindow = 0.0f;

	//가드 해제 시각(재입력 패널티 판단용)
	float LastGuardStopTime = -100.0f;

	//현재 쌓인 재입력 스택
	int32 ParryWindowStackCount = 0;

	FTimerHandle GuardActivationTimerHandle;

	// 가드 후 (공격/구르기/스프린트 차단 시간)
	float StaggerEndTime = -100.0f;

	// 입력즉시 true 떼면 즉시 false
	bool bIsGuardInputHeld = false;


};
