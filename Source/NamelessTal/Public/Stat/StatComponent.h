// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StatComponent.generated.h"


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
	float MaxHP = 100.0f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	float CurrentHP;

	// 다른 클래스에서 호출해야함
	void ApplyDamage(float HitDamage);
	void HealHp(float HealAmount);
	void RestoreFull(); // 화톳불용

	bool IsDead() const { return CurrentHP <= 0.0f; }
	


public: //넋 관련 설정
	UPROPERTY(EditAnywhere, Category = "Stat")
	float MaxSoul = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	float CurrentSoul;

	UPROPERTY(EditAnyWhere, Category = "Stat")
	float SoulRegenRate = 30.0f; //초당 회복량 (넋)

	UPROPERTY(EditAnywhere, Category = "Stat")
	//마지막 소모 후 회복 시작까지 시간 (1초)
	float SoulRegenDelay = 1.0f;

	void DrainSoul(float Amount);

	// 가드/스프린트 누르는 시간 막기
	void SetSoulRegenBlocked(bool bBlocked);
	bool HasSoul() const { return CurrentSoul > 0.0f; }

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* PCSoulTickFunction) override;

private:
	float LastSoulDrainTime = 0.0f;
	bool bSoulRegenBlocked = false;
};
