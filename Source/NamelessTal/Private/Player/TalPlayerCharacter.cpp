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


#include "InputMappingContext.h"
#include "InputAction.h"
#include "InputActionValue.h"

#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"

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
#include "DrawDebugHelpers.h"//디버그용

//무기헤더
#include "Player/TalWeapon.h"

//스탯 컴포넌트
#include "Stat/StatComponent.h"

// 패링/가드 컴포넌트
#include "Guard/GuardComponent.h"

//락온 추가
#include "NPC/Components/NTLockOnTargetComponent.h"
#include "Engine/World.h"
#include "UObject/UObjectIterator.h"

// 적 애니메이션 ( 맞춤 패링 )
#include "NPC/Interface/NTNPCCharacterBaseInterface.h"
// 모션워핑
#include "MotionWarpingComponent.h"

//Revive Setting
#include "DdoKaebiFire/DdokaebiFire.h"

#include "Game/NTGameDataSingleton.h"

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

	//이동방향으로 캐릭터 회전
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	StatComponent = CreateDefaultSubobject<UStatComponent>(TEXT("StatComponent"));
	GuardComponent = CreateDefaultSubobject<UGuardComponent>(TEXT("GuardComponent"));

	MotionWarpingComponent = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarpingComponent"));


	TalComponent = CreateDefaultSubobject<UNTTalComponent>(TEXT("TalComponent"));
}


void ATalPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bParrySlowMoActive && GetWorld()->GetRealTimeSeconds() >= ParrySlowMoEndRealTime)
	{
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
		bParrySlowMoActive = false;
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
	if (bIsLockOnActive)
	{
		UpdateLockOn(DeltaTime);
	}

	UpdateRotationMode();
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

	if (bIsSpecialArtMoving)
	{
		AddActorWorldOffset(SpecialArtMoveVelocity * DeltaTime, true);
	}
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


	// 패링,가드 성공 시(시간 초만큼 실행불가능)
	if (GuardComponent->IsStaggered() || StatComponent->IsGroggy() ||
		bIsLandingRecovering || bIsUsingFlask || bIsUsingSpecialArt)
	{
		return;
	}


	bUseControllerRotationYaw = false;//Disable rotation during dodge

	if (bIsDodging) // Already Dodging or No RollMontage
	{
		return;
	}


	DodgeDir = bIsMoving ? LastMoveInputWorldDir : GetActorForwardVector();

	const float ForwardDot = FVector::DotProduct(GetActorForwardVector(), DodgeDir);
	const float RightDot = FVector::DotProduct(GetActorRightVector(), DodgeDir);
	const float Angle = FMath::RadiansToDegrees(FMath::Atan2(RightDot, ForwardDot));

	UAnimMontage* SelectedMontage = SelectRollMontage(Angle);

	if (SelectedMontage == nullptr)
	{
		return;
	}

	bIsDodging = true;
	bIsInvincible = true;

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
	BufferedAction = EBufferedCombatAction::None;
	PlayAnimMontage(SelectedMontage);

	const float MontageDuration = SelectedMontage->GetPlayLength() - 0.3f;
	GetWorldTimerManager().SetTimer(DodgeTimerHandle, this,
		&ATalPlayerCharacter::EndDodge, MontageDuration, false);


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

	const bool bFaceTarget = bIsLockOnActive && !bIsSprinting;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = bFaceTarget;
	GetCharacterMovement()->bOrientRotationToMovement = !bFaceTarget;

}


// Player Move
void ATalPlayerCharacter::Move(const FInputActionValue& Value)
{
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
	}


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
	//락온여부
	if (bIsLockOnActive || Controller == nullptr) return;
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
	// 패링,가드 성공 시(시간 초만큼 실행불가능)
	if (GuardComponent->IsStaggered()
		|| StatComponent->IsGroggy()
		|| bIsLandingRecovering
		|| bIsUsingFlask
		|| bIsDodging
		|| bIsUsingSpecialArt
		|| !IsValid(AttackMontage)
		) return;

	switch (AttackPhase)
	{
	case EPlayerAttackPhase::None:
		StartAttackStep(1);
		break;

	case EPlayerAttackPhase::Recovery:
		if (bCanCombo)//후경직 상태일때 입력 저장
		{
			BufferedAction = EBufferedCombatAction::Attack;
		}
		break;

	case EPlayerAttackPhase::Grace:
		StartNextCombo();
		break;

	case EPlayerAttackPhase::Startup:
	case EPlayerAttackPhase::Active:
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

void ATalPlayerCharacter::OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 떄려지는 객체 확인 로그
	UE_LOG(LogTemp, Warning, TEXT("Weapon overlapped with: %s"), *GetNameSafe(OtherActor));
	if (OtherActor == nullptr || OtherActor == this || HitActors.Contains(OtherActor))
	{
		return;
	}

	INTNPCCharacterBaseInterface* NPCCharacter = Cast<INTNPCCharacterBaseInterface>(OtherActor);
	if (NPCCharacter == nullptr)
	{
		return;
	}

	if (OtherComp && OtherComp->ComponentHasTag(TEXT("WeaponCollision")))
	{
		//MC_LOG(LogMC, Log, TEXT("WeaponCollision"));
		return; // 무기끼리 부딪힌 경우는 데미지 적용 안 함
	}

	HitActors.Add(OtherActor);
	
	// 넋 안차요 때리면
	StatComponent->NotifyCombatAction();


	float DamageToApply = AttackDamage;
	float DrainToApply = 10.0f;

		if (bIsUsingSpecialArt)
		{
			DamageToApply = static_cast<float>(CurrentSpecialArtData.HPDamage);
			DrainToApply = static_cast<float>(CurrentSpecialArtData.SoulDamage);
		}

	//UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, GetController(), this, UDamageType::StaticClass());
	//드레인 수치는 임의값, 나중에 수정할것
	float bNPCDead = NPCCharacter->ApplyDamage(this, DamageToApply, DrainToApply, false);

	if (bNPCDead < 0)
	{
		StatComponent->RecoverSoul(KillSoulReward);
		UE_LOG(LogTemp, Warning, TEXT("20회복했읍니다"));
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
	SelectedTalChiTarget.Reset();

	if (!IsValid(TargetActor)
		|| TargetActor == this
		|| TargetActor->IsActorBeingDestroyed())
	{
		return;
	}

	const float MaxDistance = FMath::Max(0.0f, InteractionDistance);
	const float DistanceSquared = FVector::DistSquared(
		GetActorLocation(),
		TargetActor->GetActorLocation()
	);
	const float ActualDistance = FMath::Sqrt(DistanceSquared);

	MC_LOG(
		LogSY,
		Warning,
		TEXT("Target=%s / Distance=%.1f / MaxDistance=%.1f"),
		*GetNameSafe(TargetActor),
		ActualDistance,
		MaxDistance
	);

	if (DistanceSquared > FMath::Square(MaxDistance))
	{
		MC_LOG(LogSY, Warning, TEXT("탈을 빼앗기에 상대가 너무 멉니다"));
		return;
	}

	UNTTalComponent* TargetTalComponent = TargetActor->FindComponentByClass<UNTTalComponent>();

	if (!IsValid(TargetTalComponent) || !IsValid(TargetTalComponent->GetCurrentTal()))
	{
		MC_LOG(LogSY, Warning, TEXT("상대 탈이 없다"));
		return;
	}

	UNTTalChiTargetComponent* TargetPoint = TargetActor->FindComponentByClass<UNTTalChiTargetComponent>();

	if (!IsValid(TargetPoint) || !TargetPoint->IsTalchiAvailable())
	{
		MC_LOG(LogSY, Warning, TEXT("타겟이 없습니다"));
		return;
	}

	SelectedTalChiTarget = TargetPoint;

	const FTransform TargetTransform = TargetPoint->GetTalchiTransform();
	const FVector TargetLocation = TargetTransform.GetLocation();

	///////////////////////
	//디버그용이랑 지울것들
	DrawDebugSphere(
		GetWorld(),
		TargetLocation,
		15.0f,
		12,
		FColor::Green,
		false,
		2.0f
	);
	// 초록 화살표: 도착 후 바라볼 방향
	DrawDebugDirectionalArrow(
		GetWorld(),
		TargetLocation,
		TargetLocation + TargetTransform.GetRotation().GetForwardVector() * 80.0f,
		15.0f,
		FColor::Green,
		false,
		2.0f,
		0,
		2.0f
	);
	UE_LOG(
		LogTemp,
		Log,
		TEXT("TalChi selected: %s / Position: %s"),
		*GetNameSafe(TargetActor),
		*TargetLocation.ToString()
	);

	/////////////////////////////////
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

	MotionWarpingComponent->AddOrUpdateWarpTargetFromTransform(TalchiWarpTargetName,TargetTransform);

	const float MontageLength = PlayAnimMontage(PlayerTalchiMontage);

	if (MontageLength <= 0.0f)
	{
		MC_LOG(LogSY, Warning, TEXT("탈취 몽타주 재생 실패"));
		return;
	}

	MC_LOG(LogSY, Warning, TEXT("탈취 모션 시작"));

}

void ATalPlayerCharacter::Interact()
{
	SelectedTalChiTarget.Reset();

	if (!FollowCamera || !GetWorld()) return;

	const FVector Start = FollowCamera->GetComponentLocation();

	const float TraceLength = FVector::Distance(Start, GetActorLocation())
		+ FMath::Max(0.0f, InteractionDistance);

	const FVector End = Start + FollowCamera->GetForwardVector() * TraceLength;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	//내 탈은 무시
	if (ATalBase* MyTal = GetCurrentTal())
	{
		QueryParams.AddIgnoredActor(MyTal);
	}

	FHitResult HitResult;

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);

	//디버깅용 지울것
	DrawDebugLine(
		GetWorld(),
		Start,
		bHit ? HitResult.ImpactPoint : End,
		bHit ? FColor::Green : FColor::Red,
		false,
		2.0f,
		0,
		1.5f
	);

	if (!bHit)
	{
		MC_LOG(LogSY, Warning, TEXT("탈취 대상을 찾지 못했습니다"));
		return;
	}

	AActor* TargetActor = HitResult.GetActor();
	if (ATalBase* HitTal = Cast<ATalBase>(TargetActor))
	{
		TargetActor = HitTal->GetOwner();
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
	//입력제거
	ClearAttackBuffer();

	//후경직 상태에서만 입력받기
	bCanCombo = AttackPhase == EPlayerAttackPhase::Recovery;
}
void ATalPlayerCharacter::CloseComboWindow()
{
	bCanCombo = false;

	const bool bHasBufferedAttack = BufferedAction == EBufferedCombatAction::Attack;
	const bool bCanContinueCombo = ComboIndex > 0 && ComboIndex < MaxBasicAttackCombo;

	ClearAttackBuffer();

	if (bHasBufferedAttack && bCanContinueCombo)
	{
		StartNextCombo();
		return;
	}

	if (bCanContinueCombo)
	{
		StartComboGrace();
		return;
	}

	AttackPhase = EPlayerAttackPhase::None;
	ComboIndex = 0;
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


void ATalPlayerCharacter::UseSpecialAttack(const FInputActionValue& Value)
{
	if (GuardComponent->IsStaggered() || 
		StatComponent->IsGroggy() || 
		bIsLandingRecovering|| 
		bIsUsingFlask || 
		bIsDodging ||
		bIsUsingSpecialArt)
	{
		return;
	}

	ATalBase* MyTal = GetCurrentTal();
	if (MyTal == nullptr) { return; }

	const TArray<FName> Candidates = UNTGameDataSingleton::Get().FindPlayerSpecialArtIDsByWeapon(MyTal->GetTalStyle());
	const FName SpecialArtID = Candidates.Num() > 0 ? Candidates[0] : TEXT("SA_M001_Slash");

	const FNTPlayerSpecialArtData* ArtData = UNTGameDataSingleton::Get().GetPlayerSpecialArt(SpecialArtID);

	if (ArtData == nullptr)
	{
		MC_LOG(LogDG, Warning, TEXT("No SpecialArtData! : %s"), *SpecialArtID.ToString());
		return;
	}

	if (SpecialArtUseCount >= ArtData->UseLimit)
	{
		MC_LOG(LogDG, Warning, TEXT("특수기 사용 횟수 초과!"));
		return;
	}

	if (!StatComponent->HasEnoughSoul(static_cast<float>(ArtData->SoulCost)))
	{
		return;
	}
	StatComponent->DrainSoul(static_cast<float>(ArtData->SoulCost));

	SpecialArtUseCount++;

	bIsUsingSpecialArt = true;

	CurrentSpecialArtID = SpecialArtID;
	StartSpecialArtStep(ArtData->FirstStepID);

}

void ATalPlayerCharacter::StartSpecialArtStep(FName StepID)
{
	const FNTPlayerSpecialArtStepData* StepData = 
		UNTGameDataSingleton::Get().GetPlayerSpecialArtStep(StepID);


	if (StepData == nullptr)
	{
		MC_LOG(LogMC, Warning, TEXT("No SpecialArtStepData!"));

		CurrentSpecialArtData.NextStepID = FName(TEXT(""));
		CurrentSpecialArtData.HPDamage = 10;
		CurrentSpecialArtData.SoulDamage = 10;
		CurrentSpecialArtData.StaggerDamage = 10;
		CurrentSpecialArtData.CastTime = 0.4f;
		CurrentSpecialArtData.RecoveryTime = 0.5f;
		CurrentSpecialArtData.MoveDistance = 0.0f;
		CurrentSpecialArtData.MontageName = FName(TEXT("AM_PlayerSpecialArt1"));
	}
	else
	{
		CurrentSpecialArtData.NextStepID = StepData->NextStepID;
		CurrentSpecialArtData.HPDamage = StepData->HPDamage;
		CurrentSpecialArtData.SoulDamage = StepData->SoulDamage;
		CurrentSpecialArtData.StaggerDamage = StepData->StaggerDamage;
		CurrentSpecialArtData.CastTime = StepData->CastTime;
		CurrentSpecialArtData.RecoveryTime = StepData->RecoveryTime;
		CurrentSpecialArtData.MoveDistance = StepData->MoveDistance;
		CurrentSpecialArtData.MontageName = StepData->MontageName;
	}

	TObjectPtr<UAnimMontage>* FoundMontage = SpecialArtMontages.Find(CurrentSpecialArtData.MontageName);
	if (FoundMontage == nullptr || *FoundMontage == nullptr)
	{
		MC_LOG(LogMC, Warning, TEXT("No SpecialArtMontage! : %s"), *CurrentSpecialArtData.MontageName.ToString());
		return;
	}

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance == nullptr) { return; }

	UAnimMontage* MontageToPlay = *FoundMontage;
	AnimInstance->Montage_Play(MontageToPlay, 1.0f);

	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ATalPlayerCharacter::OnSpecialArtMontageEnd);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, MontageToPlay);

	// 돌진 시작
	if (CurrentSpecialArtData.MoveDistance != 0.0f && CurrentSpecialArtData.CastTime > 0.0f)
	{
		SpecialArtMoveVelocity = GetActorForwardVector() * (CurrentSpecialArtData.MoveDistance / CurrentSpecialArtData.CastTime);
		bIsSpecialArtMoving = true;

		GetWorldTimerManager().SetTimer(SpecialArtMoveTimer, this,
			&ATalPlayerCharacter::EndSpecialArtMove, CurrentSpecialArtData.CastTime, false);
	}
}

void ATalPlayerCharacter::OnSpecialArtMontageEnd(UAnimMontage* TargetMontage, bool IsProperlyEnded)
{
	if (CurrentSpecialArtData.NextStepID.IsNone() || CurrentSpecialArtData.NextStepID == FName(TEXT("")))
	{
		// 다음 스텝이 없으면 후딜 시작, 후딜 끝나면 특수기 종료
		
		GetWorldTimerManager().SetTimer(SpecialArtRecoveryTimer, this,
			&ATalPlayerCharacter::EndSpecialArtRecovery, CurrentSpecialArtData.RecoveryTime, false);
		
		return;
	}
	StartSpecialArtStep(CurrentSpecialArtData.NextStepID);
}

void ATalPlayerCharacter::CancelSpecialArt()
{
	if (!bIsUsingSpecialArt) return;

	GetWorldTimerManager().ClearTimer(SpecialArtRecoveryTimer);
	GetWorldTimerManager().ClearTimer(SpecialArtMoveTimer);

	bIsSpecialArtMoving = false;

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (TObjectPtr<UAnimMontage>* FoundMontage =
			SpecialArtMontages.Find(CurrentSpecialArtData.MontageName))
		{
			AnimInstance->Montage_Stop(0.2f, *FoundMontage);
		}
	}
	bIsUsingSpecialArt = false;
}

void ATalPlayerCharacter::EndSpecialArtRecovery()
{
	bIsUsingSpecialArt = false;
}

void ATalPlayerCharacter::EndSpecialArtMove()
{
	bIsSpecialArtMoving = false;
	SpecialArtMoveVelocity = FVector::ZeroVector;
}

void ATalPlayerCharacter::RecoverSoul(float Amount)
{
	StatComponent->RecoverSoul(Amount);
}



// 패링 / 가드 시스템 연동
void ATalPlayerCharacter::StartGuard(const FInputActionValue& Value)
{
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
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), ParrySlowMoTimeDilation);
	bParrySlowMoActive = true; // 슬로우 시작
	ParrySlowMoEndRealTime = GetWorld()->GetRealTimeSeconds() + ParrySlowMoRealDuration;
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
	if (!IsLocallyControlled() || !IsValid(CameraTarget) || !IsValid(FollowCamera)) return;

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

	const FTransform TargetTransform = LastBonfire.IsValid()
		? LastBonfire->GetActorTransform() : InitialSpawnTransform;

	SetActorLocation(TargetTransform.GetLocation());

	SetActorRotation(TargetTransform.GetRotation());

	SetActorEnableCollision(true);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);
	}
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		EnableInput(PlayerController);
	}
	
	if (LastBonfire.IsValid())
	{
		LastBonfire->TriggerRest(this);
	} else {
		StatComponent->RestoreFull();
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
	if (!SelectedTalChiTarget.IsValid())
	{
		MC_LOG(LogSY, Warning, TEXT("탈취할 사람이 없다"));
		return;
	}

	UNTTalChiTargetComponent* TargetPoint = SelectedTalChiTarget.Get();
	ACharacter* VictimCharacter = Cast<ACharacter>(TargetPoint->GetOwner());
	UAnimMontage * VictimMontage = TargetPoint->GetVictimTalchiMontage();

	if(!IsValid(VictimCharacter) || !IsValid(VictimMontage)) return;

	const float MontageLength = VictimCharacter->PlayAnimMontage(VictimMontage);

	if (MontageLength <= 0.0f) return;

	MC_LOG(LogSY, Warning, TEXT("적탈취 모션 시작"));

}

void ATalPlayerCharacter::TransferSelectedTal()
{
	if (!IsValid(TalComponent) || !SelectedTalChiTarget.IsValid()) return;

	AActor* TargetActor = SelectedTalChiTarget->GetOwner();

	if (!IsValid(TargetActor)) return;

	UNTTalComponent* TargetTalComponent = TargetActor->FindComponentByClass<UNTTalComponent>();

	if (!IsValid(TargetTalComponent)) return;

	ATalBase* TargetTal = TargetTalComponent->GetCurrentTal();

	if (!IsValid(TargetTal)) return;

	ATalBase* OldPlayerTal = TalComponent->UnequipTal();
	ATalBase* StolenTal = TargetTalComponent->UnequipTal();

	if (!IsValid(StolenTal))
	{
		if (IsValid(OldPlayerTal))
		{
			TalComponent->EquipTal(OldPlayerTal);
		}

		return;
	}

	if (TalComponent->EquipTal(StolenTal))
	{
		if (IsValid(OldPlayerTal))
		{
			OldPlayerTal->Destroy();
		}

		return;
	}

	const bool bEnemyRestored = TargetTalComponent->EquipTal(StolenTal);

	bool bPlayerRestored = true;
	if (IsValid(OldPlayerTal))
	{
		bPlayerRestored = TalComponent->EquipTal(OldPlayerTal);
	}
}

void ATalPlayerCharacter::GrabSelectedTalToHand()
{
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
}

void ATalPlayerCharacter::EquipHeldTalToFace()
{
	if (!IsValid(TalComponent) || !IsValid(HeldStolenTal)) return;

	ATalBase* TalToEquip = HeldStolenTal;
	ATalBase* OldPlayerTal = TalComponent->UnequipTal();

	if (TalComponent->EquipTal(TalToEquip))
	{
		if (IsValid(OldPlayerTal))
		{
			OldPlayerTal->Destroy();
		}

		HeldStolenTal = nullptr;

		return;
	}

	bool bOldTalRestored = true;

	if (IsValid(OldPlayerTal))
	{
		bOldTalRestored = TalComponent->EquipTal(OldPlayerTal);
	}
}