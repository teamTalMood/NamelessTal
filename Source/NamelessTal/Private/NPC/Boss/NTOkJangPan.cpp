// Fill out your copyright notice in the Description page of Project Settings.

#include "NPC/Boss/NTOkJangPan.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "NiagaraComponent.h"
#include "Player/Interface/NTPlayerApplyDamageInterface.h"
#include "MCLOG.h"

ANTOkJangPan::ANTOkJangPan()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(RootComponent);
	CollisionSphere->SetSphereRadius(Radius);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WarningEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WarningEffect"));
	WarningEffect->SetupAttachment(RootComponent);
	WarningEffect->SetAutoActivate(false);

	ExplosionEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ExplosionEffect"));
	ExplosionEffect->SetupAttachment(RootComponent);
	ExplosionEffect->SetAutoActivate(false);
}

void ANTOkJangPan::BeginPlay()
{
	Super::BeginPlay();

	CollisionSphere->SetSphereRadius(Radius);
}

void ANTOkJangPan::StartAOE()
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	WarningEffect->Activate(true);

	GetWorld()->GetTimerManager().SetTimer(ExplodeTimerHandle, this, &ANTOkJangPan::Explode, WarningDuration, false);
}

void ANTOkJangPan::Explode()
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	WarningEffect->Deactivate();

	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	ExplosionEffect->Activate(true);

	DealDamageToPlayerInRange();

	GetWorld()->GetTimerManager().SetTimer(DestroyTimerHandle, this, &ANTOkJangPan::DestroyAOE, ExplosionEffectDuration, false);
}

void ANTOkJangPan::DestroyAOE()
{
	Destroy();
}

void ANTOkJangPan::DealDamageToPlayerInRange()
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!IsPlayerInRange(PlayerPawn))
	{
		return;
	}

	INTPlayerApplyDamageInterface* PlayerCharacter = Cast<INTPlayerApplyDamageInterface>(PlayerPawn);
	if (!PlayerCharacter)
	{
		return;
	}

	FPlayerApplyDamageData DamageData;
	DamageData.HPDamage = FMath::RoundToInt(Damage);

	PlayerCharacter->ApplyDamage(this, DamageData);

	MC_LOG(LogMC, Log, TEXT("OkJangPan dealt %d damage to %s"), DamageData.HPDamage, *GetNameSafe(PlayerPawn));
}

bool ANTOkJangPan::IsPlayerInRange(AActor* Player) const
{
	if (!Player)
	{
		return false;
	}

	return FVector::Dist(GetActorLocation(), Player->GetActorLocation()) <= Radius;
}
