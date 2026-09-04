// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NTNPCCharacterBase.h"
#include "MCLOG.h"

//Components
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"

//Engine
#include "Kismet/GameplayStatics.h"

//AI
#include "BehaviorTree/BehaviorTreeComponent.h"

//NT
#include "NPC/NTAIController.h"
#include "Animation/AnimMontage.h"

//Interface
#include "Variant_Combat/Interfaces/CombatDamageable.h"

// Sets default values
ANTNPCCharacterBase::ANTNPCCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), FRotator(0, -90, 0));

	// AIController
	AIControllerClass = ANTAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 무기 Mesh Set
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(GetMesh(), TEXT("WeaponSocket_R"));
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("WeaponCollision"));
	WeaponCollision->SetupAttachment(WeaponMesh);
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	WeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &ANTNPCCharacterBase::OnWeaponOverlap);
}

// Called when the game starts or when spawned
void ANTNPCCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	AAIController* AICon = GetController<AAIController>();
	BTComponent = Cast<UBehaviorTreeComponent>(AICon->GetBrainComponent());
}

float ANTNPCCharacterBase::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	MC_LOG(LogMC, Log, TEXT("Begin"));

	if (BTComponent)
	{
		MC_LOG(LogMC, Log, TEXT("BTComponent PauseLogic"));
		BTComponent->PauseLogic(TEXT("Reason"));
		GetMesh()->bPauseAnims = true;

		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([&]
			{
				MC_LOG(LogMC, Log, TEXT("BTComponent ResumeLogic"));
				BTComponent->ResumeLogic(TEXT("Reason"));
				GetMesh()->bPauseAnims = false;
			}
		), 3, false);
	}

	return 0.0f;
}

// Called every frame
void ANTNPCCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ANTNPCCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float ANTNPCCharacterBase::GetAIPatrolRadius()
{
	return 0.0f;
}

float ANTNPCCharacterBase::GetAIDetectRange()
{
	return 400.0f;
}

float ANTNPCCharacterBase::GetAIAttackRange()
{
	return 100.0f;
}

float ANTNPCCharacterBase::GetAITurnSpeed()
{
	return 0.0f;
}

void ANTNPCCharacterBase::AttackByAI()
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	ProcessComboCommand();
	//AttackHitCheck();
}

void ANTNPCCharacterBase::SetAIAttackDelegate(const FAICharacterAttackFinished& InOnAttackFinished)
{
	OnAICharacterAttackFinished = InOnAttackFinished;
}

void ANTNPCCharacterBase::EnableWeaponCollision()
{
	if (WeaponCollision == nullptr)
	{
		MC_LOG(LogMC, Error, TEXT("No WeaponCollision!"));
		return;
	}
	HitActors.Empty(); // 새 공격이라 이전 기록 비우기
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}

void ANTNPCCharacterBase::DisableWeaponCollision()
{
	if (WeaponCollision == nullptr)
	{
		MC_LOG(LogMC, Error, TEXT("No WeaponCollision!"));
		return;
	}
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OnAICharacterAttackFinished.ExecuteIfBound();
}

void ANTNPCCharacterBase::AttackHitCheck()
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	FHitResult OutHitResult;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), false, this);

	const float AttackRange = 40.0f;
	const float AttackRadius = 50.0f;
	const FVector Start = GetActorLocation() + GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector End = Start + GetActorForwardVector() * AttackRange;

	bool HitDetected = GetWorld()->SweepSingleByChannel(OutHitResult, Start, End, FQuat::Identity, ECC_WorldDynamic, FCollisionShape::MakeSphere(AttackRadius), Params);
	if (HitDetected)
	{
		AActor* HitActor = OutHitResult.GetActor();
		if (HitActor)
		{
			UGameplayStatics::ApplyDamage(HitActor, AttackDamage, GetController(), this, UDamageType::StaticClass());
		}
	}
	MC_LOG(LogMC, Log, TEXT("HitDetected : %d"), HitDetected);

#if ENABLE_DRAW_DEBUG

	FVector CapsuleOrigin = Start + (End - Start) * 0.5f;
	float CapsuleHalfHeight = AttackRange * 0.5f;
	FColor DrawColor = HitDetected ? FColor::Green : FColor::Red;

	DrawDebugCapsule(GetWorld(), CapsuleOrigin, CapsuleHalfHeight, AttackRadius, FRotationMatrix::MakeFromZ(GetActorForwardVector()).ToQuat(), DrawColor, false, 5.0f);

#endif

	OnAICharacterAttackFinished.ExecuteIfBound();

	MC_LOG(LogMC, Log, TEXT("End"));
}

void ANTNPCCharacterBase::ProcessComboCommand()
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	AnimInstance->Montage_Play(ComboAttackMontage);
}

void ANTNPCCharacterBase::OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 떄려지는 객체 확인 로그
	if (OtherActor == nullptr || OtherActor == this || HitActors.Contains(OtherActor))
	{
		return;
	}
	MC_LOG(LogMC, Log, TEXT("Weapon overlapped with: %s"), *GetNameSafe(OtherActor));

	//ICombatDamageable* Damageable = Cast<ICombatDamageable>(OtherActor);
	//if (Damageable == nullptr)
	//{
	//	return;
	//}
	HitActors.Add(OtherActor);

	//const FVector ImpactPoint = SweepResult.bBlockingHit ? FVector(SweepResult.ImpactPoint) : OtherActor->GetActorLocation();
	//Damageable->ApplyDamage(AttackDamage, this, ImpactPoint, GetActorForwardVector() * 200.0f);
	UGameplayStatics::ApplyDamage(OtherActor, AttackDamage, GetController(), this, UDamageType::StaticClass());
}
