// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/TalPlayerCharacter.h"
#include "Variant_Combat/Interfaces/CombatDamageable.h"

#include "MCLOG.h"

#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"

#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Controller.h"


#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"

#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Curves/CurveFloat.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"

#include "Engine/LocalPlayer.h"

#include "TimerManager.h"

#include "Kismet/GameplayStatics.h"

//tal 헤더들 추가
#include "TalBase.h"
#include "Components/NTTalComponent.h"
#include "NPC/Components/NTTalChiTargetComponent.h"

//무기헤더
#include "Player/TalWeapon.h"

//스탯 컴포넌트
#include "Stat/StatComponent.h"

// 패링/가드 컴포넌트
#include "Guard/GuardComponent.h"

// 히트 스톱용
#include "Components/NTCombatFeelComponent.h"

//락온 추가
#include "NPC/Components/NTLockOnTargetComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/UObjectIterator.h"

// 적 애니메이션 ( 맞춤 패링 )
#include "NPC/Interface/NTNPCCharacterBaseInterface.h"
// 모션워핑
#include "MotionWarpingComponent.h"

//Revive Setting
#include "DdoKaebiFire/DdokaebiFire.h"

#include "Game/NTGameDataSingleton.h"

// 화롯불 안쓰고 죽을때를 대비해 처음으로 돌아갈때를 대비하기 위해 사용
#include "NamelessTalGameMode.h" 

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"

#include "NPC/NTNPCCharacterBase.h"

ATalPlayerCharacter::ATalPlayerCharacter()
{
	//Cmaera Component Set
	CameraTarget = CreateDefaultSubobject<USceneComponent>(TEXT("CameraTarget"));
	CameraTarget->SetupAttachment(RootComponent);
	CameraTarget->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));

	//Camera Arm Set
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(CameraTarget);
	CameraBoom->TargetArmLength = CameraDistance;
	CameraBoom->bUsePawnControlRotation = true;

	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 12.0f;
	CameraBoom->CameraLagMaxDistance = 100.0f;
	CameraBoom->bUseCameraLagSubstepping = true;

	// Follow Camera Set
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	//Talchi Camera Set
	TalchiCameraTarget = CreateDefaultSubobject<USceneComponent>(TEXT("TalchiCameraTarget"));
	TalchiCameraTarget->SetupAttachment(RootComponent);
	TalchiCameraTarget->SetRelativeLocation(FVector(20.0f, 0.0f, 160.0f));

	TalchiCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TalchiCamera"));
	TalchiCamera->SetupAttachment(TalchiCameraTarget);
	TalchiCamera->bUsePawnControlRotation = false;
	TalchiCamera->bAutoActivate = false;

	// For Roll Dodge Input
	PrimaryActorTick.bCanEverTick = true;

	// 무기 Mesh Set
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(GetMesh(), TEXT("WeaponSocket_R"));
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("WeaponCollision"));
	WeaponCollision->ComponentTags.Add(TEXT("WeaponCollision"));
	WeaponCollision->SetupAttachment(WeaponMesh);
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	WeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &ATalPlayerCharacter::OnWeaponOverlap);

	LeftTalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftTalMesh"));
	LeftTalMesh->SetupAttachment(GetMesh(), LeftTalSocketName);
	LeftTalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//이동방향으로 캐릭터 회전
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	StatComponent = CreateDefaultSubobject<UStatComponent>(TEXT("StatComponent"));
	GuardComponent = CreateDefaultSubobject<UGuardComponent>(TEXT("GuardComponent"));

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));


	TalComponent = CreateDefaultSubobject<UNTTalComponent>(TEXT("TalComponent"));

	CombatFeelComponent = CreateDefaultSubobject<UNTCombatFeelComponent>(
		TEXT("CombatFeelComponent"));
}


void ATalPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	

	// 카운터 창 만료시 (슬로모션과 무관하게 독립 판정)
	if (bParryCounterReady && GetWorld()->GetRealTimeSeconds()
		>= ParryCounterEndRealTime)
	{
		CloseParryCounterWindow();
	}

	if (CombatFeelComponent && CombatFeelComponent->IsParrySlowMoActive()
		&& LastParriedTarget.IsValid())
	{
		const FVector ToTarget = (LastParriedTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		if (!ToTarget.IsNearlyZero())
		{
			const FRotator TargetRotation = ToTarget.Rotation();

			SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation,
				DeltaTime, ParryFaceRotationSpeed / 90.0f));

		}
	}

	//카메라 거리보간
	const float SafeCameraDistance = FMath::Max(0.0f, CameraDistance);
	const float SafeInterpSpeed = FMath::Max(0.0f, CameraDistanceInterpSpeed);

	//CameraBoom->TargetArmLength = FMath::FInterpTo(
	//	CameraBoom->TargetArmLength,
	//	SafeCameraDistance,
	//	DeltaTime,
	//	SafeInterpSpeed
	//);

	//플레이어 표시 여부
	UpdatePlayerCameraVisibility();

	//카메라 Pitch제한
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager)
		{
			const float SafePitchMin = FMath::Clamp(FMath::Min(MinCameraPitch, MaxCameraPitch), -89.0f, 89.0f);
			const float SafePitchMax = FMath::Clamp(FMath::Max(MinCameraPitch, MaxCameraPitch), -89.0f, 89.0f);

			CameraManager->ViewPitchMin = SafePitchMin;
			CameraManager->ViewPitchMax = SafePitchMax;
		}
	}
	
	const bool bParryCameraMoving =
		CombatFeelComponent &&
		CombatFeelComponent->IsParryCameraMoving();

	UpdateTalchiCamera();
	UpdateTalchiMaskApproach();
	UpdateTalchiMaskPreview();

	if (bIsLockOnActive && !bParryCameraMoving && !bIsTalchiInProgress)
	{
		UpdateLockOn(DeltaTime);
	}

	UpdateRotationMode();
	UpdateAttackStepMove(DeltaTime);
	UpdateRealFalling();
	//if (bIsDodging) 이거 때문에 회전이 강제되어 구르기와 캐릭터의 회전이 되지 않았음
	//{
	//	const FRotator TargetRotation = DodgeDir.Rotation();
	//	SetActorRotation(FMath::RInterpConstantTo(GetActorRotation(), TargetRotation,
	//		DeltaTime, DodgeRotationSpeed));
	//}
	//else if (Controller != nullptr)
	//{
	//	const FRotator TargetRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
	//	SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation,
	//		DeltaTime, NormalRotationInterpSpeed));
	//}

	// Sprint Speed Interpolation
	float TargetSpeed = GuardComponent->IsGuarding() ? GuardComponent->GetGuardWalkSpeed()
		: (bIsSprinting ? SprintSpeed : WalkSpeed);

	if (bIsUsingFlask)
	{
		TargetSpeed *= FlaskMoveSpeedMultiplier;
	}

	if (StatComponent->IsSoulExhausted())
	{
		TargetSpeed *= StatComponent->SoulExhaustedMoveSpeedMultiplier;
	}

	GetCharacterMovement()->MaxWalkSpeed =
		FMath::FInterpTo(GetCharacterMovement()->MaxWalkSpeed, TargetSpeed, DeltaTime, SpeedInterpSpeed);

	if (bIsTalchiInProgress)
	{
		UAnimInstance* AnimInstance =
			GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;

		const bool bMontageActive =
			AnimInstance
			&& IsValid(ActiveTalchiMontage)
			&& AnimInstance->Montage_IsActive(ActiveTalchiMontage);

		if (!bMontageActive)
		{
			FinishTalchi();
		}
	}
	if (bIsTalchiInProgress)
	{
		TalchiSlowElapsed += DeltaTime;

		const float Interval =
			FMath::Max(0.01f, TalchiSlowUpdateInterval);

		if (TalchiSlowElapsed >= Interval)
		{
			TalchiSlowElapsed = 0.0f;
			UpdateTalchiSlow();
		}
	}

	UpdateTalchiPrompt(DeltaTime);
}

UAnimMontage* ATalPlayerCharacter::SelectRollMontage(float Angle) const
{
	if (Angle >= -22.5f && Angle < 22.5f)   return RollMontage_F;
	if (Angle >= 22.5f && Angle < 67.5f)    return RollMontage_FR;
	if (Angle >= 67.5f && Angle < 112.5f)   return RollMontage_R;
	if (Angle >= 112.5f && Angle < 157.5f)  return RollMontage_BR;
	if (Angle >= 157.5f || Angle < -157.5f) return RollMontage_B;
	if (Angle >= -157.5f && Angle < -112.5f) return RollMontage_BL;
	if (Angle >= -112.5f && Angle < -67.5f)  return RollMontage_L;
	return RollMontage_FL; // -67.5 ~ -22.5
}

void ATalPlayerCharacter::StartDodge(const FInputActionValue& Value)
{
	if (bIsTalchiInProgress)
	{
		return;
	}
	// 구르기 자체가 불가능한 상태
	if (bIsDead
		|| GuardComponent->IsStaggered()
		|| StatComponent->IsGroggy()
		|| bIsLandingRecovering
		|| bIsUsingFlask
		|| bIsUsingSpecialArt
		|| bIsDodging)
	{
		return;
	}

	if (bBasicAttackActionLocked)
	{
		// 저장 구간: 첫 입력일 때만 구르기 예약
		if (ComboInputPhase == EComboInputPhase::BufferOnly)
		{
			if (BufferedAction == EBufferedCombatAction::None)
			{
				BufferedAction = EBufferedCombatAction::Dodge;
			}

			return;
		}

		// 실행 구간이 아니면 구르기 금지
		if (ComboInputPhase != EComboInputPhase::Executable)
		{
			return;
		}
	}

	// 실제 실행 시점의 이동 방향으로 구르기
	DodgeDir = bIsMoving
		? LastMoveInputWorldDir
		: GetActorForwardVector();

	const float ForwardDot =
		FVector::DotProduct(GetActorForwardVector(), DodgeDir);

	const float RightDot =
		FVector::DotProduct(GetActorRightVector(), DodgeDir);

	const float Angle =
		FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));

	UAnimMontage* SelectedMontage = SelectRollMontage(Angle);

	if (!IsValid(SelectedMontage))
	{
		return;
	}

	// 구르기가 가능함을 확인한 뒤 현재 공격 정리
	if (bBasicAttackActionLocked)
	{
		CloseComboWindow(ComboIndex);
	}

	bUseControllerRotationYaw = false;
	StopAttackStepMove();
	bIsDodging = true;
	bIsInvincible = true;

	// 남은 공격 애니메이션 중단
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (AnimInstance->Montage_IsPlaying(AttackMontage))
		{
			AnimInstance->Montage_Stop(0.1f, AttackMontage);
		}
	}

	ComboIndex = 0;
	bCanCombo = false;
	AttackPhase = EPlayerAttackPhase::None;
	ComboInputPhase = EComboInputPhase::Closed;
	bBasicAttackActionLocked = false;
	ClearAttackBuffer();

	PlayAnimMontage(SelectedMontage);

	const float MontageDuration =
		SelectedMontage->GetPlayLength() - 0.3f;

	GetWorldTimerManager().SetTimer(
		DodgeTimerHandle,
		this,
		&ATalPlayerCharacter::EndDodge,
		MontageDuration,
		false
	);
}

void ATalPlayerCharacter::EndDodge()
{
	bIsDodging = false;
	bIsInvincible = false;

	bUseControllerRotationYaw = bIsLockOnActive; //락온중이면 회전 활성화, 아니면 비활성화
}

void ATalPlayerCharacter::UpdateRotationMode()
{
	if (bIsDodging)
	{
		return;
	}

	const bool bFaceTarget = bIsLockOnActive && !bIsSprinting && !bIsTalchiInProgress;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = bFaceTarget;
	GetCharacterMovement()->bOrientRotationToMovement = !bFaceTarget;

}


// Player Move
void ATalPlayerCharacter::Move(const FInputActionValue& Value)
{
	if (bIsTalchiInProgress)
	{
		return;
	}
	if (StatComponent->IsGroggy() || bIsLandingRecovering)
	{
		return;
	}

	const FVector2D MovementVector = Value.Get<FVector2D>();
	bIsMoving = !MovementVector.IsNearlyZero(); // Input is not zero

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// Last Move Input World Direction remember for Dodge Direction
		FVector InputDir = ForwardDirection * MovementVector.Y + RightDirection * MovementVector.X;
		if (!InputDir.IsNearlyZero())
		{
			LastMoveInputWorldDir = InputDir.GetSafeNormal();
		}

		if (bIsDodging)
		{ // AddmovementInput is ignored during dodge
			return;
		}

		if (GuardComponent->IsGuarding())
		{
			if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
			{
				FName CurrentSection = AnimInstance->Montage_GetCurrentSection(GuardMontage);

				if (CurrentSection == TEXT("Guard_hit") || CurrentSection == TEXT("Parry_LDRU")
					|| CurrentSection == TEXT("Parry_RULD"))
				{
					return; // 패링,가드 (맞는 경우 이동 무시)
				}
			}
		}

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);

		if (bBasicAttackActionLocked)
		{
			// 키를 놓은 입력은 행동으로 예약하지 않음
			if (MovementVector.IsNearlyZero())
			{
				return;
			}

			if (ComboInputPhase == EComboInputPhase::BufferOnly)
			{
				// 먼저 예약된 행동이 없을 때만 이동 예약
				if (BufferedAction == EBufferedCombatAction::None)
				{
					BufferedAction = EBufferedCombatAction::Move;
					BufferedMoveInput = MovementVector;
				}

				return;
			}

			if (ComboInputPhase == EComboInputPhase::Executable)
			{
				ExecuteBufferedMove(MovementVector);
			}

			return;
		}
	}


}

void ATalPlayerCharacter::ExecuteBufferedMove(
	const FVector2D& MovementVector)
{
	if (MovementVector.IsNearlyZero()
		|| Controller == nullptr
		|| bIsDead
		|| bIsDodging
		|| bIsUsingFlask
		|| bIsUsingSpecialArt
		|| bIsLandingRecovering
		|| GuardComponent->IsStaggered()
		|| StatComponent->IsGroggy())
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	// 기존 Move()의 가드 피격 중 이동 제한 유지
	if (GuardComponent->IsGuarding() && AnimInstance)
	{
		const FName Section =
			AnimInstance->Montage_GetCurrentSection(GuardMontage);

		if (Section == TEXT("Guard_hit")
			|| Section == TEXT("Parry_LDRU")
			|| Section == TEXT("Parry_RULD"))
		{
			return;
		}
	}

	// 공격 중 실행 가능한 구간에서만 전환
	if (!bBasicAttackActionLocked
		|| ComboInputPhase != EComboInputPhase::Executable)
	{
		return;
	}

	// 무기 판정과 공격 상태를 먼저 정리
	CloseComboWindow(ComboIndex);

	// 남은 공격 애니메이션 중단
	if (AnimInstance
		&& AnimInstance->Montage_IsPlaying(AttackMontage))
	{
		AnimInstance->Montage_Stop(0.1f, AttackMontage);
	}

	const FRotator YawRotation(
		0.0f,
		Controller->GetControlRotation().Yaw,
		0.0f
	);

	const FVector ForwardDirection =
		FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	const FVector RightDirection =
		FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, MovementVector.Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

// Player Input Component Setup
void ATalPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered,
			this, &ATalPlayerCharacter::Move);
		// Move

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed,
			this, &ATalPlayerCharacter::Move);
		// Move (Zero Vec / Input = Null)

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered,
			this, &ATalPlayerCharacter::Look);
		// Look

		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started,
			this, &ATalPlayerCharacter::StartDodge);
		// Roll Dodge

		// Sprint Input
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started,
			this, &ATalPlayerCharacter::StartSprint);
		// Sprint Input End
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed,
			this, &ATalPlayerCharacter::StopSprint);

		//Sword Attack Input
		EnhancedInputComponent->BindAction(SwordAttackAction, ETriggerEvent::Started,
			this, &ATalPlayerCharacter::Attack);

		EnhancedInputComponent->BindAction(TalAction, ETriggerEvent::Started,
			this, &ATalPlayerCharacter::Interact);//탈 상호작용

		// 패링 가드 시스템 InputComponent 바인딩 추가
		EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Started,
			this, &ATalPlayerCharacter::StartGuard);
		EnhancedInputComponent->BindAction(GuardAction, ETriggerEvent::Completed,
			this, &ATalPlayerCharacter::StopGuard);

		//락온 바인딩 추가
		EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Started,
			this, &ATalPlayerCharacter::ToggleLockOn);
		EnhancedInputComponent->BindAction(SwitchLockOnTargetAction, ETriggerEvent::Started,
			this, &ATalPlayerCharacter::SwitchLockOnTarget);

		EnhancedInputComponent->BindAction(FlaskAction, ETriggerEvent::Started,
			this, &ATalPlayerCharacter::UseFlaskItem);

		EnhancedInputComponent->BindAction(SpecialArtAction, ETriggerEvent::Started, this,
			&ATalPlayerCharacter::UseSpecialAttack);
	}
}

float ATalPlayerCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsTalchiInProgress)
	{
		return 0.0f;
	}

	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	//회피중이면 무적(데미지 무시)
	if (bIsInvincible)
	{
		return 0.0f;
	}

	if (bIsUsingFlask)
	{
		CancelFlaskUse();
	}
	if (bIsUsingSpecialArt)
	{
		CancelSpecialArt();
	}

	StatComponent->NotifyHitDuringGroggy();

	if (GuardComponent->IsGuarding() &&
		IsWithinFrontTalGuardAngle(DamageCauser))
	{
		const bool bParried = GuardComponent->TryParry();

		if (bParried)
		{
			LastParriedTarget = DamageCauser;
			OpenParryCounterWindow();
		}

		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			if (bParried)
			{
				FName ParrySection = TEXT("Parry_LDRU"); // 패링성공시 LDRU뿜뿜

				if (INTNPCCharacterBaseInterface* NPCCharacter =
					Cast<INTNPCCharacterBaseInterface>(DamageCauser))
				{
					const uint8 AttackDirection = NPCCharacter->GetAttackDirection();
					if (AttackDirection == 3) //정면방향만 RULD
					{
						ParrySection = TEXT("Parry_RULD");
					}
				}

				AnimInstance->Montage_Stop(0.1f, GuardMontage); // 가드몽타주 정지

				PlayAnimMontage(ParryMontage, 1.0f, ParrySection); //패링몽타주 재생

				//패링 성공 후 -> 0.1초 특정 행동 사용 불가
				const float ParrySectionLength = ParryMontage ? ParryMontage->GetSectionLength(
					ParryMontage->GetSectionIndex(ParrySection)) : 0.0f;
				GuardComponent->StartStagger(ParrySectionLength + ParryStaggerTime);


				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &ATalPlayerCharacter::OnParryMontage);
				AnimInstance->Montage_SetEndDelegate(EndDelegate, ParryMontage);

				return 0.0f;
			}
			else
			{
				// 블렌드 스페이스
				// 
				//AnimInstance->Montage_JumpToSection(TEXT("Guard_hit"), GuardMontage);
				PlayAnimMontage(GuardMontage, 1.0f, TEXT("Guard_hit"));
			}

		}

		if (bParried)
		{
			return 0.0f;
		}

		// 패링 X 가드 O 성공 시 넋 30(임시) 줄어듦
		StatComponent->DrainSoul(Damage * GuardComponent->GetGuardSoulCost());

		UE_LOG(LogTemp, Warning, TEXT("가드성공"));
		if (CombatFeelComponent)
		{
			CombatFeelComponent->TriggerHitStop(ENTHitStopKind::Guard);
		}

		// 가드 성공 후 -> 0.2초 특정 행동 사용 불가
		const float GuardHitSectionLength = GuardMontage
			? GuardMontage->GetSectionLength(GuardMontage->GetSectionIndex(TEXT("Guard_hit")))
			: 0.0f;
		GuardComponent->StartStagger(GuardHitSectionLength + GuardStaggerTime);
		return 0.0f; // HP 피해 없음
	}

	StatComponent->ApplyDamage(Damage);
	StatComponent->DrainSoul(10.0f);
	// 실제로 받은 데미지 적용되었는지 호출자(공격자)에게 알려주는 용도
	return Damage;
}

float ATalPlayerCharacter::ApplyDamage(AActor* DamageCauser, const float InDamage,
	const float InAttackDrain, const float InGuardDrain, const bool bCanParry)
{
	if (bIsTalchiInProgress)
	{
		return 0.0f;
	}
	//회피중이면 무적(데미지 무시)
	if (bIsInvincible)
	{
		return 0.0f;
	}

	if (bIsUsingFlask)
	{
		CancelFlaskUse();
	}

	if (bIsUsingSpecialArt)
	{
		CancelSpecialArt();
	}

	//그로기 중 아니면 아무일도 안한다!
	StatComponent->NotifyHitDuringGroggy();

	if (GuardComponent->IsGuarding() && IsWithinFrontTalGuardAngle(DamageCauser))
	{
		const bool bParried = GuardComponent->TryParry();

		if (bParried)
		{
			LastParriedTarget = DamageCauser;
			OpenParryCounterWindow();
		}

		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			if (bParried)
			{
				FName ParrySection = TEXT("Parry_LDRU"); // 패링성공시 LDRU뿜뿜

				if (INTNPCCharacterBaseInterface* NPCCharacter =
					Cast<INTNPCCharacterBaseInterface>(DamageCauser))
				{
					const uint8 AttackDirection = NPCCharacter->GetAttackDirection();
					if (AttackDirection == 3) //정면방향만 RULD
					{
						ParrySection = TEXT("Parry_RULD");
					}

					NPCCharacter->ApplyDamage(this, 0, 50, true);
				}

				AnimInstance->Montage_Stop(0.1f, GuardMontage); // 가드몽타주 정지

				PlayAnimMontage(ParryMontage, 1.0f, ParrySection); //패링몽타주 재생

				//패링 성공 분기
				// 넉백 거리
				ApplyGuardKnockBack(InDamage);
				//패링 성공 후 -> 0.1초 특정 행동 사용 불가
				const float ParrySectionLength = ParryMontage ? ParryMontage->GetSectionLength(
					ParryMontage->GetSectionIndex(ParrySection)) : 0.0f;
				GuardComponent->StartStagger(ParrySectionLength + ParryStaggerTime);

				FOnMontageEnded EndDelegate;
				EndDelegate.BindUObject(this, &ATalPlayerCharacter::OnParryMontage);
				AnimInstance->Montage_SetEndDelegate(EndDelegate, ParryMontage);

				return 0.0f;
			}
			else
			{
				// 블렌드 스페이스
				// 
				//AnimInstance->Montage_JumpToSection(TEXT("Guard_hit"), GuardMontage);
				PlayAnimMontage(GuardMontage, 1.0f, TEXT("Guard_hit"));
			}

		}

		if (bParried)
		{
			return 0.0f;
		}


		// 패링 X 가드 O 성공 시 넋 30(임시) 줄어듦
		StatComponent->DrainSoul(InGuardDrain * GuardComponent->GetGuardSoulCost());

		// 가드 중 넋 고갈시 가드 브레이크
		if (StatComponent->IsSoulExhausted())
		{
			GuardComponent->StopGuard();
			StatComponent->SetSoulRegenBlocked(false);

			PlayAnimMontage(GuardMontage, 1.0f, TEXT("Guard_Hit_Break"));

			const float BreakLength = GuardMontage
				? GuardMontage->GetSectionLength(GuardMontage->GetSectionIndex(TEXT("Guard_Hit_Break")))
				: 0.0f;

			GuardComponent->StartStagger(BreakLength + GuardStaggerTime);

			ApplyGuardKnockBack(InDamage);
			return 0.0f; // -> 가드 깨질 시 받을 데미지 입력해야함
		}

		// 넉백
		ApplyGuardKnockBack(InDamage);

		UE_LOG(LogTemp, Warning, TEXT("가드성공"));
		if (CombatFeelComponent)
		{
			CombatFeelComponent->TriggerHitStop(ENTHitStopKind::Guard);
		}
		// 가드 성공 후 -> 0.2초 특정 행동 사용 불가
		const float GuardHitSectionLength = GuardMontage
			? GuardMontage->GetSectionLength(GuardMontage->GetSectionIndex(TEXT("Guard_hit")))
			: 0.0f;
		GuardComponent->StartStagger(GuardHitSectionLength + GuardStaggerTime);
		return 0.0f; // HP 피해 없음
	}

	StatComponent->ApplyDamage(InDamage);
	StatComponent->DrainSoul(10.0f);
	// 실제로 받은 데미지 적용되었는지 호출자(공격자)에게 알려주는 용도
	return InDamage;
}

float ATalPlayerCharacter::ApplyDamage(AActor* DamageCauser, const FPlayerApplyDamageData InDamageData)
{
	float ResultDamage = ApplyDamage(DamageCauser, InDamageData.HPDamage, InDamageData.SoulDamage, InDamageData.HPDamage, InDamageData.bCriticalEnable);

	return ResultDamage;
}

void ATalPlayerCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>
			(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void ATalPlayerCharacter::Look(const FInputActionValue& Value)
{
	if (bTalchiFirstPersonActive) return;
	if (CombatFeelComponent && CombatFeelComponent->IsParryCameraMoving())
	{
		return;
	}

	//락온여부
	if ((bIsLockOnActive && !bIsTalchiInProgress) || Controller == nullptr) return;
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller == nullptr) return;

	const float HorizontalScale = FMath::Clamp(HorizontalCameraSensitivity, 0.0f, 100.0f) / 50.0f;
	const float VerticalScale = FMath::Clamp(VerticalCameraSensitivity, 0.0f, 100.0f) / 50.0f;

	AddControllerYawInput(
		LookAxisVector.X * HorizontalScale
	);
	AddControllerPitchInput(
		-LookAxisVector.Y * VerticalScale
	);
}

void ATalPlayerCharacter::StartSprint(const FInputActionValue& Value)
{
	if (bIsTalchiInProgress)
	{
		return;
	}
	// 패링,가드 성공 시(시간 초만큼 실행불가능)
	if (GuardComponent->IsStaggered() || StatComponent->IsGroggy()
		|| bIsLandingRecovering || bIsUsingFlask || bIsUsingSpecialArt) return;

	bIsSprinting = true;

}

void ATalPlayerCharacter::StopSprint(const FInputActionValue& Value)
{
	bIsSprinting = false;
}

void ATalPlayerCharacter::Attack(const FInputActionValue& Value)
{
	if (bIsTalchiInProgress)
	{
		return;
	}
	if (bParryCounterReady && !bIsUsingSpecialArt)
	{
		TriggerParryCounter();
		return;
	}

	// 패링,가드 성공 시(시간 초만큼 실행불가능)
	if (GuardComponent->IsStaggered()
		|| StatComponent->IsGroggy()
		|| bIsLandingRecovering
		|| bIsUsingFlask
		|| bIsDodging
		|| bIsUsingSpecialArt
		|| !IsValid(AttackMontage)
		) return;

	if (AttackPhase == EPlayerAttackPhase::None)
	{
		StartAttackStep(1);
		return;
	}
	if (AttackPhase == EPlayerAttackPhase::Grace)
	{
		StartNextCombo();
		return;
	}
	if (ComboIndex >= MaxBasicAttackCombo) return;

	switch (ComboInputPhase)
	{
	case EComboInputPhase::BufferOnly:
		if (BufferedAction == EBufferedCombatAction::None)
		{
			BufferedAction = EBufferedCombatAction::Attack;
		}
		break;

	case EComboInputPhase::Executable:
		StartNextCombo();
		break;

	case EComboInputPhase::Closed:
	default:
		break;
	}

}

void ATalPlayerCharacter::StartAttackStep(int32 Step)
{
	if (!IsValid(AttackMontage) || !IsValid(GetMesh())) return;

	//이전 공격의 유예가 새 공격을 초기화하지 못 하게 막음
	GetWorldTimerManager().ClearTimer(ComboGraceTimer);//🐘

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!IsValid(AnimInstance)) return;

	if (Step<1 || Step>MaxBasicAttackCombo) return;//스텝이나 콤보수 확인

	ComboIndex = Step;
	AttackPhase = EPlayerAttackPhase::Startup;
	bCanCombo = false;

	//새 공격이 시작시
	ComboInputPhase = EComboInputPhase::Closed;
	bBasicAttackActionLocked = true;

	//기존 일반 이동과 날아있는 이동 입력 정리
	ConsumeMovementInputVector();
	GetCharacterMovement()->StopMovementImmediately();
	StopAttackStepMove();


	ClearAttackBuffer();

	const FName SectionName(*FString::Printf(TEXT("Attack%d"), ComboIndex));

	if (AnimInstance->Montage_IsPlaying(AttackMontage))
	{
		AnimInstance->Montage_JumpToSection(SectionName, AttackMontage);
	}
	else
	{
		const float PlayedDuration = PlayAnimMontage(AttackMontage, 1.0f, SectionName);
		if (PlayedDuration <= 0.0f)//몽타주 재생 실패시 초기화
		{
			AttackPhase = EPlayerAttackPhase::None;
			ComboIndex = 0;
			ClearAttackBuffer();

			ComboInputPhase = EComboInputPhase::Closed;
			bBasicAttackActionLocked = false;
		}
	}
}
void ATalPlayerCharacter::StartNextCombo()
{
	if (ComboIndex <= 0)
	{
		StartAttackStep(1);
		return;
	}
	if (ComboIndex >= MaxBasicAttackCombo) return;//4타는 막기

	StartAttackStep(ComboIndex + 1);
}

void ATalPlayerCharacter::EnableWeaponCollision()
{
	if (bIsUsingSpecialArt) return;
	//공격 판정 구간 진입
	if (AttackPhase == EPlayerAttackPhase::Startup)
	{
		AttackPhase = EPlayerAttackPhase::Active;
	}

	if (WeaponCollision == nullptr)
	{
		return;
	}
	HitActors.Empty(); // 새 공격이라 이전 기록 비우기
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}
void ATalPlayerCharacter::DisableWeaponCollision()
{
	if (bIsUsingSpecialArt) return;
	if (WeaponCollision != nullptr)
	{
		WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	//후경직 진입
	if (AttackPhase == EPlayerAttackPhase::Active)
	{
		AttackPhase = EPlayerAttackPhase::Recovery;
	}

}

void ATalPlayerCharacter::OnWeaponOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor ||
		OtherActor == this ||
		HitActors.Contains(OtherActor))
	{
		return;
	}

	if (bIsUsingSpecialArt && !bIsSpecialArtHitActive)
	{
		return;
	}

	INTNPCCharacterBaseInterface* NPCCharacter =
		Cast<INTNPCCharacterBaseInterface>(OtherActor);

	if (!NPCCharacter)
	{
		return;
	}

	if (OtherComp &&
		OtherComp->ComponentHasTag(TEXT("WeaponCollision")))
	{
		return;
	}

	HitActors.Add(OtherActor);
	StatComponent->NotifyCombatAction();

	float DamageResult = 0.0f;

	if (bIsUsingSpecialArt)
	{
		FNPCApplyDamageData DamageData{};
		DamageData.HPDamage = CurrentSpecialArtData.HPDamage;
		DamageData.SoulDamage = CurrentSpecialArtData.SoulDamage;
		DamageData.StaggerDamage = CurrentSpecialArtData.StaggerDamage;
		DamageData.bIsParry = false;

		// 스킬로 죽이면 NPC가 이 값으로 날아가게
		DamageData.LaunchImpulse = ComputeSpecialArtLaunchImpulse(OtherActor);

		// 안 죽었으면 이값으로밀려나기
		DamageData.PushBackVelocity = ComputePushBackVelocity(OtherActor,
			CurrentSpecialArtData.PushBackSpeed);

		DamageResult = NPCCharacter->ApplyDamage(this, DamageData);

	}
	else
	{
		// 기존 일반 공격의 피해량 유지
		DamageResult = NPCCharacter->ApplyDamage(
			this,
			AttackDamage,
			10.0f,
			false);
	}

	if (DamageResult < 0.0f)
	{
		StatComponent->RecoverSoul(KillSoulReward);
	}

	// 패링 슬로우 모션중 시간조작 겹치니까 건너고
	// 2단계 슬로우모션이 컴포넌트로 이사시 가드는 컴포넌안으로 들어갈것임
	if (CombatFeelComponent)
	{
		const ENTHitStopKind Kind = (DamageResult < 0.0f) ? ENTHitStopKind::Kill :
			(bIsUsingSpecialArt ? ENTHitStopKind::SpecialArt : ENTHitStopKind::Normal);

		CombatFeelComponent->TriggerHitStop(Kind);
	}

}
void ATalPlayerCharacter::EquipTal(ATalBase* NewTal)
{
	if (TalComponent)
	{
		TalComponent->EquipTal(NewTal);
	}
	SpecialArtUseCount = 0;
}

ATalBase* ATalPlayerCharacter::GetCurrentTal() const
{
	return TalComponent ? TalComponent->GetCurrentTal() : nullptr;
}

void ATalPlayerCharacter::BeginPlay()//캐릭터 생성시
{
	Super::BeginPlay();

	InitialSpawnTransform = GetActorTransform();

	FlaskCount = FlaskInitialCount;

	//기본 탈 장착
	if (TalComponent && DefaultTalClass)
	{
		TalComponent->SpawnDefaultTal(DefaultTalClass);
	}

	if (StatComponent)
	{
		StatComponent->OnDeath.AddUniqueDynamic(this, &ATalPlayerCharacter::HandleDeath);//델리게이트 등록
	}


}

ATalBase* ATalPlayerCharacter::UnequipTal()
{
	return TalComponent ? TalComponent->UnequipTal() : nullptr;
}

void ATalPlayerCharacter::TakeTalFrom(AActor* TargetActor)
{
	// The parry sequence owns the same spring arm until its return finishes.
	if (CombatFeelComponent && CombatFeelComponent->IsParryCameraMoving())
	{
		return;
	}

	if (bIsTalchiInProgress
		|| bIsDead
		|| bIsDodging
		|| bIsUsingFlask
		|| bIsUsingSpecialArt
		|| bIsLandingRecovering
		|| bTalchiPreviewActive)
	{
		return;
	}

	if (!IsValid(GuardComponent)
		|| !IsValid(StatComponent)
		|| GuardComponent->IsStaggered()
		|| StatComponent->IsGroggy())
	{
		return;
	}

	UAnimInstance* AnimInstance =
		GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;

	if (!AnimInstance)
	{
		return;
	}
    // 실행 직전에도 동일한 조건으로 다시 확인합니다.
    if (!CanTalchiTarget(TargetActor))
    {
        MC_LOG(LogSY, Warning,
            TEXT("탈취 실패: 대상이 탈취 조건을 만족하지 않습니다"));
        return;
    }

    UNTTalChiTargetComponent* TargetPoint =
        TargetActor->FindComponentByClass<UNTTalChiTargetComponent>();

    SelectedTalChiTarget = TargetPoint;

    // 아래부터는 기존 코드를 유지합니다.

	const FTransform TargetTransform = TargetPoint->GetTalchiTransform();

	//탈취 몽타주 시작
	static const FName TalchiWarpTargetName(TEXT("TalchiTarget"));

	if (!IsValid(MotionWarpingComponent))
	{
		MC_LOG(LogSY, Warning, TEXT("모션워핑컴포넌트가 없음"));
		return;
	}

	UAnimMontage* PlayerTalchiMontage = TargetPoint->GetPlayerTalchiMontage();

	if (!IsValid(PlayerTalchiMontage))
	{
		MC_LOG(LogSY, Warning, TEXT("탈취 몽타주가 없음"));
		return;
	}

	ANTNPCCharacterBase* VictimNPC =
		Cast<ANTNPCCharacterBase>(TargetActor);

	if (!IsValid(VictimNPC) || !VictimNPC->BeginTalchiHold())
	{
		SelectedTalChiTarget.Reset();

		UE_LOG(LogTemp, Warning,
			TEXT("Talchi: failed to hold victim."));
		return;
	}

	ActiveTalchiVictim = VictimNPC;
	ActiveVictimTalchiMontage = nullptr;

	MotionWarpingComponent->AddOrUpdateWarpTargetFromTransform(TalchiWarpTargetName, TargetTransform);

	if (bBasicAttackActionLocked)
	{
		CloseComboWindow(ComboIndex);
	}

	// 몽타주 재생 전에 상태를 켭니다.
	bIsTalchiInProgress = true;
	ActiveTalchiMontage = PlayerTalchiMontage;
	BeginTalchiCamera(TargetActor);
	TalchiSlowElapsed = 0.0f;
	UpdateTalchiSlow();

	// 기존 이동 입력과 속도를 정리합니다.
	ConsumeMovementInputVector();

	if (UCharacterMovementComponent* Movement =
		GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
	}

	bIsMoving = false;

	// 진행 중이던 가드를 해제합니다.
	StopGuard(FInputActionValue(false));

	HideTalchiPrompt();

	const float MontageLength = PlayAnimMontage(PlayerTalchiMontage);

	if (MontageLength <= 0.0f)
	{
		FinishTalchi();

		MC_LOG(LogSY, Warning, TEXT("탈취 몽타주 재생 실패"));
		return;
	}

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(
		this,
		&ATalPlayerCharacter::OnTalchiMontageEnded);

	AnimInstance->Montage_SetEndDelegate(
		EndDelegate,
		PlayerTalchiMontage);

	MC_LOG(LogSY, Warning, TEXT("탈취 모션 시작"));

}
void ATalPlayerCharacter::Interact()
{
	if (bIsTalchiInProgress)
	{
		return;
	}
	if (bIsDead)
	{
		return;
	}

	UAnimInstance* AnimInstance =
		GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;

	// 탈취 몽타주 도중 다시 F를 눌러도 시작하지 않습니다.
	if (IsValid(ActiveTalchiMontage)
		&& AnimInstance
		&& AnimInstance->Montage_IsActive(ActiveTalchiMontage))
	{
		return;
	}

	if (!IsValid(TalchiPromptWidget)
		|| TalchiPromptWidget->GetVisibility()
		!= ESlateVisibility::HitTestInvisible)
	{
		return;
	}

	AActor* TargetActor = FocusedTalchiTarget.Get();

	// 표시 이후 그로기가 풀리거나 범위를 벗어났을 수 있습니다.
	if (!CanTalchiTarget(TargetActor))
	{
		HideTalchiPrompt();
		return;
	}

	TakeTalFrom(TargetActor);
}
//void ATalPlayerCharacter::EquipWeapon(ATalWeapon* NewWeapon)
//{
//	if (NewWeapon == nullptr) return;
//
//	EquippedWeapon = NewWeapon;
//	EquippedWeapon->SetOwner(this);
//	EquippedWeapon->AttachToComponent(
//		GetMesh(),
//		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
//		TEXT("WeaponSocket_R")
//	);
//}

//ATalWeapon* ATalPlayerCharacter::GetEquippedWeapon() const
//{
//	return EquippedWeapon;
//}

void ATalPlayerCharacter::OpenComboWindow()
{
	//기본 공격 중이 아니면 무시
	if (!bBasicAttackActionLocked || ComboIndex <= 0) return;
	//입력제거
	ClearAttackBuffer();

	ComboInputPhase = EComboInputPhase::BufferOnly;
}

void ATalPlayerCharacter::BeginComboExecution(int32 ExpectedStep)
{
	if (!bBasicAttackActionLocked || ComboIndex != ExpectedStep)
	{
		return;
	}

	// 4타에서도 구르기는 실행할 수 있어야 함
	if (ComboIndex <= 0 || ComboIndex > MaxBasicAttackCombo)
	{
		return;
	}

	if (ComboInputPhase != EComboInputPhase::BufferOnly)
	{
		return;
	}

	ComboInputPhase = EComboInputPhase::Executable;

	// 예약 종류를 보관한 뒤 버퍼 비우기
	const EBufferedCombatAction ActionToExecute = BufferedAction;
	const FVector2D MoveToExecute = BufferedMoveInput;

	ClearAttackBuffer();

	if (bIsDead
		|| GuardComponent->IsStaggered()
		|| StatComponent->IsGroggy()
		|| bIsLandingRecovering
		|| bIsUsingFlask
		|| bIsDodging
		|| bIsUsingSpecialArt)
	{
		return;
	}

	switch (ActionToExecute)
	{
	case EBufferedCombatAction::Attack:
		if (ComboIndex < MaxBasicAttackCombo)
		{
			StartNextCombo();
		}
		break;

	case EBufferedCombatAction::Dodge:
		StartDodge(FInputActionValue(true));
		break;

	case EBufferedCombatAction::Move:
		ExecuteBufferedMove(MoveToExecute);
		break;
	default:
		// 예약이 없으면 이후 입력을 기다림
		break;
	}
}

void ATalPlayerCharacter::CloseComboWindow(int32 ExpectedStep)
{
	if (!bBasicAttackActionLocked || ComboIndex != ExpectedStep) return;

	GetWorldTimerManager().ClearTimer(ComboGraceTimer);

	ClearAttackBuffer();
	DisableWeaponCollision();
	StopAttackStepMove();

	bCanCombo = false;
	ComboInputPhase = EComboInputPhase::Closed;
	AttackPhase = EPlayerAttackPhase::None;
	ComboIndex = 0;

	bBasicAttackActionLocked = false;;
}
void ATalPlayerCharacter::ClearAttackBuffer()//버퍼 초기화
{
	BufferedAction = EBufferedCombatAction::None;
	BufferedMoveInput = FVector2D::ZeroVector;
}

void ATalPlayerCharacter::StartComboGrace()
{
	bCanCombo = false;
	ClearAttackBuffer();

	AttackPhase = EPlayerAttackPhase::Grace;

	GetWorldTimerManager().ClearTimer(ComboGraceTimer);
	GetWorldTimerManager().SetTimer(
		ComboGraceTimer,
		this,
		&ATalPlayerCharacter::EndComboGrace,
		ComboGraceDuration,
		false
	);
}
void ATalPlayerCharacter::EndComboGrace()
{
	if (AttackPhase != EPlayerAttackPhase::Grace) return;

	AttackPhase = EPlayerAttackPhase::None;
	ComboIndex = 0;
	bCanCombo = false;

	ClearAttackBuffer();
}

//🐘
void ATalPlayerCharacter::UseSpecialAttack(
	const FInputActionValue& Value)
{
	TryUseSpecialAttack(false);
}

bool ATalPlayerCharacter::TryUseSpecialAttack(bool bFreeUse)
{
	if (bIsTalchiInProgress)
	{
		return false;
	}
	if (!GuardComponent || !StatComponent || bIsDead)
	{
		return false;
	}

	// 기존 규칙 유지:
	// 패링 카운터는 스태거 검사를 건너뜁니다.
	if ((!bFreeUse && GuardComponent->IsStaggered()) ||
		StatComponent->IsGroggy() ||
		bIsLandingRecovering ||
		bIsUsingFlask ||
		bIsDodging ||
		bIsUsingSpecialArt)
	{
		return false;
	}

	ATalBase* MyTal = GetCurrentTal();
	if (!MyTal)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("SpecialArt: No equipped Tal"));
		return false;
	}

	const FName SpecialArtID = MyTal->GetSpecialArtID();

	if (SpecialArtID.IsNone())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("SpecialArt: Tal has no SpecialArtID"));
		return false;
	}

	const FNTPlayerSpecialArtData* ArtData =
		UNTGameDataSingleton::Get().GetPlayerSpecialArt(SpecialArtID);

	if (!ArtData)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("SpecialArt: Missing skill row: %s"),
			*SpecialArtID.ToString());
		return false;
	}

	const float SoulCost =
		FMath::Max(0.0f, static_cast<float>(ArtData->SoulCost));

	// 일반 사용만 횟수와 넋을 검사합니다.
	if (!bFreeUse)
	{
		if (SpecialArtUseCount >= ArtData->UseLimit)
		{
			UE_LOG(LogTemp, Warning,
				TEXT("SpecialArt: Use limit reached"));
			return false;
		}

		if (!StatComponent->HasEnoughSoul(SoulCost))
		{
			UE_LOG(LogTemp, Warning,
				TEXT("SpecialArt: Not enough soul"));
			return false;
		}
	}

	// 최소한 첫 타격 데이터가 있는지 확인합니다.
	if (!UNTGameDataSingleton::Get().FindPlayerSpecialArtStep(
		SpecialArtID, 0))
	{
		UE_LOG(LogTemp, Warning,
			TEXT("SpecialArt: Missing hit 0 for %s"),
			*SpecialArtID.ToString());
		return false;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	UAnimMontage* Montage = ArtData->Montage.LoadSynchronous();

	if (!AnimInstance || !Montage || !WeaponCollision)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("SpecialArt: Missing AnimInstance, Montage or Collision"));
		return false;
	}

	// 재생 중 호출되는 노티파이도 상태를 확인할 수 있도록
	// Montage_Play 전에 기술 상태를 설정합니다.
	CurrentSpecialArtID = SpecialArtID;
	CurrentSpecialArtMontage = Montage;
	CurrentSpecialArtData = FPlayerCurrentSpecialArtData{};
	CurrentSpecialArtComboCount = INDEX_NONE;
	bIsSpecialArtHitActive = false;
	bIsUsingSpecialArt = true;
	bIsSpecialArtCancelling = false;

	const float PlayedLength =
		AnimInstance->Montage_Play(Montage, 1.0f);

	if (PlayedLength <= 0.0f)
	{
		// 실패 시 일반 공격의 충돌 상태까지 정리하지 않습니다.
		bIsUsingSpecialArt = false;
		bIsSpecialArtHitActive = false;
		CurrentSpecialArtID = NAME_None;
		CurrentSpecialArtMontage = nullptr;
		CurrentSpecialArtComboCount = INDEX_NONE;
		CurrentSpecialArtData = FPlayerCurrentSpecialArtData{};

		UE_LOG(LogTemp, Warning,
			TEXT("SpecialArt: Montage_Play failed: %s"),
			*GetNameSafe(Montage));
		return false;
	}

	// 기존 일반 공격의 예약과 상태가 특수기에 남지 않게 정리합니다.
	GetWorldTimerManager().ClearTimer(ComboGraceTimer);
	ClearAttackBuffer();

	bCanCombo = false;
	bBasicAttackActionLocked = false;
	ComboInputPhase = EComboInputPhase::Closed;
	AttackPhase = EPlayerAttackPhase::None;
	ComboIndex = 0;

	WeaponCollision->SetCollisionEnabled(
		ECollisionEnabled::NoCollision);
	HitActors.Empty();

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(
		this,
		&ATalPlayerCharacter::OnSpecialArtMontageEnd);

	AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);

	// 실제로 재생에 성공한 경우에만 비용을 반영합니다.
	if (!bFreeUse)
	{
		++SpecialArtUseCount;
		StatComponent->DrainSoul(SoulCost);
	}

	return true;
}

FVector ATalPlayerCharacter::ComputeSpecialArtLaunchImpulse(const AActor* Target) const
{
	if (!Target)
	{
		return FVector::ZeroVector;
	}

	const FVector Forward = GetActorForwardVector().GetSafeNormal2D();

	FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();

	// 완전히 겹치면 방향 못구하니 정면으로한다
	if (ToTarget.IsNearlyZero())
	{
		ToTarget = Forward;
	}

	// 플레이어 정면 * 대상 방향
	// 0 = 정면, 1 = 멀어지는 방향 (중간값 = 대각선)
	FVector Dir = FMath::Lerp(
		Forward, ToTarget, FMath::Clamp(SpecialArtLaunchRadialBlend, 0.0f, 1.0f)).GetSafeNormal2D();

	// 정면과 대상 방향 반대면 합 0 그때는 정면으로 
	if (Dir.IsNearlyZero())
	{
		Dir = Forward;
	}

	// 위쪽 성분 더해서 뜨게 만들기
	Dir.Z = SpecialArtLaunchUpRatio;

	return Dir.GetSafeNormal() * SpecialArtLaunchPower;
}

FVector ATalPlayerCharacter::ComputePushBackVelocity(const AActor* Target, float Speed) const
{
	if (!Target || Speed <= 0.0f)
	{
		return FVector::ZeroVector;
	}

	//플레이어->NPC 방향으로 때리느 방향
	FVector Dir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();

	// 완전히 겹쳐서 방향을 못 구하면 정면으로 밀어낸다
	if (Dir.IsNearlyZero())
	{
		Dir = GetActorForwardVector().GetSafeNormal2D();
	}

	return Dir * Speed; // Z는 이미 0이니까 괜찮다에요
}

void ATalPlayerCharacter::BeginSpecialArtHit(int32 ComboCount)
{
	if (!bIsUsingSpecialArt ||
		!CurrentSpecialArtMontage ||
		!WeaponCollision ||
		bIsSpecialArtCancelling)
	{
		return;
	}

	// 이전 타격을 닫고 새 데이터를 준비합니다.
	WeaponCollision->SetCollisionEnabled(
		ECollisionEnabled::NoCollision);

	bIsSpecialArtHitActive = false;
	CurrentSpecialArtComboCount = INDEX_NONE;
	CurrentSpecialArtData = FPlayerCurrentSpecialArtData{};

	const FNTPlayerSpecialArtStepData* StepData =
		UNTGameDataSingleton::Get().FindPlayerSpecialArtStep(
			CurrentSpecialArtID,
			ComboCount);

	if (!StepData)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("SpecialArt: Missing hit data: %s / %d"),
			*CurrentSpecialArtID.ToString(),
			ComboCount);
		return;
	}

	CurrentSpecialArtData.HPDamage = StepData->HPDamage;
	CurrentSpecialArtData.SoulDamage = StepData->SoulDamage;
	CurrentSpecialArtData.StaggerDamage = StepData->StaggerDamage;
	CurrentSpecialArtData.PushBackSpeed = StepData->PushBackSpeed;

	CurrentSpecialArtComboCount = ComboCount;
	bIsSpecialArtHitActive = true;

	// 같은 적도 다음 타격에서는 다시 맞을 수 있습니다.
	HitActors.Empty();

	// 충돌을 켜기 전에 데이터와 충돌 응답을 준비합니다.
	WeaponCollision->SetCollisionResponseToAllChannels(
		ECollisionResponse::ECR_Overlap);

	WeaponCollision->SetCollisionEnabled(
		ECollisionEnabled::QueryOnly);
}

void ATalPlayerCharacter::EndSpecialArtHit(int32 ComboCount)
{
	if (!bIsUsingSpecialArt ||
		CurrentSpecialArtComboCount != ComboCount)
	{
		return;
	}

	bIsSpecialArtHitActive = false;
	CurrentSpecialArtComboCount = INDEX_NONE;

	if (WeaponCollision)
	{
		WeaponCollision->SetCollisionEnabled(
			ECollisionEnabled::NoCollision);
	}
}

void ATalPlayerCharacter::StartSpecialArtStep(FName StepID)
{
}

void ATalPlayerCharacter::OnSpecialArtMontageEnd(
	UAnimMontage* Montage,
	bool bInterrupted)
{
	if (Montage != CurrentSpecialArtMontage.Get())
	{
		return;
	}

	ResetSpecialArtState();
}

void ATalPlayerCharacter::CancelSpecialArt()
{
	StopAttackStepMove();
	if (!bIsUsingSpecialArt)
	{
		return;
	}
	bIsSpecialArtCancelling = true;

	UAnimMontage* MontageToStop = CurrentSpecialArtMontage.Get();

	// 블렌드아웃 중에도 이전 특수기 상태를 유지합니다.
	bIsSpecialArtHitActive = false;
	CurrentSpecialArtComboCount = INDEX_NONE;

	if (WeaponCollision)
	{
		WeaponCollision->SetCollisionEnabled(
			ECollisionEnabled::NoCollision);
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (MontageToStop &&
			AnimInstance->Montage_IsActive(MontageToStop))
		{
			// 종료 콜백에서 최종 상태를 정리합니다.
			AnimInstance->Montage_Stop(0.2f, MontageToStop);
			return;
		}
	}

	ResetSpecialArtState();
}

void ATalPlayerCharacter::ResetSpecialArtState()
{
	StopAttackStepMove();
	bIsSpecialArtCancelling = false;
	bIsSpecialArtHitActive = false;

	if (WeaponCollision)
	{
		WeaponCollision->SetCollisionEnabled(
			ECollisionEnabled::NoCollision);
	}

	HitActors.Empty();

	bIsUsingSpecialArt = false;
	CurrentSpecialArtID = NAME_None;
	CurrentSpecialArtMontage = nullptr;
	CurrentSpecialArtComboCount = INDEX_NONE;
	CurrentSpecialArtData = FPlayerCurrentSpecialArtData{};
}

void ATalPlayerCharacter::RecoverSoul(float Amount)
{
	StatComponent->RecoverSoul(Amount);
}



// 패링 / 가드 시스템 연동
void ATalPlayerCharacter::StartGuard(const FInputActionValue& Value)
{
	if (bIsTalchiInProgress)
	{
		return;
	}
	//고갈중 가드 불가
	if (StatComponent->IsSoulExhausted() || bIsUsingFlask || bIsUsingSpecialArt)
	{
		return;
	}
	GuardComponent->StartGuard();
	StatComponent->SetSoulRegenBlocked(true);
	//PlayAnimMontage(GuardMontage, 1.0f, TEXT("Guard_Start"));
	// 가드 블렌드 스페이스 사용위해 잠시 주석처리 잘되면 지울거임
}

void ATalPlayerCharacter::StopGuard(const FInputActionValue& Value)
{
	GuardComponent->StopGuard();
	StatComponent->SetSoulRegenBlocked(false);
	//가드 블렌드 스페이스 사용 위해 주석처리
	//if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	//{
	//	//GuardMontage에서 Guard_End를 찾아 점프하십시오
	//	AnimInstance->Montage_JumpToSection(TEXT("Guard_End"), GuardMontage);
	//}
}

bool ATalPlayerCharacter::IsWithinFrontTalGuardAngle(AActor* DamageCauser) const
{
	if (!IsValid(DamageCauser))
	{
		return false;
	}

	const FVector ToAttacker = (DamageCauser->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	const FVector Forward = GetActorForwardVector().GetSafeNormal2D();

	const float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Forward, ToAttacker)));

	return AngleDeg <= GuardFrontHalfAngle;
}

void ATalPlayerCharacter::TriggerParrySlowMo()
{
    if (CombatFeelComponent)
    {
        // 카메라가 바라볼 적을 먼저 전달
        CombatFeelComponent->SetParryCameraTarget(
            LastParriedTarget.Get());

        // 그다음 슬로우모션과 카메라 연출 시작
        CombatFeelComponent->StartParrySlowMo();
    }
}

void ATalPlayerCharacter::OnParryMontage(UAnimMontage* Montage, bool bInterrupted)
{
	// 블렌드 스페이스 위해 주석삭제
	// 아직 가드 버튼 누르고 있음
	/*if (GuardComponent->IsGuarding())
	{
		PlayAnimMontage(GuardMontage, 1.0f, TEXT("Guard_loop"));
	}*/
}

void ATalPlayerCharacter::ApplyGuardKnockBack(float InAttackDamage)
{
	const float Scale = InAttackDamage / KnockBackRefDamage;
	const FVector KnockbackOffSet = -GetActorForwardVector() * KnockBackDistance * Scale;

	// true = 벽 등 sweep(도형 기반 감지) 처리
	SetActorLocation(GetActorLocation() + KnockbackOffSet, true);
}

bool ATalPlayerCharacter::IsGuarding() const
{
	return GuardComponent->IsGuarding();
}

bool ATalPlayerCharacter::IsGuardInputHeld() const
{
	return GuardComponent->IsGuardInputHeld();
}

bool ATalPlayerCharacter::IsInParryCounterWindow() const
{
	return bParryCounterReady;
}

void ATalPlayerCharacter::OpenParryCounterWindow()
{
	bParryCounterReady = true;

	ParryCounterEndRealTime = GetWorld()->GetRealTimeSeconds()
		+ ParryCounterWindowDuration;

	if (CombatFeelComponent)
	{
		CombatFeelComponent->SetParryCameraCounterEndTime(
			ParryCounterEndRealTime);
	}
}

void ATalPlayerCharacter::CloseParryCounterWindow()
{
	bParryCounterReady = false;
}

void ATalPlayerCharacter::TriggerParryCounter()
{
	// 무료 발동을 먼저 시도한다. 실패하면 아무 상태도 바꾸지 않는다
	if (!TryUseSpecialAttack(true))
	{
		// 창을 유지하므로 다음 클릭이 다시 시도할 수 있다
		return;
	}

	// 여기부터는 카운터가 확실히 발동한 경우만 실행된다

	// 슬로우 즉시 해제
	if (CombatFeelComponent)
	{
		CombatFeelComponent->StopParrySlowMo();
		CombatFeelComponent->StartParryCameraSkillReturn();
	}

	// 패링 몽타주 스태거 해제 (카운터 이후 행동 불가 방지)
	GuardComponent->StartStagger(0.0f);

	CloseParryCounterWindow();
}

//락온 시스템
void ATalPlayerCharacter::BeginLockOn()
{
	if (!IsValid(CurrentLockOnTarget)) return;

	bIsLockOnActive = true;

	//락온 중에는 상대를 바라봄
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	CurrentLockOnTarget->SetLockOnWidgetVisible(true);
}

void ATalPlayerCharacter::ToggleLockOn(const FInputActionValue& Value)
{
	if (bIsTalchiInProgress) return;
	if (IsValid(CurrentLockOnTarget))
	{
		ClearLockOnTarget();
		return;
	}

	CurrentLockOnTarget = FindBestLockOnTarget();

	if (IsValid(CurrentLockOnTarget))
	{
		BeginLockOn();
	}
}

void ATalPlayerCharacter::ClearLockOnTarget()
{
	if (IsValid(CurrentLockOnTarget))
	{
		CurrentLockOnTarget->SetLockOnWidgetVisible(false);
	}

	CurrentLockOnTarget = nullptr;
	bIsLockOnActive = false;

	//일반 이동, 카메라 상태 복구
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

UNTLockOnTargetComponent* ATalPlayerCharacter::FindBestLockOnTarget() const
{
	MC_LOG(LogSY, Log, TEXT("최고의 락온 대상을 찾는 중"));
	const APlayerController* PlayerController = Cast<APlayerController>(Controller);

	if (!IsValid(PlayerController)) return nullptr;

	int32 ViewportWidth = 0;
	int32 ViewportHeight = 0;
	PlayerController->GetViewportSize(ViewportWidth, ViewportHeight);

	if (ViewportWidth <= 0 || ViewportHeight <= 0) return nullptr;

	const FVector2D ScreenCenter(ViewportWidth * 0.5f, ViewportHeight * 0.5f);

	UNTLockOnTargetComponent* BestTarget = nullptr;
	float BestScreenDistanceSquared = TNumericLimits<float>::Max();

	for (TObjectIterator<UNTLockOnTargetComponent> It; It; ++It)
	{
		UNTLockOnTargetComponent* Candidate = *It;
		FVector2D CandidateScreenPosition;

		if (!TryGetValidLockOnCandidateScreenPosition(
			Candidate,
			PlayerController,
			CandidateScreenPosition))
		{
			continue;
		}

		const float ScreenDistanceSquared =
			FVector2D::DistSquared(
				CandidateScreenPosition,
				ScreenCenter
			);

		if (ScreenDistanceSquared < BestScreenDistanceSquared)
		{
			BestScreenDistanceSquared = ScreenDistanceSquared;
			BestTarget = Candidate;
		}

	}
	return BestTarget;
}

void ATalPlayerCharacter::UpdateLockOn(float DeltaTime)
{
	if (!IsValid(CurrentLockOnTarget)
		|| !CurrentLockOnTarget->IsLockOnAvailable()
		|| Controller == nullptr)
	{
		ClearLockOnTarget();
		return;
	}

	const FVector TargetLocation = CurrentLockOnTarget->GetLockOnLocation();
	const float DistanceSquared = FVector::DistSquared(GetActorLocation(), TargetLocation);

	if (DistanceSquared > FMath::Square(MaxLockOnDistance))
	{
		ClearLockOnTarget();
		return;
	}

	const FVector CameraPivotLocation =
		IsValid(CameraTarget)
		? CameraTarget->GetComponentLocation()
		: GetActorLocation();

	const FVector DirectionToTarget = TargetLocation - CameraPivotLocation;

	if (DirectionToTarget.IsNearlyZero()) return;

	FRotator TargetControlRotation = DirectionToTarget.Rotation();

	const APlayerController* PlayerController = Cast<APlayerController>(Controller);

	int32 ViewportWidth = 0;
	int32 ViewportHeight = 0;

	if (IsValid(PlayerController) && IsValid(FollowCamera))
	{
		PlayerController->GetViewportSize(ViewportWidth, ViewportHeight);

		if (ViewportWidth > 0 && ViewportHeight > 0)
		{
			const float ScreenYRatio = FMath::Clamp(LockOnTargetScreenYRatio, 0.0f, 1.0f);

			const float AspectRatio = static_cast<float>(ViewportWidth) / static_cast<float>(ViewportHeight);

			const float HorizontalHalfFOV = FMath::DegreesToRadians(FollowCamera->FieldOfView * 0.5f);
			const float VerticalHalfFOV = FMath::Atan(FMath::Tan(HorizontalHalfFOV) / AspectRatio);

			const float TargetNDCY = 1.0f - (ScreenYRatio * 2.0f);

			const float PitchOffset = FMath::RadiansToDegrees(FMath::Atan(TargetNDCY * FMath::Tan(VerticalHalfFOV)));

			TargetControlRotation.Pitch -= PitchOffset;
		}
	}

	const float SafePitchMin = FMath::Clamp(
		FMath::Min(MinCameraPitch, MaxCameraPitch),
		-89.0f,
		89.0f
	);

	const float SafePitchMax = FMath::Clamp(
		FMath::Max(MinCameraPitch, MaxCameraPitch),
		-89.0f,
		89.0f
	);

	TargetControlRotation.Pitch = FMath::Clamp(
		FRotator::NormalizeAxis(TargetControlRotation.Pitch),
		SafePitchMin,
		SafePitchMax
	);

	TargetControlRotation.Roll = 0.0f;

	const FRotator NewControlRotation = FMath::RInterpTo(
		Controller->GetControlRotation(),
		TargetControlRotation,
		DeltaTime,
		FMath::Max(0.0f, LockOnCameraInterpSpeed)
	);

	Controller->SetControlRotation(NewControlRotation);
}

bool ATalPlayerCharacter::TryGetValidLockOnCandidateScreenPosition(
	UNTLockOnTargetComponent* Candidate,
	const APlayerController* PlayerController,
	FVector2d& OutScreenPosition
) const
{
	if (!IsValid(Candidate)
		|| Candidate->IsTemplate()
		|| Candidate->GetWorld() != GetWorld()
		|| !Candidate->IsLockOnAvailable()
		|| !IsValid(PlayerController)
		|| !IsValid(FollowCamera))
	{
		return false;
	}

	AActor* TargetActor = Candidate->GetOwner();

	if (!IsValid(TargetActor) || TargetActor == this)
	{
		return false;
	}

	const FVector TargetLocation = Candidate->GetLockOnLocation();
	const float DistanceSquared = FVector::DistSquared(
		GetActorLocation(), TargetLocation
	);

	if (DistanceSquared > FMath::Square(MaxLockOnDistance))
	{
		return false;
	}

	const FVector CameraLocation = FollowCamera->GetComponentLocation();
	const FVector CameraToTarget = TargetLocation - CameraLocation;

	if (FVector::DotProduct(FollowCamera->GetForwardVector(), CameraToTarget) <= 0.0f)
	{
		return false;
	}

	if (!PlayerController->ProjectWorldLocationToScreen(
		TargetLocation,
		OutScreenPosition,
		true))
	{
		return false;
	}

	int32 ViewportWidth = 0;
	int32 ViewportHeight = 0;

	PlayerController->GetViewportSize(
		ViewportWidth,
		ViewportHeight
	);

	const bool bInsideScreen =
		OutScreenPosition.X >= 0.0f
		&& OutScreenPosition.X <= ViewportWidth
		&& OutScreenPosition.Y >= 0.0f
		&& OutScreenPosition.Y <= ViewportHeight;

	if (!bInsideScreen) return false;

	FCollisionQueryParams QueryParams(
		SCENE_QUERY_STAT(LockOnVisibility),
		false,
		this
	);

	FHitResult VisibilityHit;

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		VisibilityHit,
		CameraLocation,
		TargetLocation,
		ECC_Visibility,
		QueryParams
	);

	return !bHit || VisibilityHit.GetActor() == TargetActor;
}

void ATalPlayerCharacter::SwitchLockOnTarget(const FInputActionValue& Value)
{
	if (bIsTalchiInProgress) return;
	if (!bIsLockOnActive || !IsValid(CurrentLockOnTarget)) return;

	const float Direction = Value.Get<float>();

	if (FMath::IsNearlyZero(Direction)) return;

	const APlayerController* PlayerController = Cast<APlayerController>(Controller);

	if (!IsValid(PlayerController)) return;

	FVector2D CurrentScreenPosition;

	if (!PlayerController->ProjectWorldLocationToScreen(
		CurrentLockOnTarget->GetLockOnLocation(),
		CurrentScreenPosition,
		true))
	{
		return;
	}

	UNTLockOnTargetComponent* BestTarget = nullptr;
	float BestHorizontalDistance = TNumericLimits<float>::Max();

	for (TObjectIterator<UNTLockOnTargetComponent> It; It; ++It)
	{
		UNTLockOnTargetComponent* Candidate = *It;

		if (Candidate == CurrentLockOnTarget) continue;

		FVector2D CandidateScreenPosition;

		if (!TryGetValidLockOnCandidateScreenPosition(
			Candidate,
			PlayerController,
			CandidateScreenPosition))
		{
			continue;
		}

		const float HorizontalDelta = CandidateScreenPosition.X - CurrentScreenPosition.X;

		const bool bCorrectDirection =
			Direction < 0.0f
			? HorizontalDelta < 0.0f
			: HorizontalDelta > 0.0f;

		if (!bCorrectDirection) continue;

		const float HorizontalDistance = FMath::Abs(HorizontalDelta);

		if (HorizontalDistance < BestHorizontalDistance)
		{
			BestHorizontalDistance = HorizontalDistance;
			BestTarget = Candidate;
		}
	}

	if (!IsValid(BestTarget)) return;

	//여기서 체인쥐
	CurrentLockOnTarget->SetLockOnWidgetVisible(false);
	CurrentLockOnTarget = BestTarget;
	CurrentLockOnTarget->SetLockOnWidgetVisible(true);
}

void ATalPlayerCharacter::UpdatePlayerCameraVisibility()
{
	// First-person visibility is owned by the talchi sequence until restoration.
	if (bTalchiFirstPersonActive) return;
	if (!IsLocallyControlled() || !IsValid(CameraTarget) || !IsValid(FollowCamera)) return;

	// 패링 카메라 연출 중에는 플레이어와 무기를 표시
	if (CombatFeelComponent &&
		CombatFeelComponent->IsParryCameraMoving())
	{
		if (bHiddenByCameraDistance)
		{
			bHiddenByCameraDistance = false;

			if (IsValid(GetMesh()))
			{
				GetMesh()->SetOwnerNoSee(false);
			}

			if (IsValid(WeaponMesh))
			{
				WeaponMesh->SetOwnerNoSee(false);
			}
		}

		return;
	}

	const float DistanceSquared = FVector::DistSquared(
		CameraTarget->GetComponentLocation(),
		FollowCamera->GetComponentLocation()
	);

	if (!bHiddenByCameraDistance && DistanceSquared <= FMath::Square(PlayerHideDistance))
	{
		bHiddenByCameraDistance = true;
		GetMesh()->SetOwnerNoSee(true);
		WeaponMesh->SetOwnerNoSee(true);
	}
	else if (bHiddenByCameraDistance && DistanceSquared >= FMath::Square(PlayerShowDistance))
	{
		bHiddenByCameraDistance = false;
		GetMesh()->SetOwnerNoSee(false);
		WeaponMesh->SetOwnerNoSee(false);
	}
}



void ATalPlayerCharacter::HandleDeath()
{
	FinishTalchi();
	if (bIsDead) return;

	bIsDead = true;

	ClearLockOnTarget(); //락온 해제
	SetActorEnableCollision(false); //갖고있는 콜리전 다 해제
	//isableWeaponCollision(); //공격 판정 해제


	//이동중단
	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		MovementComponent->StopMovementImmediately();
		MovementComponent->DisableMovement();
	}
	//입력차단
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		DisableInput(PlayerController);
	}

	//사망 애니메이션 재생
	if (DeathMontage)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Play(DeathMontage);
		}
	}

	MC_LOG(LogSY, Warning, TEXT("Player Dead!"));

	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this,
		&ATalPlayerCharacter::RespawnAtBonfire, RespawnDelay, false);
}

void ATalPlayerCharacter::RespawnAtBonfire()
{
	bIsDead = false;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	// 사망 몽타주는 AutoBlendOut이 꺼져있어 직접 안멈추면 사망 포즈로 굳음
	if (AnimInstance)
	{
		AnimInstance->Montage_Stop(0.0f, DeathMontage);
	}

	const FTransform TargetTransform = LastBonfire.IsValid()
		? LastBonfire->GetActorTransform() : InitialSpawnTransform;

	SetActorLocation(TargetTransform.GetLocation());

	SetActorRotation(TargetTransform.GetRotation());

	SetActorEnableCollision(true);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);
	}

	// TriggerRest는 전투중이면 막히므로 부활X
	StatComponent->RestoreFull();
	RechargeFlask();

	if (ANamelessTalGameMode* GameMode = Cast<ANamelessTalGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->RespawnAllMonsters();
	}

	if (LastBonfire.IsValid())
	{
		LastBonfire->OnBonfireRested.Broadcast();
	}

	if (AnimInstance && ReviveAnim) {

		AnimInstance->PlaySlotAnimationAsDynamicMontage(ReviveAnim, TEXT("DefaultSlot"));

		GetWorldTimerManager().SetTimer(ReviveTimerHandle, this,
			&ATalPlayerCharacter::EndRevive, ReviveAnim->GetPlayLength(), false);
	}
	else {
		EndRevive();
	}
}

void ATalPlayerCharacter::EndRevive()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		EnableInput(PlayerController);
	}
}

void ATalPlayerCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->Velocity *= LandingSpeedMultiplier;
	}

	if (bWasRealFallingThisAirtime && !bIsDodging && !bIsLandingRecovering)
	{
		bIsLandingRecovering = true;
		GetWorldTimerManager().SetTimer(LandingRecoveryTimeHandle, this,
			&ATalPlayerCharacter::EndLandingRecovery, LandingRecoverTime, false);
	}
	bWasRealFallingThisAirtime = false;
}

void ATalPlayerCharacter::UpdateRealFalling()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		bIsRealFalling = false;
		return;
	}

	// 이미 공중 구간에서 낙하로 판정시 유지
	if (bWasRealFallingThisAirtime)
	{
		bIsRealFalling = true;
		return;
	}

	const FVector Start = GetActorLocation();
	const FVector End = Start - FVector(0.0f, 0.0f, 2000.0f);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	FHitResult Hit;
	bool bHighEnough = false;

	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams))
	{
		const float HeightAboveGround =
			Hit.Distance - GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		bHighEnough = HeightAboveGround >= MinFallHeightToPlayAnim;
	}

	bIsRealFalling = bHighEnough;

	if (bIsRealFalling)
	{
		bWasRealFallingThisAirtime = true;
	}

}

void ATalPlayerCharacter::EndLandingRecovery()
{
	bIsLandingRecovering = false;
}

void ATalPlayerCharacter::UseFlaskItem(const FInputActionValue& Value)
{
	if (bIsTalchiInProgress)
	{
		return;
	}
	if (GuardComponent->IsStaggered() || StatComponent->IsGroggy() || StatComponent->IsSoulExhausted() ||
		bIsLandingRecovering || bIsDodging || bIsUsingFlask || bIsUsingSpecialArt || FlaskCount <= 0
		|| FlaskMontage == nullptr)
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance == nullptr) return;

	bIsUsingFlask = true;

	PlayAnimMontage(FlaskMontage);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ATalPlayerCharacter::OnFlaskMontageEnded);

	AnimInstance->Montage_SetEndDelegate(EndDelegate, FlaskMontage);
}

void ATalPlayerCharacter::CancelFlaskUse()
{
	if (!bIsUsingFlask) return;

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Stop(0.2f, FlaskMontage);
	}
	// bIsUsingFlask는 OnFlaskMontageEnded가 블렌드아웃 끝나고 false로 바꿔줌
}

void ATalPlayerCharacter::OnFlaskMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	bIsUsingFlask = false;
}

void ATalPlayerCharacter::ApplyFlaskHeal()
{
	if (!bIsUsingFlask || FlaskCount <= 0)
	{
		return;
	}

	FlaskCount--;

	StatComponent->HealHp(StatComponent->MaxHP * FlaskHealPercent);
}

void ATalPlayerCharacter::RechargeFlask()
{
	FlaskCount = FlaskMaxCount;;
}

void ATalPlayerCharacter::StartVictimTalchiMontage()
{
	if (!bIsTalchiInProgress)
	{
		return;
	}

	ANTNPCCharacterBase* Victim = ActiveTalchiVictim.Get();

	if (!IsValid(Victim)
		|| Victim->IsActorBeingDestroyed()
		|| !SelectedTalChiTarget.IsValid()
		|| SelectedTalChiTarget->GetOwner() != Victim)
	{
		FinishTalchi();
		return;
	}

	UAnimMontage* VictimMontage =
		SelectedTalChiTarget->GetVictimTalchiMontage();

	if (!IsValid(VictimMontage))
	{
		FinishTalchi();
		return;
	}

	// 동일 노티파이가 중복 호출돼도 다시 재생하지 않습니다.
	if (IsValid(ActiveVictimTalchiMontage))
	{
		return;
	}

	ActiveVictimTalchiMontage = VictimMontage;

	const float MontageLength =
		Victim->PlayAnimMontage(VictimMontage);

	if (MontageLength <= 0.0f)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("Talchi: victim montage failed."));

		FinishTalchi();
		return;
	}

	MC_LOG(LogSY, Warning, TEXT("적탈취 모션 시작"));
}
void ATalPlayerCharacter::GrabSelectedTalToHand()
{
	if (!bIsTalchiInProgress)
	{
		return;
	}
	static const FName TalGrabSocketName(TEXT("TalGrabSocket"));

	if (IsValid(HeldStolenTal)) return;
	if (!SelectedTalChiTarget.IsValid() || !IsValid(GetMesh())) return;
	if (!GetMesh()->DoesSocketExist(TalGrabSocketName)) return;

	AActor* TargetActor = SelectedTalChiTarget->GetOwner();
	if (!IsValid(TargetActor)) return;

	UNTTalComponent* TargetTalComponent = TargetActor->FindComponentByClass<UNTTalComponent>();
	if (!IsValid(TargetTalComponent) || !IsValid(TargetTalComponent->GetCurrentTal())) return;

	ATalBase* StolenTal = TargetTalComponent->UnequipTal();
	if (!IsValid(StolenTal)) return;

	const bool bAttached = StolenTal->AttachToComponent(
		GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		TalGrabSocketName
	);

	if (!bAttached)
	{
		TargetTalComponent->EquipTal(StolenTal);
		return;
	}

	StolenTal->SetOwner(this);
	HeldStolenTal = StolenTal;
	HeldTalVictim = TargetActor;
}

void ATalPlayerCharacter::EquipHeldTalToFace()
{
	if (!bIsTalchiInProgress)
	{
		return;
	}
	if (!IsValid(TalComponent) || !IsValid(HeldStolenTal)) return;

	// 카메라에 붙어 있던 연출용 부착을 먼저 풀어야 얼굴 소켓에 정상적으로 붙습니다.
	EndTalchiMaskApproach();

	ATalBase* TalToEquip = HeldStolenTal.Get();
	ATalBase* OldPlayerTal = TalComponent->UnequipTal();

	if (!TalComponent->EquipTal(TalToEquip))
	{
		if (IsValid(OldPlayerTal))
		{
			TalComponent->EquipTal(OldPlayerTal);
		}

		return;
	}
	
	//얼굴 장착 성공
	// 1인칭 연출 중이면 화면 오른쪽 자리에 그대로 붙여 둡니다.
	SettleTalchiRightMask();
	AActor* VictimActor = HeldTalVictim.Get();

	HeldStolenTal = nullptr;
	HeldTalVictim.Reset();

	SpecialArtUseCount = 0;

	if (IsValid(OldPlayerTal))
	{
		OldPlayerTal->Destroy();
	}

	if (IsValid(VictimActor))
	{
		INTNPCCharacterBaseInterface* VictimNPC = Cast<INTNPCCharacterBaseInterface>(VictimActor);

		if (VictimNPC)
		{
			VictimNPC->TalChiNPC(FNPCTalchiData{});
		}
	}
}

bool ATalPlayerCharacter::CanTalchiTarget(AActor* TargetActor) const
{
	if (!GetWorld()
		|| !IsValid(TargetActor)
		|| TargetActor == this
		|| TargetActor->IsActorBeingDestroyed())
	{
		return false;
	}

	// 1. 그로기 상태인지 확인
	INTNPCCharacterBaseInterface* TargetNPC =
		Cast<INTNPCCharacterBaseInterface>(TargetActor);

	if (!TargetNPC || !TargetNPC->GetIsExhausting())
	{
		return false;
	}

	// 2. 탈을 가지고 있는지 확인
	UNTTalComponent* TargetTalComponent =
		TargetActor->FindComponentByClass<UNTTalComponent>();

	if (!IsValid(TargetTalComponent)
		|| !IsValid(TargetTalComponent->GetCurrentTal()))
	{
		return false;
	}

	// 3. 탈취 설정과 정면 부채꼴 확인
	UNTTalChiTargetComponent* TargetPoint =
		TargetActor->FindComponentByClass<UNTTalChiTargetComponent>();

	if (!IsValid(TargetPoint)
		|| !TargetPoint->IsTalchiAvailable()
		|| !TargetPoint->IsInsideTalchiSector(this))
	{
		return false;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());

	if (!IsValid(PC) || !PC->IsLocalController())
	{
		return false;
	}

	// 4. 적의 중심이 화면 안에 있는지 확인
	// 캐릭터 액터의 위치는 보통 캡슐 중심입니다.
	const FVector TargetLocation = TargetActor->GetActorLocation();

	FVector2D ScreenPosition;
	if (!PC->ProjectWorldLocationToScreen(
		TargetLocation, ScreenPosition, false))
	{
		return false;
	}

	int32 ViewportWidth = 0;
	int32 ViewportHeight = 0;
	PC->GetViewportSize(ViewportWidth, ViewportHeight);

	if (ViewportWidth <= 0 || ViewportHeight <= 0
		|| ScreenPosition.X < 0.0f
		|| ScreenPosition.Y < 0.0f
		|| ScreenPosition.X >= ViewportWidth
		|| ScreenPosition.Y >= ViewportHeight)
	{
		return false;
	}

	// 5. 플레이어와 적 사이 / 카메라와 적 사이 장애물 검사
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(TargetActor);

	if (ATalBase* MyTal = GetCurrentTal())
	{
		QueryParams.AddIgnoredActor(MyTal);
	}

	QueryParams.AddIgnoredActor(TargetTalComponent->GetCurrentTal());

	FHitResult Hit;

	const bool bBlockedFromPlayer =
		GetWorld()->LineTraceSingleByChannel(
			Hit,
			GetActorLocation(),
			TargetLocation,
			ECC_Visibility,
			QueryParams);

	if (bBlockedFromPlayer)
	{
		return false;
	}

	FVector CameraLocation;
	FRotator CameraRotation;
	PC->GetPlayerViewPoint(CameraLocation, CameraRotation);

	const bool bBlockedFromCamera =
		GetWorld()->LineTraceSingleByChannel(
			Hit,
			CameraLocation,
			TargetLocation,
			ECC_Visibility,
			QueryParams);

	return !bBlockedFromCamera;
}

AActor* ATalPlayerCharacter::FindBestTalchiTarget() const
{
	if (!GetWorld())
	{
		return nullptr;
	}

	AActor* BestTarget = nullptr;
	double BestDistanceSquared = TNumericLimits<double>::Max();

	for (TActorIterator<ACharacter> It(GetWorld()); It; ++It)
	{
		ACharacter* Candidate = *It;

		if (!IsValid(Candidate) || Candidate == this)
		{
			continue;
		}

		const double DistanceSquared = FVector::DistSquared(
			GetActorLocation(),
			Candidate->GetActorLocation());

		// 이미 찾은 대상보다 멀면 추가 검사하지 않습니다.
		if (DistanceSquared >= BestDistanceSquared)
		{
			continue;
		}

		if (!CanTalchiTarget(Candidate))
		{
			continue;
		}

		BestTarget = Candidate;
		BestDistanceSquared = DistanceSquared;
	}

	return BestTarget;
}

void ATalPlayerCharacter::HideTalchiPrompt()
{
	FocusedTalchiTarget.Reset();

	if (IsValid(TalchiPromptWidget))
	{
		TalchiPromptWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ATalPlayerCharacter::UpdateTalchiPrompt(float DeltaTime)
{
	APlayerController* PC = Cast<APlayerController>(GetController());

	if (!IsValid(PC) || !PC->IsLocalController() || bIsDead)
	{
		HideTalchiPrompt();
		return;
	}

	if (bIsTalchiInProgress)
	{
		HideTalchiPrompt();
		return;
	}

	// 컨트롤러가 준비된 시점에 한 번 생성합니다.
	if (!IsValid(TalchiPromptWidget))
	{
		if (!TalchiPromptWidgetClass)
		{
			HideTalchiPrompt();
			return;
		}

		TalchiPromptWidget =
			CreateWidget<UUserWidget>(PC, TalchiPromptWidgetClass);

		if (!IsValid(TalchiPromptWidget))
		{
			HideTalchiPrompt();
			return;
		}

		TalchiPromptWidget->AddToViewport(10);
		TalchiPromptWidget->SetAlignmentInViewport(
			FVector2D(0.5f, 0.5f));
		TalchiPromptWidget->SetVisibility(
			ESlateVisibility::Collapsed);
	}

	// 전체 대상 검색은 0.1초마다 수행합니다.
	TalchiSearchElapsed += DeltaTime;

	if (TalchiSearchElapsed >= 0.1f)
	{
		TalchiSearchElapsed = 0.0f;
		FocusedTalchiTarget = FindBestTalchiTarget();
	}

	AActor* TargetActor = FocusedTalchiTarget.Get();

	// 선택된 대상이 조건을 잃으면 즉시 숨깁니다.
	if (!CanTalchiTarget(TargetActor))
	{
		HideTalchiPrompt();
		return;
	}

	// 위치는 매 프레임 갱신하여 카메라 움직임을 따라갑니다.
	FVector2D WidgetPosition;

	const bool bProjected =
		UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
			PC,
			TargetActor->GetActorLocation(),
			WidgetPosition,
			false);

	if (!bProjected)
	{
		HideTalchiPrompt();
		return;
	}

	// 이 좌표에는 DPI 보정이 이미 반영되어 있으므로 false입니다.
	TalchiPromptWidget->SetPositionInViewport(
		WidgetPosition + TalchiPromptScreenOffset,
		false);

	// 마우스 입력을 가로채지 않는 표시 전용 위젯입니다.
	TalchiPromptWidget->SetVisibility(
		ESlateVisibility::HitTestInvisible);
}

void ATalPlayerCharacter::EndPlay(
	const EEndPlayReason::Type EndPlayReason)
{

	if (IsValid(TalchiPromptWidget))
	{
		TalchiPromptWidget->RemoveFromParent();
		TalchiPromptWidget = nullptr;
	}

	FocusedTalchiTarget.Reset();

	FinishTalchi();

	Super::EndPlay(EndPlayReason);
}

void ATalPlayerCharacter::OnTalchiMontageEnded(
	UAnimMontage* Montage,
	bool bInterrupted)
{
	if (Montage != ActiveTalchiMontage.Get())
	{
		return;
	}

	FinishTalchi();
}

void ATalPlayerCharacter::EnterTalchiFirstPerson()
{
	if ((!bIsTalchiInProgress && !bTalchiPreviewActive) || bTalchiFirstPersonActive || !IsLocallyControlled()
		|| !IsValid(TalchiCamera) || !IsValid(FollowCamera) || !IsValid(GetMesh()))
	{
		return;
	}

	// The existing body animation supplies the hands; only hide the head subtree.
	// Do not hide a guessed ancestor bone, which could also hide the arms.
	if (!TalchiFirstPersonHeadBone.IsNone()
		&& GetMesh()->GetBoneIndex(TalchiFirstPersonHeadBone) == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("Talchi: first-person head bone not found: %s"),
			*TalchiFirstPersonHeadBone.ToString());
		return;
	}

	bTalchiSavedFollowCameraActive = FollowCamera->IsActive();
	bTalchiSavedFirstPersonCameraActive = TalchiCamera->IsActive();
	bTalchiSavedMeshOwnerNoSee = GetMesh()->bOwnerNoSee;
	bTalchiSavedWeaponOwnerNoSee = IsValid(WeaponMesh) && WeaponMesh->bOwnerNoSee;
	TalchiHiddenHeadBone = TalchiFirstPersonHeadBone;
	bTalchiSavedHeadHidden = !TalchiHiddenHeadBone.IsNone()
		&& GetMesh()->IsBoneHiddenByName(TalchiHiddenHeadBone);
	bTalchiFirstPersonActive = true;

	GetMesh()->SetOwnerNoSee(false);
	if (!TalchiHiddenHeadBone.IsNone() && !bTalchiSavedHeadHidden)
	{
		GetMesh()->HideBoneByName(TalchiHiddenHeadBone, PBO_None);
	}
	if (IsValid(WeaponMesh)) WeaponMesh->SetOwnerNoSee(true);

	if (!bTalchiPreviewActive)
	{
		// 교체될 예전 오른쪽 탈을 치워 빼앗은 탈이 들어올 자리를 비웁니다.
		HideTalchiFaceMask();
	}

	FollowCamera->Deactivate();
	TalchiCamera->Activate();
	BeginTalchiLeftMaskFraming();

	if (bTalchiPreviewActive)
	{
		// 미리보기에는 빼앗은 탈이 없으므로 지금 쓰고 있는 탈을 그 자리에 놓습니다.
		SettleTalchiRightMask();
	}
	else
	{
		BeginTalchiMaskApproach();
	}
}

// 기본 니어 클립이 10cm라 그보다 가까우면 탈이 잘려 보입니다.
static constexpr float TalchiMaskMinCoverDistance = 12.0f;

FTransform ATalPlayerCharacter::GetTalchiRightMaskCoverTransform() const
{
	FVector CoverOffset = TalchiRightMaskOffset;
	CoverOffset.X = FMath::Max(CoverOffset.X, TalchiMaskMinCoverDistance);

	return FTransform(
		TalchiRightMaskRotation,
		CoverOffset,
		FVector(FMath::Max(0.01f, TalchiRightMaskScale)));
}

FTransform ATalPlayerCharacter::GetTalchiLeftMaskFrameTransform() const
{
	FVector LeftOffset = TalchiLeftMaskOffset;
	LeftOffset.X = FMath::Max(LeftOffset.X, TalchiMaskMinCoverDistance);

	return FTransform(
		TalchiLeftMaskRotation,
		LeftOffset,
		TalchiLeftMaskSavedTransform.GetScale3D() * FMath::Max(0.01f, TalchiLeftMaskScale));
}

void ATalPlayerCharacter::BeginTalchiLeftMaskFraming()
{
	if (bTalchiLeftMaskFramed || !IsValid(LeftTalMesh) || !IsValid(TalchiCamera)) return;

	TalchiLeftMaskSavedTransform = LeftTalMesh->GetRelativeTransform();

	// 오른쪽 탈과 같은 부모에 두어야 중앙 이음새가 흔들리지 않습니다.
	if (!LeftTalMesh->AttachToComponent(
		TalchiCamera, FAttachmentTransformRules::KeepRelativeTransform))
	{
		return;
	}

	bTalchiLeftMaskFramed = true;

	LeftTalMesh->SetRelativeTransform(GetTalchiLeftMaskFrameTransform());
}

void ATalPlayerCharacter::RestoreTalchiLeftMask()
{
	if (!bTalchiLeftMaskFramed) return;
	bTalchiLeftMaskFramed = false;

	if (!IsValid(LeftTalMesh) || !IsValid(GetMesh())) return;

	LeftTalMesh->AttachToComponent(
		GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, LeftTalSocketName);
	LeftTalMesh->SetRelativeTransform(TalchiLeftMaskSavedTransform);
}

void ATalPlayerCharacter::SettleTalchiRightMask()
{
	if (!bTalchiFirstPersonActive || bTalchiRightMaskSettled || !IsValid(TalchiCamera)) return;

	ATalBase* Mask = GetCurrentTal();
	USceneComponent* MaskRoot = IsValid(Mask) ? Mask->GetRootComponent() : nullptr;
	if (!MaskRoot) return;

	TalchiRightMaskSavedScale3D = MaskRoot->GetRelativeScale3D();

	// 장착 때문에 얼굴 소켓으로 끌려간 탈을 연출 구도로 되돌립니다.
	if (!Mask->AttachToComponent(
		TalchiCamera, FAttachmentTransformRules::KeepRelativeTransform))
	{
		return;
	}

	MaskRoot->SetRelativeTransform(GetTalchiRightMaskCoverTransform());
	bTalchiRightMaskSettled = true;
}

void ATalPlayerCharacter::RestoreTalchiRightMask()
{
	if (!bTalchiRightMaskSettled) return;
	bTalchiRightMaskSettled = false;

	ATalBase* Mask = GetCurrentTal();
	if (!IsValid(Mask) || !IsValid(TalComponent) || !IsValid(GetMesh())) return;

	Mask->AttachToComponent(
		GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		TalComponent->GetTalSocketName());

	// 연출용으로 덮어썼던 스케일을 되돌립니다.
	if (USceneComponent* MaskRoot = Mask->GetRootComponent())
	{
		MaskRoot->SetRelativeScale3D(TalchiRightMaskSavedScale3D);
	}
}

void ATalPlayerCharacter::UpdateTalchiMaskPreview()
{
	if (!bTalchiPreviewActive) return;

	// 미리보기 중에는 디테일 패널에서 바꾼 값이 바로 보이도록 매 프레임 다시 적용합니다.
	if (bTalchiLeftMaskFramed && IsValid(LeftTalMesh))
	{
		LeftTalMesh->SetRelativeTransform(GetTalchiLeftMaskFrameTransform());
	}

	if (!bTalchiRightMaskSettled) return;

	ATalBase* Mask = GetCurrentTal();
	USceneComponent* MaskRoot = IsValid(Mask) ? Mask->GetRootComponent() : nullptr;
	if (!MaskRoot) return;

	MaskRoot->SetRelativeTransform(GetTalchiRightMaskCoverTransform());
}

void ATalPlayerCharacter::NTPreviewTalchiMask()
{
	if (bTalchiPreviewActive)
	{
		bTalchiPreviewActive = false;
		RestoreTalchiFirstPerson();

		MC_LOG(LogSY, Warning, TEXT("탈 미리보기 종료"));
		return;
	}

	// 실제 탈취 연출과 겹치면 양쪽 복구가 서로 꼬입니다.
	if (bIsTalchiInProgress || bTalchiFirstPersonActive)
	{
		MC_LOG(LogSY, Warning, TEXT("탈 미리보기 실패: 탈취 연출이 진행 중입니다"));
		return;
	}

	// 오른쪽 자리에는 지금 쓰고 있는 탈을 세워 구도를 확인합니다.
	if (!IsValid(GetCurrentTal()))
	{
		MC_LOG(LogSY, Warning, TEXT("탈 미리보기 실패: 착용 중인 탈이 없습니다"));
		return;
	}

	bTalchiPreviewActive = true;
	EnterTalchiFirstPerson();

	if (!bTalchiFirstPersonActive)
	{
		// 진입에 실패했다면 상태를 남기지 않습니다.
		bTalchiPreviewActive = false;

		MC_LOG(LogSY, Warning, TEXT("탈 미리보기 실패: 1인칭 진입에 실패했습니다"));
		return;
	}

	UpdateTalchiMaskPreview();

	MC_LOG(LogSY, Warning, TEXT("탈 미리보기 시작"));
}

void ATalPlayerCharacter::BeginTalchiMaskApproach()
{
	if (!bIsTalchiInProgress || bTalchiMaskApproachActive
		|| !IsLocallyControlled() || !IsValid(TalchiCamera) || !GetWorld())
	{
		return;
	}

	ATalBase* Mask = HeldStolenTal.Get();
	USceneComponent* MaskRoot = IsValid(Mask) ? Mask->GetRootComponent() : nullptr;
	if (!MaskRoot) return;

	// 월드 트랜스폼을 유지해 손에서 카메라로 옮기는 순간 튀지 않게 합니다.
	if (!Mask->AttachToComponent(
		TalchiCamera, FAttachmentTransformRules::KeepWorldTransform))
	{
		UE_LOG(LogTemp, Warning, TEXT("Talchi: could not attach mask to the first-person camera."));
		return;
	}

	TalchiApproachMask = Mask;
	TalchiMaskApproachStart = MaskRoot->GetRelativeTransform();
	TalchiMaskApproachStartTime = GetWorld()->GetRealTimeSeconds();
	bTalchiMaskApproachActive = true;

	if (bTalchiMaskDisableShadow)
	{
		TArray<UPrimitiveComponent*> MaskPrimitives;
		Mask->GetComponents<UPrimitiveComponent>(MaskPrimitives);
		for (UPrimitiveComponent* Primitive : MaskPrimitives)
		{
			bTalchiMaskSavedCastShadow = Primitive->CastShadow;
			Primitive->SetCastShadow(false);
		}
	}

	UpdateTalchiMaskApproach();
}

void ATalPlayerCharacter::UpdateTalchiMaskApproach()
{
	if (!bTalchiMaskApproachActive || !GetWorld()) return;

	ATalBase* Mask = TalchiApproachMask.Get();
	USceneComponent* MaskRoot = IsValid(Mask) ? Mask->GetRootComponent() : nullptr;
	if (!MaskRoot)
	{
		EndTalchiMaskApproach();
		return;
	}

	const float Duration = FMath::Max(0.0f, TalchiMaskApproachDuration);
	const float Alpha = Duration <= 0.0f
		? 1.0f
		: FMath::Clamp(static_cast<float>(
			(GetWorld()->GetRealTimeSeconds() - TalchiMaskApproachStartTime) / Duration), 0.0f, 1.0f);

	const float Eased = IsValid(TalchiMaskApproachCurve)
		? TalchiMaskApproachCurve->GetFloatValue(Alpha)
		: Alpha * Alpha * (3.0f - 2.0f * Alpha);

	FTransform Current;
	Current.Blend(TalchiMaskApproachStart, GetTalchiRightMaskCoverTransform(), Eased);
	MaskRoot->SetRelativeTransform(Current);
}

void ATalPlayerCharacter::EndTalchiMaskApproach()
{
	if (!bTalchiMaskApproachActive) return;
	bTalchiMaskApproachActive = false;

	ATalBase* Mask = TalchiApproachMask.Get();
	TalchiApproachMask.Reset();

	if (!IsValid(Mask)) return;

	if (bTalchiMaskDisableShadow)
	{
		TArray<UPrimitiveComponent*> MaskPrimitives;
		Mask->GetComponents<UPrimitiveComponent>(MaskPrimitives);
		for (UPrimitiveComponent* Primitive : MaskPrimitives)
		{
			Primitive->SetCastShadow(bTalchiMaskSavedCastShadow);
		}
	}

	// 카메라에 붙은 채로 두면 얼굴 장착과 반환이 모두 꼬입니다.
	USceneComponent* MaskRoot = Mask->GetRootComponent();
	if (MaskRoot && MaskRoot->GetAttachParent() == TalchiCamera)
	{
		Mask->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
}

void ATalPlayerCharacter::HideTalchiFaceMask()
{
	ATalBase* FaceMask = GetCurrentTal();
	if (!bTalchiFirstPersonActive || !IsValid(FaceMask)) return;

	const TWeakObjectPtr<AActor> Key(FaceMask);
	if (!TalchiSavedMaskVisibility.Contains(Key))
	{
		TalchiSavedMaskVisibility.Add(Key, FaceMask->IsHidden());
	}
	FaceMask->SetActorHiddenInGame(true);
}

void ATalPlayerCharacter::RestoreTalchiFirstPerson()
{
	EndTalchiMaskApproach();
	RestoreTalchiRightMask();
	RestoreTalchiLeftMask();
	if (!bTalchiFirstPersonActive) return;
	bTalchiFirstPersonActive = false;

	if (IsValid(GetMesh()))
	{
		GetMesh()->SetOwnerNoSee(bTalchiSavedMeshOwnerNoSee);
		if (!TalchiHiddenHeadBone.IsNone() && !bTalchiSavedHeadHidden)
		{
			GetMesh()->UnHideBoneByName(TalchiHiddenHeadBone);
		}
	}
	TalchiHiddenHeadBone = NAME_None;
	if (IsValid(WeaponMesh)) WeaponMesh->SetOwnerNoSee(bTalchiSavedWeaponOwnerNoSee);
	for (const auto& Entry : TalchiSavedMaskVisibility)
	{
		if (AActor* Mask = Entry.Key.Get()) Mask->SetActorHiddenInGame(Entry.Value);
	}
	TalchiSavedMaskVisibility.Empty();
	if (IsValid(TalchiCamera)) TalchiCamera->SetActive(bTalchiSavedFirstPersonCameraActive);
	if (IsValid(FollowCamera)) FollowCamera->SetActive(bTalchiSavedFollowCameraActive);
}

void ATalPlayerCharacter::BeginTalchiCamera(AActor* TargetActor)
{
	if (bTalchiCameraActive || !IsValid(CameraBoom)
		|| !IsValid(Controller) || !IsValid(TargetActor) || !GetWorld())
	{
		return;
	}

	TalchiCameraOriginalArmLength = CameraBoom->TargetArmLength;
	bTalchiOriginalUseControlRotation = CameraBoom->bUsePawnControlRotation;
	bTalchiOriginalRotationLag = CameraBoom->bEnableCameraRotationLag;
	TalchiCameraTargetArmLength = FMath::Max(0.0f, TalchiCameraDistance);
	TalchiCameraActiveBlendDuration = FMath::Max(0.0f, TalchiCameraBlendDuration);
	TalchiCameraStartTime = GetWorld()->GetRealTimeSeconds();
	bTalchiCameraActive = true;

	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraRotationLag = false;

	// Align yaw once; leave pitch and subsequent player look input intact.
	const FVector ToTarget = TargetActor->GetActorLocation() - GetActorLocation();
	if (ToTarget.SizeSquared2D() > UE_KINDA_SMALL_NUMBER)
	{
		FRotator Rotation = Controller->GetControlRotation();
		Rotation.Yaw = ToTarget.Rotation().Yaw;
		Controller->SetControlRotation(Rotation);
	}

	UpdateRotationMode();
	UpdateTalchiCamera();
}

void ATalPlayerCharacter::UpdateTalchiCamera()
{
	if (!bTalchiCameraActive || !IsValid(CameraBoom) || !GetWorld()) return;

	// Real time keeps the authored 0.2-second transition stable during hit stop.
	const double Elapsed = GetWorld()->GetRealTimeSeconds() - TalchiCameraStartTime;
	const float Alpha = TalchiCameraActiveBlendDuration <= 0.0f
		? 1.0f
		: FMath::Clamp(static_cast<float>(Elapsed / TalchiCameraActiveBlendDuration), 0.0f, 1.0f);
	const float SmoothAlpha = Alpha * Alpha * (3.0f - 2.0f * Alpha);
	CameraBoom->TargetArmLength = FMath::Lerp(
		TalchiCameraOriginalArmLength, TalchiCameraTargetArmLength, SmoothAlpha);
}

void ATalPlayerCharacter::RestoreTalchiCamera()
{
	if (!bTalchiCameraActive) return;
	bTalchiCameraActive = false;

	if (IsValid(CameraBoom))
	{
		CameraBoom->TargetArmLength = TalchiCameraOriginalArmLength;
		CameraBoom->bUsePawnControlRotation = bTalchiOriginalUseControlRotation;
		CameraBoom->bEnableCameraRotationLag = bTalchiOriginalRotationLag;
	}
}

void ATalPlayerCharacter::FinishTalchi()
{
	RestoreTalchiFirstPerson();
	RestoreTalchiCamera();
	ClearTalchiSlow();
	// 여러 경로에서 정리를 호출해도 중복 실행하지 않습니다.
	if (!bIsTalchiInProgress)
	{
		return;
	}

	bIsTalchiInProgress = false;

	// 정리 중 발생하는 종료 콜백을 무시하도록 먼저 비웁니다.
	UAnimMontage* MontageToStop = ActiveTalchiMontage.Get();
	ActiveTalchiMontage = nullptr;

	if (UAnimInstance* AnimInstance =
		GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
	{
		if (MontageToStop
			&& AnimInstance->Montage_IsActive(MontageToStop))
		{
			AnimInstance->Montage_Stop(0.1f, MontageToStop);
		}
	}

	// 손에만 들고 있고 얼굴에 장착하지 못한 탈은 반환합니다.
	// 얼굴 장착이 성공했다면 기존 코드에서 HeldStolenTal을 비웁니다.
	if (IsValid(HeldStolenTal))
	{
		ATalBase* TalToReturn = HeldStolenTal.Get();
		AActor* VictimActor = HeldTalVictim.Get();

		HeldStolenTal = nullptr;
		HeldTalVictim.Reset();

		TalToReturn->DetachFromActor(
			FDetachmentTransformRules::KeepWorldTransform);
		TalToReturn->SetOwner(nullptr);

		UNTTalComponent* VictimTalComponent =
			IsValid(VictimActor)
			&& !VictimActor->IsActorBeingDestroyed()
			? VictimActor->FindComponentByClass<UNTTalComponent>()
			: nullptr;

		const bool bReturned =
			IsValid(VictimTalComponent)
			&& VictimTalComponent->EquipTal(TalToReturn);

		if (!bReturned)
		{
			// 반환 대상이 사라졌거나 다른 탈이 장착된 경우,
			// 탈은 현재 월드 위치에 분리된 상태로 남깁니다.
			UE_LOG(LogTemp, Warning,
				TEXT("Talchi: interrupted mask could not be returned."));
		}
	}

	ANTNPCCharacterBase* Victim = ActiveTalchiVictim.Get();
	UAnimMontage* VictimMontage = ActiveVictimTalchiMontage.Get();

	// 콜백이 발생해도 같은 대상을 다시 정리하지 않도록 먼저 비웁니다.
	ActiveTalchiVictim.Reset();
	ActiveVictimTalchiMontage = nullptr;

	if (IsValid(Victim) && !Victim->IsActorBeingDestroyed())
	{
		// 탈취 몽타주만 중지합니다.
		// 별도로 재생 중인 사망 몽타주는 건드리지 않습니다.
		if (UAnimInstance* VictimAnim =
			Victim->GetMesh() ? Victim->GetMesh()->GetAnimInstance() : nullptr)
		{
			if (VictimMontage
				&& VictimAnim->Montage_IsActive(VictimMontage))
			{
				VictimAnim->Montage_Stop(0.1f, VictimMontage);
			}
		}

		Victim->EndTalchiHold();
	}

	HeldTalVictim.Reset();
	SelectedTalChiTarget.Reset();

	if (MotionWarpingComponent)
	{
		MotionWarpingComponent->RemoveWarpTarget(
			FName(TEXT("TalchiTarget")));
	}

	HideTalchiPrompt();

	// 다음 갱신에서 바로 대상 검색을 수행합니다.
	TalchiSearchElapsed = 0.1f;
}

void ATalPlayerCharacter::UpdateTalchiSlow()
{
	if (!bIsTalchiInProgress || !GetWorld())
	{
		ClearTalchiSlow();
		return;
	}

	TSet<TWeakObjectPtr<AActor>> DesiredActors;

	const float Radius = FMath::Max(0.0f, TalchiSlowRadius);
	const float RadiusSquared = FMath::Square(Radius);
	const float Multiplier =
		FMath::Clamp(TalchiSlowMultiplier, 0.01f, 1.0f);

	ANTNPCCharacterBase* Victim = ActiveTalchiVictim.Get();

	// 현재 범위 안에 있는 살아 있는 NPC를 찾습니다.
	for (TActorIterator<ANTNPCCharacterBase> It(GetWorld()); It; ++It)
	{
		ANTNPCCharacterBase* NPC = *It;

		if (!IsValid(NPC)
			|| NPC->IsActorBeingDestroyed()
			|| NPC == Victim)
		{
			continue;
		}

		const UStatComponent* NPCStat =
			NPC->FindComponentByClass<UStatComponent>();

		if (!IsValid(NPCStat) || NPCStat->IsDead())
		{
			continue;
		}

		if (FVector::DistSquared(
			GetActorLocation(), NPC->GetActorLocation()) > RadiusSquared)
		{
			continue;
		}

		DesiredActors.Add(TWeakObjectPtr<AActor>(NPC));

		// AIController도 별도 액터이므로 함께 적용합니다.
		if (AController* AIController = NPC->GetController())
		{
			if (IsValid(AIController)
				&& !AIController->IsActorBeingDestroyed()
				&& !AIController->IsPlayerController())
			{
				DesiredActors.Add(
					TWeakObjectPtr<AActor>(AIController));
			}
		}
	}

	// 범위를 나갔거나 사망한 대상은 기존 속도로 복구합니다.
	for (auto It = TalchiOriginalTimeDilations.CreateIterator(); It; ++It)
	{
		AActor* Actor = It.Key().Get();

		if (!IsValid(Actor))
		{
			It.RemoveCurrent();
			continue;
		}

		if (!DesiredActors.Contains(It.Key()))
		{
			Actor->CustomTimeDilation = It.Value();
			It.RemoveCurrent();
		}
	}

	// 신규 대상은 원래 속도를 한 번만 저장합니다.
	for (const TWeakObjectPtr<AActor>& WeakActor : DesiredActors)
	{
		AActor* Actor = WeakActor.Get();

		if (!IsValid(Actor))
		{
			continue;
		}

		float* OriginalDilation =
			TalchiOriginalTimeDilations.Find(WeakActor);

		if (!OriginalDilation)
		{
			TalchiOriginalTimeDilations.Add(
				WeakActor, Actor->CustomTimeDilation);

			OriginalDilation =
				TalchiOriginalTimeDilations.Find(WeakActor);
		}

		Actor->CustomTimeDilation = *OriginalDilation * Multiplier;
	}
}

void ATalPlayerCharacter::ClearTalchiSlow()
{
	for (const auto& Entry : TalchiOriginalTimeDilations)
	{
		if (AActor* Actor = Entry.Key.Get())
		{
			Actor->CustomTimeDilation = Entry.Value;
		}
	}

	TalchiOriginalTimeDilations.Empty();
	TalchiSlowElapsed = 0.0f;
}

void ATalPlayerCharacter::BeginAttackStepMove(float InOverrideDistance, float InOverrideDuration)
{
	// 기본공격 중이 아니면 무시
	if (!bBasicAttackActionLocked || ComboIndex <= 0) return;

	if (bIsDead
		|| bIsDodging
		|| bIsUsingSpecialArt
		|| bIsTalchiInProgress
		|| StatComponent->IsGroggy()
		|| GuardComponent->IsStaggered())
	{
		return;
	}

	if (bBlockAttackStepWhileFalling && GetCharacterMovement()->IsFalling())
	{
		return;
	}

	float Distance = InOverrideDistance;
	if (Distance <= 0.0f)
	{
		const int32 Index = ComboIndex - 1;
		Distance = ComboStepDistances.IsValidIndex(Index) ? ComboStepDistances[Index] : 0.0f;
	}
	if (Distance <= KINDA_SMALL_NUMBER) return;

	const float Duration = FMath::Max(
		InOverrideDuration > 0.0f ? InOverrideDuration : ComboStepDuration, 0.01f);

	// 방향: 락온 중이면 타겟 쪽, 아니면 캐릭터 정면
	FVector Dir = GetActorForwardVector();

	if (bIsLockOnActive && IsValid(CurrentLockOnTarget))
	{
		const FVector TargetLoc = CurrentLockOnTarget->GetLockOnLocation();
		const FVector ToTarget = (TargetLoc - GetActorLocation()).GetSafeNormal2D();

		if (!ToTarget.IsNearlyZero())
		{
			Dir = ToTarget;

			// 타겟을 뚫고 지나가지 않도록 거리 제한
			const float Gap =
				FVector::Dist2D(TargetLoc, GetActorLocation()) - ComboStepMinTargetGap;

			Distance = FMath::Clamp(Gap, 0.0f, Distance);
			if (Distance <= KINDA_SMALL_NUMBER) return;
		}
	}

	AttackStepDir = Dir.GetSafeNormal2D();
	if (AttackStepDir.IsNearlyZero()) return;

	AttackStepSpeed = Distance / Duration;
	AttackStepTimeLeft = Duration;
	bIsAttackStepMoving = true;
}

void ATalPlayerCharacter::StopAttackStepMove()
{
	bIsAttackStepMoving = false;
	AttackStepTimeLeft = 0.0f;
	AttackStepSpeed = 0.0f;
	AttackStepDir = FVector::ZeroVector;
}

void ATalPlayerCharacter::UpdateAttackStepMove(float DeltaTime)
{
	if (!bIsAttackStepMoving) return;

	// 공격이 끊겼으면 즉시 중단
	if (!bBasicAttackActionLocked
		|| bIsDead
		|| bIsDodging
		|| bIsUsingSpecialArt
		|| bIsTalchiInProgress
		|| StatComponent->IsGroggy()
		|| GuardComponent->IsStaggered())
	{
		StopAttackStepMove();
		return;
	}

	const float StepTime = FMath::Min(DeltaTime, AttackStepTimeLeft);
	AttackStepTimeLeft -= StepTime;

	const FVector Delta = AttackStepDir * AttackStepSpeed * StepTime;

	FHitResult Hit;
	AddActorWorldOffset(Delta, true, &Hit);   // sweep: 벽/적에 막히면 멈춤

	if (Hit.bBlockingHit || AttackStepTimeLeft <= 0.0f)
	{
		StopAttackStepMove();
	}
}