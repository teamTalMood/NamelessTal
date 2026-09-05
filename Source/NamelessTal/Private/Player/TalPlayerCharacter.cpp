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
	FollowCamera -> SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// For Roll Dodge Input
	PrimaryActorTick.bCanEverTick = true;

	// 무기 Mesh Set
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(GetMesh(), TEXT("WeaponSocket_R"));
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("WeaponCollision"));
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
}


void ATalPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//카메라 거리보간
	const float SafeCameraDistance = FMath::Max(0.0f, CameraDistance);
	const float SafeInterpSpeed = FMath::Max(0.0f, CameraDistanceInterpSpeed);

	CameraBoom->TargetArmLength = FMath::FInterpTo(
		CameraBoom->TargetArmLength,
		SafeCameraDistance,
		DeltaTime,
		SafeInterpSpeed
	);
	//카메라 Pitch제한
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (APlayerCameraManager* CameraManager = PlayerController->PlayerCameraManager)
		{
			const float SafePitchMin = FMath::Clamp(FMath::Min(MinCameraPitch, MaxCameraPitch), -89.0f, 89.0f);
			const float SafePitchMax = FMath::Clamp(FMath::Max(MinCameraPitch, MaxCameraPitch), -89.0f, 89.0f);

			CameraManager->ViewPitchMin = SafePitchMin;
			CameraManager ->ViewPitchMax = SafePitchMax;
		}
	}
	if (bIsLockOnActive)
	{
		UpdateLockOn(DeltaTime);
	}

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
	const float TargetSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeed =
		FMath::FInterpTo(GetCharacterMovement()->MaxWalkSpeed, TargetSpeed, DeltaTime, SpeedInterpSpeed);
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
	bUseControllerRotationYaw = false;//Disable rotation during dodge

	if (bIsDodging) // Already Dodging or No RollMontage
	{
		return;
	}

	StatComponent->DrainSoul(15.0f);//회피시 넋 15 소모

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

// Player Move
void ATalPlayerCharacter::Move(const FInputActionValue& Value)
{
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
		if(!InputDir.IsNearlyZero())
		{
			LastMoveInputWorldDir = InputDir.GetSafeNormal();
		}

		if (bIsDodging)
		{ // AddmovementInput is ignored during dodge
			return;
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

	if (GuardComponent->IsGuarding())
	{
		if (GuardComponent->TryParry())
		{
			return 0.0f;
		}

		// 패링 X 가드 O 성공 시 넋 30(임시) 줄어듦
		StatComponent->DrainSoul(30.0f);

		UE_LOG(LogTemp, Warning, TEXT("가드성공"));
		const float ReducedDamage = Damage * 0.3f; // 임시 값
		StatComponent->ApplyDamage(ReducedDamage);
		return ReducedDamage;
	}

	StatComponent->ApplyDamage(Damage);

	// 실제로 받은 데미지 적용되었는지 호출자(공격자)에게 알려주는 용도
	return Damage;
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
				Subsystem -> AddMappingContext(DefaultMappingContext, 0);
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
	if (!StatComponent->HasSoul()) return;

	bIsSprinting = true;

	StatComponent->SetSoulRegenBlocked(true);
}

void ATalPlayerCharacter::StopSprint(const FInputActionValue& Value)
{
	bIsSprinting = false;

	StatComponent->SetSoulRegenBlocked(false);
}

void ATalPlayerCharacter::Attack(const FInputActionValue& Value)
{
	if (bIsDodging || AttackMontage == nullptr)
	{
		return;
	}
	StatComponent->DrainSoul(10.0f);//공격시 넋 10 소모

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance == nullptr) return;

	//몽타주 재생이 아니면 1타 시작
	if (!AnimInstance->Montage_IsPlaying(AttackMontage))
	{
		ComboIndex = 1;
		bCanCombo = false;

		PlayAnimMontage(AttackMontage, 1.0f, TEXT("Attack1"));

		return;
	}

	//공격무시
	if (!bCanCombo) return;

	//2타 연결
	if (ComboIndex == 1)
	{
		//AnimInstance->Montage_SetNextSection(
		//	TEXT("Attack1"),
		//	TEXT("Attack2"),
		//	AttackMontage
		//);

		ComboIndex = 2;
		bCanCombo = false;

		//바로 애니메이션 연결하려면 JumpToSection
		AnimInstance->Montage_JumpToSection(
			TEXT("Attack2"),
			AttackMontage
		);
	}
	//3타 연결
	else if (ComboIndex == 2)
	{
		//AnimInstance->Montage_SetNextSection(
		//	TEXT("Attack2"),
		//	TEXT("Attack3"),
		//	AttackMontage
		//);

		ComboIndex = 3;
		bCanCombo = false;

		AnimInstance->Montage_JumpToSection(
			TEXT("Attack3"),
			AttackMontage
		);
	}
}

void ATalPlayerCharacter::EnableWeaponCollision()
{
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
	if (WeaponCollision == nullptr)
	{
		return;
	}
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATalPlayerCharacter::OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 떄려지는 객체 확인 로그
	UE_LOG(LogTemp, Warning, TEXT("Weapon overlapped with: %s"), *GetNameSafe(OtherActor));
	if (OtherActor == nullptr || OtherActor == this || HitActors.Contains(OtherActor))
	{
		return;
	}

	//ICombatDamageable* Damageable = Cast<ICombatDamageable>(OtherActor);
	//if (Damageable == nullptr)
	//{
	//	return;
	//}
	HitActors.Add(OtherActor);

	//const FVector ImpactPoint = SweepResult.bBlockingHit ? 	FVector(SweepResult.ImpactPoint) : OtherActor->GetActorLocation();
	//Damageable->ApplyDamage(AttackDamage, this, ImpactPoint, GetActorForwardVector() * 200.0f);
	UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, GetController(), this, UDamageType::StaticClass());
}

void ATalPlayerCharacter::EquipTal(ATalBase* NewTal)
{
	if (!NewTal) return; //널처리

	//탈바꿈
	CurrentTal = NewTal;
	CurrentTal->SetOwner(this);
	CurrentTal->AttachToComponent(
		GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		TEXT("TalSocket")
	);
}

ATalBase* ATalPlayerCharacter::GetCurrentTal() const
{
	if (!CurrentTal) return NULL; //널처리

	return CurrentTal;
}

void ATalPlayerCharacter::BeginPlay()//캐릭터 생성시
{
	Super::BeginPlay();

	//기본 탈 장착
	if (DefaultTalClass)
	{
		ATalBase* NewTal = GetWorld()->SpawnActor<ATalBase>(DefaultTalClass);

		EquipTal(NewTal);
	}

	////기본 무기 장착
	//if (DefaultWeaponClass)
	//{
	//	FActorSpawnParameters SpawnParams;
	//	SpawnParams.Owner = this;
	//	SpawnParams.Instigator = this;
	//	SpawnParams.SpawnCollisionHandlingOverride =
	//		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	//	ATalWeapon* NewWeapon =
	//		GetWorld()->SpawnActor<ATalWeapon>(
	//			DefaultWeaponClass,
	//			FTransform::Identity,
	//			SpawnParams
	//		);

	//	EquipWeapon(NewWeapon);
	//}
}

ATalBase* ATalPlayerCharacter::UnequipTal()
{
	if (!CurrentTal) return nullptr;

	ATalBase* OldTal = CurrentTal;
	CurrentTal = nullptr;
	OldTal->DetachFromActor(
		FDetachmentTransformRules::KeepWorldTransform
	);

	OldTal->SetOwner(nullptr);

	return OldTal;
}

void ATalPlayerCharacter::TakeTalFrom(ATalPlayerCharacter* TargetCharacter)
{
	if (!TargetCharacter || TargetCharacter == this) return;
	if (!TargetCharacter->GetCurrentTal())			 return;

	ATalBase* TakenTal = TargetCharacter->UnequipTal();
	if (!TakenTal) return;

	ATalBase* MyOldTal = UnequipTal();
	if (MyOldTal) MyOldTal->K2_DestroyActor();

	EquipTal(TakenTal);
}

void ATalPlayerCharacter::Interact()
{
	if (!FollowCamera) return;

	//카메라 위치
	const FVector Start = FollowCamera->GetComponentLocation();

	const FVector End = Start + FollowCamera->GetForwardVector() * 1000.0f;

	FHitResult HitResult;

	FCollisionQueryParams QueryParams;

	//자기 자신은 검사하지 않음
	QueryParams.AddIgnoredActor(this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECC_Visibility,
		QueryParams
	);

	if (!bHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("아무것도 못 맞춤..")); return;
	}

	AActor* HitActor = HitResult.GetActor();
	UE_LOG(
		LogTemp,
		Warning,
		TEXT("Hit Actor = %s / Class = %s"),
		*GetNameSafe(HitActor),
		*GetNameSafe(HitActor ? HitActor->GetClass() : nullptr)
	);

	//확인용 선
	DrawDebugLine(
		GetWorld(),
		Start,
		End,
		bHit ? FColor::Green : FColor::Red,
		false,
		2.0f,
		0,
		2.0f
	);
	ATalPlayerCharacter* TargetCharacter = Cast<ATalPlayerCharacter>(HitResult.GetActor());
	TakeTalFrom(TargetCharacter);
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
	bCanCombo = true;
}
void ATalPlayerCharacter::CloseComboWindow()
{
	bCanCombo = false;
}



// 패링 / 가드 시스템 연동 
void ATalPlayerCharacter::StartGuard(const FInputActionValue& Value)
{
	GuardComponent->StartGuard();
	StatComponent->SetSoulRegenBlocked(true);
}

void ATalPlayerCharacter::StopGuard(const FInputActionValue& Value)
{
	GuardComponent->StopGuard();
	StatComponent->SetSoulRegenBlocked(false);
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