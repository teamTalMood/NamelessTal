// Fill out your copyright notice in the Description page of Project Settings.
#include "Player/TalWeapon.h"

#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"

#include "Variant_Combat/Interfaces/CombatDamageable.h"

ATalWeapon::ATalWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(SceneRoot);

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//공격 판정용 Collision
	WeaponCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("WeaponCollision"));
	WeaponCollision->SetupAttachment(WeaponMesh);

	//기본으론 Collision Off
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);//모든 충돌 무시

	WeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &ATalWeapon::OnWeaponOverlap);
}

void ATalWeapon::EnableWeaponCollision()//Collision 활성화
{
	if (WeaponCollision == nullptr) return;

	//새로운 공격이므로 이전에 맞춘 대상 초기화
	HitActors.Empty();

	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
}

void ATalWeapon::DisableWeaponCollision()
{
	if (WeaponCollision == nullptr) return;

	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


//오버랩
void ATalWeapon::OnWeaponOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bRfromSweep,
	const FHitResult& SweepResult)
{
	if (OtherActor == nullptr ||
		OtherActor == this ||
		OtherActor == GetOwner() ||
		HitActors.Contains(OtherActor))
	{
		return;
	}

	ICombatDamageable* Damageable = Cast<ICombatDamageable>(OtherActor);
	if (Damageable == nullptr) return;

	HitActors.Add(OtherActor);

	const FVector ImpactPoint = 
		SweepResult.bBlockingHit
		? FVector(SweepResult.ImpactPoint)
		: OtherActor->GetActorLocation();

	AActor* DamageCauser = GetOwner();//캐릭터 가져오기

	if (DamageCauser == nullptr)
	{
		DamageCauser = this;
	}

	Damageable->ApplyDamage(
		AttackDamage,
		DamageCauser,
		ImpactPoint,
		GetActorForwardVector() * 200.0f
	);
}