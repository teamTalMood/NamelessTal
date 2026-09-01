// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NTNPCCharacterBase.h"
#include "MCLOG.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "NPC/NTAIController.h"

// Sets default values
ANTNPCCharacterBase::ANTNPCCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetRelativeLocationAndRotation(FVector(0, 0, -GetCapsuleComponent()->GetScaledCapsuleHalfHeight()), FRotator(0, -90, 0));

	// AIController
	AIControllerClass = ANTAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Called when the game starts or when spawned
void ANTNPCCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
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
	MC_LOG(LogMC, Log, TEXT("Begin"));

	AttackHitCheck();
}

void ANTNPCCharacterBase::SetAIAttackDelegate(const FAICharacterAttackFinished& InOnAttackFinished)
{
	OnAICharacterAttackFinished = InOnAttackFinished;
}

void ANTNPCCharacterBase::AttackHitCheck()
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	FHitResult OutHitResult;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), false, this);

	const float AttackRange = 40.0f;
	const float AttackRadius = 50.0f;
	const float AttackDamage = 30.0f;
	const FVector Start = GetActorLocation() + GetActorForwardVector() * GetCapsuleComponent()->GetScaledCapsuleRadius();
	const FVector End = Start + GetActorForwardVector() * AttackRange;

	bool HitDetected = GetWorld()->SweepSingleByChannel(OutHitResult, Start, End, FQuat::Identity, ECC_WorldDynamic, FCollisionShape::MakeSphere(AttackRadius), Params);
	if (HitDetected)
	{

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

