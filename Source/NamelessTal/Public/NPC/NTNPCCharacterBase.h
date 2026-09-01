// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NPC/Interface/NTNPCCharacterBaseInterface.h"
#include "NTNPCCharacterBase.generated.h"

UCLASS()
class NAMELESSTAL_API ANTNPCCharacterBase : public ACharacter, public INTNPCCharacterBaseInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANTNPCCharacterBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

// INTNPCCharacterBaseInterface override
public:
	virtual float GetAIPatrolRadius() override;
	virtual float GetAIDetectRange() override;
	virtual float GetAIAttackRange() override;
	virtual float GetAITurnSpeed() override;
	virtual void AttackByAI() override;
	virtual void SetAIAttackDelegate(const FAICharacterAttackFinished& InOnAttackFinished) override;

// Attack
private:
	void AttackHitCheck();

private:
	FAICharacterAttackFinished OnAICharacterAttackFinished;

};
