// Fill out your copyright notice in the Description page of Project Settings.


#include "TalBase.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Animation/AnimMontage.h"



// Sets default values
ATalBase::ATalBase()
{
	//Tick Off -> 한번만 전달하면 됨
	PrimaryActorTick.bCanEverTick = false;

	TalRoot = CreateDefaultSubobject<USceneComponent>(TEXT("TalRoot"));
	SetRootComponent(TalRoot);

	TalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TalMesh"));
	TalMesh->SetupAttachment(TalRoot);

	//Collision Off -> 플레이어와의 충돌방지
	TalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


ETalStyle ATalBase::GetTalStyle() const
{
	return TalStyle;
}
UAnimMontage* ATalBase::GetAttackMontage() const
{
	return AttackMontage;
}