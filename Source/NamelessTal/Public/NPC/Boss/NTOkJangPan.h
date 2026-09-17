// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TimerHandle.h"
#include "NTOkJangPan.generated.h"

class USphereComponent;
class UNiagaraComponent;

UCLASS()
class NAMELESSTAL_API ANTOkJangPan : public AActor
{
	GENERATED_BODY()

public:
	ANTOkJangPan();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable, Category = "OkJangPan")
	void StartAOE();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraComponent> WarningEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX")
	TObjectPtr<UNiagaraComponent> ExplosionEffect;

	UPROPERTY(EditAnywhere, Category = "OkJangPan Settings")
	float Radius = 200.0f;

	UPROPERTY(EditAnywhere, Category = "OkJangPan Settings")
	float WarningDuration = 3.0f;

	UPROPERTY(EditAnywhere, Category = "OkJangPan Settings")
	float Damage = 15.0f;

	UPROPERTY(EditAnywhere, Category = "OkJangPan Settings")
	float ExplosionEffectDuration = 1.5f;

	FTimerHandle ExplodeTimerHandle;
	FTimerHandle DestroyTimerHandle;

	void Explode();
	void DestroyAOE();
	void DealDamageToPlayerInRange();
	bool IsPlayerInRange(AActor* Player) const;
};
