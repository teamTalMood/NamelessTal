// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/TalPlayerCharacter.h"
#include "Variant_Combat/Interfaces/CombatDamageable.h"
#include "Animation/AnimMontage.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

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

ATalPlayerCharacter::ATalPlayerCharacter()
{
	//Camera Arm Set
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 350.0f;
	CameraBoom->bUsePawnControlRotation = true;

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
}


void ATalPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDodging)
	{
		const FRotator TargetRotation = DodgeDir.Rotation();
		SetActorRotation(FMath::RInterpConstantTo(GetActorRotation(), TargetRotation, 
			DeltaTime, DodgeRotationSpeed));
	}
	else if (Controller != nullptr)
	{
		const FRotator TargetRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation,
			DeltaTime, NormalRotationInterpSpeed));
	}

	// Sprint Speed Interpolation
	const float TargetSpeed = bIsSprinting ? SprintSpeed : WalkSpeed;
	GetCharacterMovement()->MaxWalkSpeed =
		FMath::FInterpTo(GetCharacterMovement()->MaxWalkSpeed, TargetSpeed, DeltaTime, SpeedInterpSpeed);
}

void ATalPlayerCharacter::StartDodge(const FInputActionValue& Value)
{
	bUseControllerRotationYaw = false;//Disable rotation during dodge

	if (bIsDodging || RollMontage == nullptr) // Already Dodging or No RollMontage
	{
		return;
	}

	bIsDodging = true;
	bIsInvincible = true; // ApplyDamage will check this flag to ignore damage during dodge
	DodgeDir = bIsMoving ? LastMoveInputWorldDir : GetActorForwardVector();
	// not move, use forward direction
	
	PlayAnimMontage(RollMontage);

	const float MontageDuration = RollMontage->GetPlayLength();
	GetWorldTimerManager().SetTimer(DodgeTimerHandle, this,
		&ATalPlayerCharacter::EndDodge, MontageDuration, false);
}

void ATalPlayerCharacter::EndDodge()
{
	bIsDodging = false;
	bIsInvincible = false;
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
	}
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
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(-LookAxisVector.Y);
	}
}

void ATalPlayerCharacter::StartSprint(const FInputActionValue& Value)
{
	bIsSprinting = true;
}

void ATalPlayerCharacter::StopSprint(const FInputActionValue& Value)
{
	bIsSprinting = false;
}

void ATalPlayerCharacter::Attack(const FInputActionValue& Value)
{
	if (bIsDodging || AttackMontage == nullptr)
	{
		return;
	}

	PlayAnimMontage(AttackMontage);
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

	ICombatDamageable* Damageable = Cast<ICombatDamageable>(OtherActor);
	if (Damageable == nullptr)
	{
		return;
	}
	HitActors.Add(OtherActor);

	const FVector ImpactPoint = SweepResult.bBlockingHit ?
		FVector(SweepResult.ImpactPoint) : OtherActor->GetActorLocation();
	Damageable->ApplyDamage(AttackDamage, this, ImpactPoint, GetActorForwardVector() * 200.0f);
}
