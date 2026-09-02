// Fill out your copyright notice in the Description page of Project Settings.
#include "Player/TalWeapon.h"

#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"

#include "Variant_Combat/Interfaces/CombatDamageable.h"

ATalWeapon::ATalWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	//메시를 Root로 사용
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//공격 판정용 Collision
	WeaponCollision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("WeaponCollision"));
	WeaponCollision->SetupAttachment(WeaponMesh);

	//기본으론 Collision Off
	WeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);//모든 충돌 무시

	WeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &ATalWeapon::OnWeaponOverlap);
}

void ATalWeapon::EnableWeaponCollision()
{

}

void ATalWeapon::DisableWeaponCollision()
{

}

void ATalWeapon::OnWeaponOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bRfromSweep,
	const FHitResult& SweepResult)
{

}