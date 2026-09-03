// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NPC/Interface/NTNPCCharacterBaseInterface.h"
#include "Interface/NTWeaponCollisionInterface.h"
#include "NTNPCCharacterBase.generated.h"

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
public:
	virtual float GetAIPatrolRadius() override;
	virtual float GetAIDetectRange() override;
	virtual float GetAIAttackRange() override;
	virtual float GetAITurnSpeed() override;
	virtual void AttackByAI() override;
	virtual void SetAIAttackDelegate(const FAICharacterAttackFinished& InOnAttackFinished) override;



////////////////////////////////////////
//
//   Attack
//
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	TObjectPtr<class UAnimMontage> ComboAttackMontage;

	// 한번 공격시 줄 데미지 양, 에디터에서 조절
	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackDamage = 10.0f;

private:
	FAICharacterAttackFinished OnAICharacterAttackFinished;

	// 이번 공격(한번의 콜리전) 동안 맞은 대상들 목록
	// 콜리전 켜지면 비워주고 맞으면 추가해준다
	TArray<AActor*> HitActors;

	// Weapon
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr <class UStaticMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack", meta = (AllowPrivateAccess = "true"))
	TObjectPtr <class UCapsuleComponent> WeaponCollision;


// INTWeaponCollisionInterface override
public:
	virtual void EnableWeaponCollision() override;
	virtual void DisableWeaponCollision() override;

// Attack
private:
	void AttackHitCheck();
	void ProcessComboCommand();

	// 무기 콜리전이 겹칠 때 호출
	UFUNCTION()
	void OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);



////////////////////////////////////////
//
//   Damage
//






};
