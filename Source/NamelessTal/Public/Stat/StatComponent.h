// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPChanged, float, NewHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSoulChanged, float, NewSoul, float, MaxSoul);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NAMELESSTAL_API UStatComponent : public UActorComponent
{
	GENERATED_BODY()



protected:
	virtual void BeginPlay() override;

public:
	// Sets default values for this component's properties
	UStatComponent();

public: // 체력 관련 설정
	UPROPERTY(EditAnywhere, Category="Stat")
	float MaxHP = 200.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	float CurrentHP;

	UPROPERTY(BlueprintAssignable, Category = "Stat|Events")
	FOnDeath OnDeath; //죽음

	UPROPERTY(BlueprintAssignable, Category = "Stat|Events")
	FOnHPChanged OnHPChanged; // 체력 변화


	UPROPERTY(BlueprintAssignable, Category = "Stat|Events")
	FOnSoulChanged OnSoulChanged; // 넋 변화

	// 다른 클래스에서 호출해야함
	void ApplyDamage(float HitDamage);
	void HealHp(float HealAmount);
	void RestoreFull(); // 화톳불용

	bool IsDead() const { return CurrentHP <= 0.0f; }

	void RestoreSoul();

	// 적 처치 보상
	void RecoverSoul(float Amount);


public: //넋 관련 설정
	UPROPERTY(EditAnywhere, Category = "Stat")
	float MaxSoul = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	float CurrentSoul;

	// 넋 회복 속도
	UPROPERTY(EditAnywhere, Category = "Stat")
	float SoulRegenRate = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Stat")
	//마지막 소모 후 회복 시작까지 시간 (1초)
	float SoulRegenDelay = 1.0f;

	// 넋 고갈 시

	// 30 이상 시 고갈 상태 해제
	UPROPERTY(EditAnyWhere, Category = "Stat")
	float SoulExhaustedUnlock = 30.0f;

	// 고갈 중 회복 배율
	UPROPERTY(EditAnyWhere, Category = "Stat")
	float SoulExhaustedRegenMultiplier = 1.5f;

	// 고갈 중 이동 속도 배율
	UPROPERTY(EditAnyWhere, Category = "Stat")
	float SoulExhaustedMoveSpeedMultiplier = 0.6f;




	void DrainSoul(float Amount);

	// 가드/스프린트 누르는 시간 막기
	void SetSoulRegenBlocked(bool bBlocked);
	bool HasSoul() const { return CurrentSoul > 0.0f; }


	// 넋 소모하는 행동이 내가 가진 넋보다 많은지 체크하기
	bool HasEnoughSoul(float Cost) const { return CurrentSoul >= Cost; }

	// 넋이 바닥났다가 30이상이냐 < 체크
	bool IsSoulExhausted() const { return bIsSoulExhausted; }

	bool IsGroggy() const { return bIsGroggy; }

	// 전투중인지 확인하기
	bool IsInCombat() const;

	// 그로기 중 피격 시 호출 -> 그로기 조기 해제
	void NotifyHitDuringGroggy();

	// 비전투 상태 시 넋 회복
	void NotifyCombatAction();

	// 넋 고갈 시 그로기 지속 시간
	UPROPERTY(EditAnywhere, Category = "Stat")
	float GroggyDuration = 3.0f;

	// 그로기 해제(피격 또는 시간경과) 시 즉시 회복 되는 넋
	UPROPERTY(EditAnywhere, Category = "Stat")
	float GroggyRecoverSoul = 20.0f;

	// 마지막 전투 행동 후 비전투로 바뀌는 시간
	UPROPERTY(EditAnywhere, Category = "Stat")
	float CombatCooldown = 5.0f;

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* PCSoulTickFunction) override;

	// 지금 넋이 초당 몇 회복되어야하는지 계산하는 함수
	float GetSoulRegenRateForCurrentHP() const;

	float GetSoulRegenRate() const;

private:

	void EndGroggy();

	bool bIsGroggy = false;
	float GroggyEndTime = 0.0f;

	float LastSoulDrainTime = 0.0f;
	bool bSoulRegenBlocked = false;
	bool bIsSoulExhausted = false;
	bool bDeathBroadcasted = false;//중복 방송 방지



public://Getter
	UFUNCTION(BlueprintPure, Category = "Stat")
	float GetCurrentHP() const { return CurrentHP; }
	UFUNCTION(BlueprintPure, Category = "Stat")
	float GetMaxHP() const { return MaxHP; }
	UFUNCTION(BlueprintPure, Category = "Stat")
	float GetCurrentSoul() const { return CurrentSoul; }
	UFUNCTION(BlueprintPure, Category = "Stat")
	float GetMaxSoul() const { return MaxSoul; }


};
