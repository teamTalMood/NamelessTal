// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interface/NTWeaponCollisionInterface.h"
#include "Player/Interface/NTPlayerApplyDamageInterface.h"
#include "DdoKaebiFire/DdokaebiFire.h"
#include "TalPlayerCharacter.generated.h"


class UAnimMontage;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UStaticMeshComponent;
class UCapsuleComponent;
class ATalBase; //탈 전방선언
class UNTTalComponent; //탈 탈부착 전방선언
class UNTTalChiTargetComponent; //탈 탈취 전방선언
class ATalWeapon; //무기 전방선언
class UStatComponent; //스탯 전방선언
class UGuardComponent; // 가드/패링 클래스 전방선언
class UNTLockOnTargetComponent; //락온 전방선언
class UMotionWarpingComponent; //모션워핑 전방선언
class ADdokaebiFire; // 화톳불 전방선언
class UAnimSequenceBase; // 부활 모션을 위한 애니메이션베이스

//플레이어 공격 규칙 타입
UENUM(BlueprintType)
enum class EPlayerAttackPhase : uint8
{
	None,
	Startup,
	Active,
	Recovery,
	Grace //유예구간
};
UENUM(BlueprintType)
enum class EComboInputPhase : uint8
{
	Closed,
	BufferOnly,
	Executable
};
UENUM(BlueprintType)
enum class EBufferedCombatAction : uint8
{
	None,
	Attack,
	Dodge,
	Guard,
	Move
};

// 
struct FPlayerCurrentSpecialArtData
{
	FName NextStepID;
	int32 HPDamage;
	int32 SoulDamage;
	int32 StaggerDamage;
	float CastTime;
	float RecoveryTime;
	float MoveDistance;
	FName MontageName;
};


UCLASS()
class NAMELESSTAL_API ATalPlayerCharacter : public ACharacter, public INTWeaponCollisionInterface, public INTPlayerApplyDamageInterface
{
	GENERATED_BODY()

	////////////////////////////////////////
	//
	//   Character Basic : 캐릭터나 Actor 제공 클래스 override
	//
public:
	ATalPlayerCharacter();

	virtual void Tick(float DeltaTime) override;


protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

	virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	float ApplyDamage(AActor* DamageCauser, const float InDamage, const float InAttackDrain, const float InGuardDrain, const bool bCanParry);
	virtual float ApplyDamage(AActor* DamageCauser, const FPlayerApplyDamageData InDamageData) override;

	////////////////////////////////////////
	//
	//   Basic Components : 컴포넌트들
	//
protected:
	// CameraBoom , Follow Camera, Camera Root ( 카메라 암, 카메라 )
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAcess = "true"))
	TObjectPtr<class USceneComponent> CameraTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr <class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr <class UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> TalchiCameraTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> TalchiCamera;//1인칭 카메라(연출용)


	//카메라 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float CameraDistance = 350.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float CameraDistanceInterpSpeed = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "-89.0", UIMin = "89.0"))
	float MinCameraPitch = -60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "-89.0", UIMin = "89.0"))
	float MaxCameraPitch = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "0.0", UIMin = "100.0", UTMin = "0.0", UTMax = "100.0"))
	float HorizontalCameraSensitivity = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera", meta = (ClampMin = "0.0", UIMin = "100.0", UTMin = "0.0", UTMax = "100.0"))
	float VerticalCameraSensitivity = 50.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

	// 무기
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr <UStaticMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	TObjectPtr <UCapsuleComponent> WeaponCollision;



	////////////////////////////////////////
	//
	//   Input
	//
protected: // (Player Input) Player Move, Use CameraMovement use Mouse (플레이어 이동 , 카메라 이동 마우스 사용)
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr <UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr <UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr <UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category = "Input") // Sword Attack Input
		TObjectPtr <UInputAction> SwordAttackAction;

	//F 상호작용 - 탈바꿈
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr <UInputAction> TalAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr <UInputAction> DodgeAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr <UInputAction> SprintAction;

	//락온
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr <UInputAction> LockOnAction;
	//락온변경
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> SwitchLockOnTargetAction;

	void UpdateRotationMode();


	////////////////////////////////////////
	//
	//   카메라 무빙, 기본이동, 질주
	//
protected:
	UPROPERTY(EditAnywhere, Category = "Input")
	float WalkSpeed = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Input")
	float SprintSpeed = 800.0f;

	UPROPERTY(EditAnywhere, Category = "Input")
	float SpeedInterpSpeed = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsSprinting = false;

	//카메라 숨김설정(플레이어)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Visibility")
	float PlayerHideDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera|Visibility")
	float PlayerShowDistance = 130.0f;

	bool bHiddenByCameraDistance = false;

public: // Property Camera (카메라 속성)
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

protected: // Player Move and use CameraMovement use Mouse
	//(플레이어 이동, 카메라 이동 마우스 사용)
	void Move(const FInputActionValue& Value);

	virtual void NotifyControllerChanged() override;

	void Look(const FInputActionValue& Value);

	void StartSprint(const FInputActionValue& Value);

	void StopSprint(const FInputActionValue& Value);

	void UpdatePlayerCameraVisibility();

	////////////////////////////////////////
	//
	//   Dodging
	//
protected:
	UPROPERTY(EditAnywhere, Category = "Dodge")
	TObjectPtr <UAnimMontage> RollMontage_F;

	UPROPERTY(EditAnywhere, Category = "Dodge")
	TObjectPtr <UAnimMontage> RollMontage_B;

	UPROPERTY(EditAnywhere, Category = "Dodge")
	TObjectPtr <UAnimMontage> RollMontage_L;

	UPROPERTY(EditAnywhere, Category = "Dodge")
	TObjectPtr <UAnimMontage> RollMontage_R;

	UPROPERTY(EditAnywhere, Category = "Dodge")
	TObjectPtr <UAnimMontage> RollMontage_FL;

	UPROPERTY(EditAnywhere, Category = "Dodge")
	TObjectPtr <UAnimMontage> RollMontage_FR;

	UPROPERTY(EditAnywhere, Category = "Dodge")
	TObjectPtr <UAnimMontage> RollMontage_BL;

	UPROPERTY(EditAnywhere, Category = "Dodge")
	TObjectPtr <UAnimMontage> RollMontage_BR;

	FVector LastMoveInputWorldDir = FVector::ForwardVector;
	FVector DodgeDir = FVector::ZeroVector;
	FTimerHandle DodgeTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Dodge")
	float DodgeRotationSpeed = 540.0f;
	// Roll Dodge Input Function (Speed)

	UPROPERTY(EditAnywhere, Category = "Dodge")
	float NormalRotationInterpSpeed = 10.0f;

private:
	bool bIsInvincible = false;
	bool bIsMoving = false;
	bool bIsDodging = false;

public:
	FORCEINLINE bool IsDodging() const { return bIsDodging; }

protected: // Roll Dodge Input (회피 입력)
	UAnimMontage* SelectRollMontage(float Angle) const;
	void StartDodge(const FInputActionValue& Value);
	void EndDodge();



	////////////////////////////////////////
	//
	//   탈
	//
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tal")
	TSubclassOf<ATalBase> DefaultTalClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tal")
	TObjectPtr<UNTTalComponent> TalComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")//탈바꿈 테스트용
		float InteractionDistance = 300.0f;

public:
	void EquipTal(ATalBase* NewTal);// 탈 착용
	ATalBase* GetCurrentTal() const;// 현재 탈 반환
	ATalBase* UnequipTal();//탈 벗고 반환
	void TakeTalFrom(AActor* TargetActor);//탈바꿈
	void StartVictimTalchiMontage();//탈취 애니메이션 시작
	void TransferSelectedTal(); //탈 이전
	void GrabSelectedTalToHand();
	void EquipHeldTalToFace();

protected:
	void Interact();//탈바꿈

	UPROPERTY(Transient)
	TWeakObjectPtr<UNTTalChiTargetComponent> SelectedTalChiTarget;

	UPROPERTY(Transient)
	TObjectPtr<ATalBase> HeldStolenTal;

	////////////////////////////////////////
	//
	//   무기
	//
	////////////////////////////////////////////////////////////////////////////////weapon 구버전
protected: // Attack
	UPROPERTY(EditAnywhere, Category = "Attack")
	UAnimMontage* AttackMontage;

public: // 무기 콜리젼 조절하는곳
	virtual void EnableWeaponCollision() override;
	virtual void DisableWeaponCollision() override;



protected:
	void Attack(const FInputActionValue& Value);

	UFUNCTION() // 무기 콜리전이 겹칠 때 호출
		void OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 한번 공격시 줄 데미지 양, 에디터에서 조절
	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackDamage = 10.0f;

	// 공격에 사용되는 넋의 양
	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackSoulCost1 = 10.0f; // 1타

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackSoulCost2 = 11.0f; // 2타

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackSoulCost3 = 14.0f; // 3타

	UPROPERTY(EditAnywhere, Category = "Attack")
	float KillSoulReward = 20.0f;

	// 이번 공격(한번의 콜리전) 동안 맞은 대상들 목록
	// 콜리전 켜지면 비워주고 맞으면 추가해준다
	TArray<AActor*> HitActors;
	
	void StartAttackStep(int32 Step);//공격의 시작
	void StartNextCombo();//현재 공격의 다음 콤보를 시작

	void StartComboGrace();
	void EndComboGrace();

	void ClearAttackBuffer();//입력 초기화

	static constexpr int32 MaxBasicAttackCombo = 4;//최대 콤보 횟수

	////////////////////////////////////////////////////////////////////////////////weapon 신버전
	/*
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
	*/



	////////////////////////////////////////////////////////////////////////////
	///////		콤보
protected:
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OpenComboWindow();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void BeginComboExecution(int32 ExpectedStep);

	void ExecuteBufferedMove(const FVector2D& MovementVector);

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CloseComboWindow(int32 ExpectedStep);

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bCanCombo = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	int32 ComboIndex = 0;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	EPlayerAttackPhase AttackPhase = EPlayerAttackPhase::None;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	EComboInputPhase ComboInputPhase = EComboInputPhase::Closed;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	bool bBasicAttackActionLocked = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Combat")
	EBufferedCombatAction BufferedAction = EBufferedCombatAction::None;

	FVector2D BufferedMoveInput = FVector2D::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ComboGraceDuration = 0.15f;

	FTimerHandle ComboGraceTimer;


	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	////////////////////		특수기

protected:
	UPROPERTY(EditAnywhere, Category = "SpecialAttack")
	TObjectPtr<UInputAction> SpecialArtAction;

	// 특수기 이름표 -> 에셋 매핑
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpecialAttack")
	TMap<FName, TObjectPtr<UAnimMontage>> SpecialArtMontages;

	// 지금 실행중인 특수기 ID
	FName CurrentSpecialArtID;

	// 지금 특수기 사용중인가?
	bool bIsUsingSpecialArt = false;

	// 지금 장착한 탈로 특수기 몇번 썼는지
	int32 SpecialArtUseCount = 0;

	// 특수기 후딜 타이머
	FTimerHandle SpecialArtRecoveryTimer;

	// 특수기 돌진 중인지
	bool bIsSpecialArtMoving = false;

	// 특수기 돌진 속도 (초당 이동량)
	FVector SpecialArtMoveVelocity = FVector::ZeroVector;

	//특수기 돌진 종료 타이머
	FTimerHandle SpecialArtMoveTimer;

	// 지금 실행 중인 스텝의 데이터
	FPlayerCurrentSpecialArtData CurrentSpecialArtData;

	void UseSpecialAttack(const FInputActionValue& Value);

	void StartSpecialArtStep(FName StepID);

	// 몽타주 종료 콜백 함수
	void OnSpecialArtMontageEnd(class UAnimMontage* TargetMontage, bool IsProperlyEnded);
	void CancelSpecialArt();
	
	void EndSpecialArtRecovery();

	// 돌진기 관련 함수
	void EndSpecialArtMove();


	////////////////////////////////////////////////////////////////////////////
///////		스탯
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStatComponent> StatComponent;

public:
	UFUNCTION(BlueprintPure, Category = "Stat")
	UStatComponent* GetStatComponent() const
	{
		return StatComponent;
	}

public :
	void RecoverSoul(float AMount);

	////////////////////////////////////////////////////////////////////////////
	///////        가드 / 패링 시스템

protected:
	//가드 몽타주 
	UPROPERTY(EditAnywhere, Category = "Guard")
	UAnimMontage* GuardMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Guard",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UGuardComponent> GuardComponent;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> GuardAction;

	void StartGuard(const FInputActionValue& Value);
	void StopGuard(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, Category = "Guard")
	float ParrySlowMoTimeDilation = 0.15f;

	// 패링 성공 시 바라볼 대상
	TWeakObjectPtr<AActor> LastParriedTarget;

	//패링 후 대상 바라보는 회전 속도(도/초)
	UPROPERTY(EditAnywhere, Category = "Guard")
	float ParryFaceRotationSpeed = 720.0f;

	UPROPERTY(EditAnywhere, Category = "Guard")
	float ParrySlowMoRealDuration = 1.0f;

	bool bParrySlowMoActive = false;
	float ParrySlowMoEndRealTime = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Guard")
	UAnimMontage* ParryMontage;

	// 정면 120도 범위 절반
	UPROPERTY(EditAnywhere, Category = "Guard")
	float GuardFrontHalfAngle = 60.0f;

	bool IsWithinFrontTalGuardAngle(AActor* DamageCauser) const;

	// 가드,패링 애니메이션 끝난 후 시간(달리기,구르기,공격 금지)
	UPROPERTY(EditAnywhere, Category = "Guard")
	float ParryStaggerTime = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Guard")
	float GuardStaggerTime = 0.2f;

	// 넉백 0.2m
	UPROPERTY(EditAnywhere, Category = "Guard")
	float KnockBackDistance = 20.0f;

	// 이 데미지 기준 넉백 거리 적용 (강하거나 약하면 비례해서)
	UPROPERTY(EditAnywhere, Category = "Guard")
	float KnockBackRefDamage = 20.0f;



public:
	void TriggerParrySlowMo();

	void OnParryMontage(UAnimMontage* Montage, bool bInterrupted);

	void ApplyGuardKnockBack(float InAttackDamage);

	// 가드 중인지 ( 내부접근하기 Guardcomponent) 
	bool IsGuarding() const;


////////////////////////////////////////////////////////////////////////////
///////		락온 시스템
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn|Camera", meta = (ClampMin = "0.0", UTMin ="0.0"))
	float LockOnCameraInterpSpeed = 8.0f; //카메라 락온시 회전 보간속도

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn|Camera", meta = (ClampMin = "0.0", ClampMax = "1.0", UTMin = "0.0", UIMax = "1.0"))
	float LockOnTargetScreenYRatio = 0.25f; //카메라 높이 비율(위에서 3/4지점)

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LockOn")
	bool bIsLockOnActive = false;//락온 활성화 여부

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn", meta = (ClampMin = "0.0", UTMin = "0.0"))
	float MaxLockOnDistance = 1500.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "LockOn")
	TObjectPtr<UNTLockOnTargetComponent> CurrentLockOnTarget;

	void ToggleLockOn(const FInputActionValue& Value);

	UNTLockOnTargetComponent* FindBestLockOnTarget() const;

	void ClearLockOnTarget();

	void BeginLockOn();
	void UpdateLockOn(float DeltaTime);

	void SwitchLockOnTarget(const FInputActionValue& Value);
	bool TryGetValidLockOnCandidateScreenPosition(
		UNTLockOnTargetComponent* Candidate,
		const class APlayerController* PlayerController,
		FVector2D& OutScreenPosition
	) const;



	/////////////////////////////////////////////////////////////////
	//플레이어 상태
protected:
	UFUNCTION()
	void HandleDeath();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsDead = false;

	// 사망 후 리스폰까지 대기 시간 (나중에 연출넣을자리)
	UPROPERTY(EditAnywhere, Category = "State")
	float RespawnDelay = 3.0f;

	FTimerHandle RespawnTimerHandle;

	// 리스폰 시 시작 위치 ( 화톳불 아닐 시 맨 처음 화면)
	FTransform InitialSpawnTransform;

	// 마지막으로 사용한 화톳불 위치
	TWeakObjectPtr<ADdokaebiFire> LastBonfire;

	void RespawnAtBonfire();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")

	TObjectPtr<UAnimMontage> DeathMontage;//사망 애니메이션

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimSequenceBase> ReviveAnim;

	FTimerHandle ReviveTimerHandle;

	void EndRevive();


	virtual void Landed(const FHitResult& Hit) override;

	//착치시 
	UPROPERTY(EditAnywhere, Category = "Movement")
	float LandingSpeedMultiplier = 0.5f;

public :
	bool IsRealFalling() const { return bIsRealFalling; }

	void SetLastBonfire(ADdokaebiFire* Bonfire) { LastBonfire = Bonfire; }
protected : 
	// 낙하 높이 설정하기 
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MinFallHeightToPlayAnim = 200.0f;

	// 착치 후 이동/행동 막히는 시간 
	UPROPERTY(EditAnywhere, Category = "Movement")
	float LandingRecoverTime = 0.15f;

	void UpdateRealFalling();
	void EndLandingRecovery();

private :
	bool bIsRealFalling = false;
	bool bWasRealFallingThisAirtime = false;
	bool bIsLandingRecovering = false;
	FTimerHandle LandingRecoveryTimeHandle;


///////////////////////////////////////////////
////////	호리병

protected:
	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<UAnimMontage> FlaskMontage;

	UPROPERTY(EditAnywhere, Category = "Item")
	TObjectPtr<UInputAction> FlaskAction;

	UPROPERTY(EditAnywhere, Category = "Item")
	int32 FlaskInitialCount = 3;

	UPROPERTY(EditAnywhere, Category = "Item")
	float FlaskHealPercent = 0.4f;

	UPROPERTY(EditAnywhere, Category = "Item")
	int32 FlaskMaxCount = 4;

	//  호리병 사용 중 이동 속도 배율
	UPROPERTY(EditAnywhere, Category = "Item")
	float FlaskMoveSpeedMultiplier = 0.333f;

	UPROPERTY(BlueprintReadOnly, Category = "Item")
	int32 FlaskCount = 0;


	UPROPERTY(BlueprintReadOnly, Category = "Item")
	bool bIsUsingFlask = false;

	void UseFlaskItem(const FInputActionValue& Value);
	void CancelFlaskUse();
	void OnFlaskMontageEnded(UAnimMontage* Montage, bool bInterrupted);

public:
	// Montage의 노티파이 호출 - 실제 회복 적용 시점은 여기입니다
	UFUNCTION(BlueprintCallable, Category = "Item")
	void ApplyFlaskHeal();

	// 도깨비불(모닥불)에서 호출 - 최대치로 충전시키기
	void RechargeFlask();


};
