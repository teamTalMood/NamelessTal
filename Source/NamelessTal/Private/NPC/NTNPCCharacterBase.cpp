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
	return 0.0f;
}

float ANTNPCCharacterBase::GetAIAttackRange()
{
	return 10.0f;
}

float ANTNPCCharacterBase::GetAITurnSpeed()
{
	return 0.0f;
}

void ANTNPCCharacterBase::AttackByAI()
{
}

void ANTNPCCharacterBase::SetAIAttackDelegate(const FAICharacterAttackFinished& InOnAttackFinished)
{
}

