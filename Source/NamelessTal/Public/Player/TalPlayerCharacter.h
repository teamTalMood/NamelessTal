// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TalPlayerCharacter.generated.h"

class UAnimMontage;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UStaticMeshComponent;
class UCapsuleComponent;
class ATalBase; //탈 전방선언
class ATalWeapon; //무기 전방선언

/**
 * 
 */
UCLASS()
class NAMELESSTAL_API ATalPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ATalPlayerCharacter();

	virtual void Tick(float DeltaTime) override;

protected: // CameraBoom , Follow Camera ( 카메라 암, 카메라 )
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;


protected: // (Player Input) Player Move, Use CameraMovement use Mouse (플레이어 이동 , 카메라 이동 마우스 사용)
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input") // Sword Attack Input
	UInputAction* SwordAttackAction;

	//F 상호작용 - 탈바꿈
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* TalAction;


	//탈 추가
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tal")
	TSubclassOf<ATalBase> DefaultTalClass;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Tal")
	TObjectPtr<ATalBase> CurrentTal;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")//탈바꿈 테스트용
	float InteractionDistance = 300.0f;

public: // Use Roll Dodge Input && 탈 관련 함수
	FORCEINLINE bool IsDodging() const { return bIsDodging; }

	//탈 관련 함수
	void EquipTal(ATalBase* NewTal);// 탈 착용
	ATalBase* GetCurrentTal() const;// 현재 탈 반환
	ATalBase* UnequipTal();//탈 벗고 반환
	void TakeTalFrom(ATalPlayerCharacter* TargetCharacter);//탈바꿈

protected: // Roll Dodge Input (회피 입력)
	UPROPERTY(EditAnywhere, Category= "Input")
	UInputAction* DodgeAction;

	UPROPERTY(EditAnywhere, Category = "Dodge")
	UAnimMontage* RollMontage;

	bool bIsInvincible = false;
	bool bIsMoving = false;
	bool bIsDodging = false;


	FVector LastMoveInputWorldDir = FVector::ForwardVector;
	FVector DodgeDir = FVector::ZeroVector;

	FTimerHandle DodgeTimerHandle;

	
	UPROPERTY(EditAnywhere, Category = "Dodge")
	float DodgeRotationSpeed = 540.0f;
	// Roll Dodge Input Function (Speed)

	UPROPERTY(EditAnywhere, Category = "Dodge")
	float NormalRotationInterpSpeed = 10.0f;

	void StartDodge(const FInputActionValue& Value);
	void EndDodge();

protected: // Player Move and use CameraMovement use Mouse 
	//(플레이어 이동, 카메라 이동 마우스 사용)
	void Move(const FInputActionValue& Value);

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void NotifyControllerChanged() override;

	void Look(const FInputActionValue& Value);

	virtual void BeginPlay() override;
	void Interact();//탈바꿈

public: // Property Camera (카메라 속성)
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	


protected:
	UPROPERTY(EditAnywhere, Category = "Input")
	float WalkSpeed = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Input")
	float SprintSpeed = 800.0f;

	UPROPERTY(EditAnywhere, Category = "Input")
	float SpeedInterpSpeed = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting = false;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* SprintAction;

	void StartSprint(const FInputActionValue& Value);
	void StopSprint(const FInputActionValue& Value);

	
////////////////////////////////////////////////////////////////////////////////weapon 구버전
protected: // Weapon 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon" , meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* WeaponMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* WeaponCollision;

protected: // Attack
	UPROPERTY(EditAnywhere, Category = "Attack")
	UAnimMontage* AttackMontage;

	void Attack(const FInputActionValue& Value);

public : // 무기 콜리젼 조절하는곳
	void EnableWeaponCollision();
	void DisableWeaponCollision();
	
protected: 
	UFUNCTION() // 무기 콜리전이 겹칠 때 호출
	void OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 한번 공격시 줄 데미지 양, 에디터에서 조절
	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackDamage = 10.0f;

	// 이번 공격(한번의 콜리전) 동안 맞은 대상들 목록
	// 콜리전 켜지면 비워주고 맞으면 추가해준다
	TArray<AActor*> HitActors;

	////////////////////////////////////////////////////////////////////////////////weapon 신버전
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<ATalWeapon> DefaultWeaponClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Weapon")
	ATalWeapon* EquippedWeapon = nullptr;

public:
	void EquipWeapon(ATalWeapon* NewWeapon);

	//AnimNorifyState에서 찾을 때 사용
	UFUNCTION(BlueprintPure, Category = "Weapon")
	ATalWeapon* GetEquippedWeapon() const;

};
