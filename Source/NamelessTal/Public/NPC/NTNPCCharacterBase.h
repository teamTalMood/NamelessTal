// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NPC/Interface/NTNPCCharacterBaseInterface.h"
#include "Interface/NTWeaponCollisionInterface.h"
#include "NTNPCCharacterBase.generated.h"

class ATalBase;
class UNTTalComponent;

struct FNPCCurrentAttackData
{
	FName NextStepID;
	int32 DamageHP;
	int32 DamageSoul;
	float GuardBack;
	uint8 bCriticalEnable;
	int32 HitStun;
	float RecoveryTime;
	FName AnimationClip;
};

UCLASS()
class NAMELESSTAL_API ANTNPCCharacterBase : public ACharacter, public INTNPCCharacterBaseInterface, public INTWeaponCollisionInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANTNPCCharacterBase();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


/////////////////////////////////////////
//
// Basic Components
//
protected:
	TObjectPtr<class UBehaviorTreeComponent> BTComponent;
	TObjectPtr<class UBlackboardComponent> BBComponent;

	// Weapon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr <class UStaticMeshComponent> WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr <class UCapsuleComponent> WeaponCollision;

	// 무기 콜리전이 겹칠 때 호출
	UFUNCTION()
	void OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


/////////////////////////////////////////
//
// INTNPCCharacterBaseInterface override
//
public:
	virtual float GetAIPatrolRadius() override;
	virtual float GetAIDetectRange() override;
	virtual float GetAIAttackRange() override;
	virtual float GetAITurnSpeed() override;
	virtual void AttackByAI() override;
	virtual void SetAIAttackDelegate(const FAICharacterAttackFinished& InOnAttackFinished) override;
	virtual float ApplyDamage(AActor* DamageCauser, const float InDamage, const float InDrain, const bool bIsParry) override; //Legacy
	virtual float ApplyDamage(AActor* DamageCauser, const FNPCApplyDamageData InDamageData) override;
	virtual uint8 GetAttackDirection() override;
	virtual TArray<FNPCAttackPatternSelectData>& GetAttackPatternDatas() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AIPatrolRadius = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AIDetectRange = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AIAttackRange = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AITurnSpeed;

	TArray<FNPCAttackPatternSelectData> AttackPatternDatas;



////////////////////////////////////////
//
//   Attack
//
protected:	// BlueprintEditable
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TMap<FName, TObjectPtr<class UAnimMontage>> AttackMontages;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	TMap<EAttackDistance, uint8> AttackDirections;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	FName PatternUserID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack")
	float AttackDistance = 150.f;

protected:	// BlueprintReadOnly
	FAICharacterAttackFinished OnAICharacterAttackFinished;

	// 이번 공격(한번의 콜리전) 동안 맞은 대상들 목록
	// 콜리전 켜지면 비워주고 맞으면 추가해준다
	TArray<AActor*> HitActors;

	EAttackDistance CurrentAttack = EAttackDistance::AttackNone;

	uint8 bIsAttacking : 1 = false;

	FVector AttackMoving = FVector(0, 0, 0);

	uint8 bISAttackMoving : 1 = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<class UAnimMontage> JumpMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	float JumpMontageSpeedRate = 2.0f;

	FNPCCurrentAttackData CurrentAttackData;


// INTWeaponCollisionInterface override
public:
	virtual void EnableWeaponCollision() override;
	virtual void DisableWeaponCollision() override;

// Attack
protected:
	void MoveToAttackPoint();
	void BeginAttack();
	void OnAttackMontageEnd(class UAnimMontage* TargetMontage, bool IsProperlyEnded);
	void EndAttack();
	void OnJumpMontageEnd(class UAnimMontage* TargetMontage, bool IsProperlyEnded);





////////////////////////////////////////
//
//   Damaged
//
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<class UAnimMontage> DamageMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	float DamageSpeedRate = 1.0f;


protected:
	void OnDamageMontageEnd(class UAnimMontage* TargetMontage, bool IsProperlyEnded);


////////////////////////////////////////
//
//   Stat
//
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	TObjectPtr <class UStatComponent> StatComponent;


////////////////////////////////////////
//
//   Exhausted
//
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
	float ExhaustedDuration = 0;

	uint8 bIsExhausting : 1 = false;


////////////////////////////////////////
//
//   Tal
//
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tal")
	TObjectPtr<UNTTalComponent> TalComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tal")
	TSubclassOf<ATalBase> DefaultTalClass;


////////////////////////////////////////
//
//   Dead
//
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<class UAnimMontage> DeathMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsDead = false;

protected:
	UFUNCTION()
	void HandleDeath();
};
