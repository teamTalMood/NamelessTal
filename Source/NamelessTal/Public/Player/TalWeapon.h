// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TalWeapon.generated.h"

class UStaticMshCOmponent;
class UCapsuleComponent;
class UPrimitiveComponent;
class USceneComponent;

UCLASS()
class NAMELESSTAL_API ATalWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATalWeapon();

public:
	//공격 판정 시작
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void EnableWeaponCollision();

	//공격 판정 끝
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void DisableWeaponCollision();

protected:
	//루트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	USceneComponent* SceneRoot;
	//무기 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	UStaticMeshComponent* WeaponMesh;
	//무기 Collision
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	UCapsuleComponent* WeaponCollision;
	//무기 공격력(을 가져도 되나?)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float AttackDamage = 10.0f;
	//공격당한 Actors
	TArray<AActor*> HitActors;

protected:
	UFUNCTION()
	void OnWeaponOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

};
