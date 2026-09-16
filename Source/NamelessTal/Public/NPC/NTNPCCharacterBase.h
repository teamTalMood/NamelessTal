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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn")
	TObjectPtr <class UNTLockOnTargetComponent> LockOnTargetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn")
	TObjectPtr <class UNTTalChiTargetComponent> TalChiTargetComponent;

protected:
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
	virtual const uint8 GetIsExhausting() override;
	virtual void TalChiNPC(FNPCTalchiData InTalchiData) override;

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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<class UAnimMontage> JumpMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	float JumpMontageSpeedRate = 2.0f;

protected:
	FAICharacterAttackFinished OnAICharacterAttackFinished;

	// 이번 공격(한번의 콜리전) 동안 맞은 대상들 목록
	// 콜리전 켜지면 비워주고 맞으면 추가해준다
	TArray<AActor*> HitActors;

	//EAttackDistance CurrentAttack = EAttackDistance::AttackNone;

	uint8 bIsAttacking : 1 = false;

	FVector AttackMoving = FVector(0, 0, 0);

	uint8 bISAttackMoving : 1 = false;

	FNPCCurrentAttackData CurrentAttackData;

	float CurrentStunRate = 0.0f;


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
	void StartStun();
	void StartDamaged();
	void OnDamageMontageEnd(class UAnimMontage* TargetMontage, bool IsProperlyEnded);
	void ReleaseStun();
	void EndExhausted();
	void StopCurrentPlayMontage();


	////////////////////////////////////////
	//
	//   Stat
	//
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	TObjectPtr <class UStatComponent> StatComponent;


	/////////////////////////////////////////
	//
	//   HP Component
	//
public:
	virtual void SetupCharacterWidget(class UNTHPBarWidget* InUserWidget);

protected:
	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly, Category = Widget, Meta = (AllowPrivateAcess = "True"))
	TObjectPtr<class UNTHPWidgetComponent> HpBar;



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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<class UAnimMontage> TalChiMontage;

protected:
	void OnTalChiMontageEnd(class UAnimMontage* TargetMontage, bool IsProperlyEnded);


	////////////////////////////////////////
	//
	//   Dead
	//
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<class UAnimMontage> DeathMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsDead = false;


	///// 죽을때 레그돌로 날아갈지 여부
	// 보스등 정해진 사망 연출이 있으면 BP에서끄기
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dead")
	bool bLaunchOnDeath = true;

	// 래그돌이 이 속도 아래로 떨어지면 멈춘것으로보고 물리 끄기
	UPROPERTY(EditAnywhere, Category = "Dead")
	float RagdollSettleSpeedThreshold = 20.0f;

	// 멈추었는지 확인하는 주기(초)
	UPROPERTY(EditAnywhere, Category = "Dead")
	float RagdollSettleCheckInterval = 0.3f;

	// 죽인 타격이 전달할 impulse. HandleDeath가 읽어감
	FVector PendingLaunchImpulse = FVector::ZeroVector;

	FTimerHandle RagdollSettleTimer;


protected:
	UFUNCTION()
	virtual void HandleDeath();

	//래그돌전환 + impulse 적용
	void StartDeathRagdoll(const FVector& Impulse);

	// 타이머로 호출한 후 멈추면 재워서 굳히기
	UFUNCTION()
	void CheckRagdollSettled();

};
