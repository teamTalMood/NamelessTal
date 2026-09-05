// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NPC/Interface/NTNPCCharacterBaseInterface.h"
#include "Interface/NTWeaponCollisionInterface.h"
#include "NTNPCCharacterBase.generated.h"

UENUM()
enum class EAttackDistance : uint8
{
	AttackNone = 0,
	AttackShort = 10,
	AttackMiddle = 20,
	AttackLong = 30
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

	UFUNCTION()
	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;


/////////////////////////////////////////
//
// INTNPCCharacterBaseInterface override
//
protected:
	TObjectPtr<class UBehaviorTreeComponent> BTComponent;


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

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AIPatrolRadius = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AIDetectRange = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AIAttackRange = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	float AITurnSpeed;




////////////////////////////////////////
//
//   Attack
//
protected:	// BlueprintEditable
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation)
	TObjectPtr<class UAnimMontage> ShortAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation)
	TObjectPtr<class UAnimMontage> MiddleAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Animation)
	TObjectPtr<class UAnimMontage> LongAttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float AttackDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float AttackRangeShort = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	float AttackRangeMiddle = 450.0f;

	// Weapon
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr <class UStaticMeshComponent> WeaponMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr <class UCapsuleComponent> WeaponCollision;

protected:	// BlueprintReadOnly
	FAICharacterAttackFinished OnAICharacterAttackFinished;

	// 이번 공격(한번의 콜리전) 동안 맞은 대상들 목록
	// 콜리전 켜지면 비워주고 맞으면 추가해준다
	TArray<AActor*> HitActors;

	EAttackDistance CurrentAttack = EAttackDistance::AttackNone;

	TMap<EAttackDistance, TObjectPtr<class UAnimMontage>> AttackMontages;


// INTWeaponCollisionInterface override
public:
	virtual void EnableWeaponCollision() override;
	virtual void DisableWeaponCollision() override;

// Attack
protected:
	void BeginAttack(const EAttackDistance InAttackType);
	void AttackHitCheck();
	void OnAttackMontageEnd(class UAnimMontage* TargetMontage, bool IsProperlyEnded);
	void EndAttack();

	// 무기 콜리전이 겹칠 때 호출
	UFUNCTION()
	void OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
