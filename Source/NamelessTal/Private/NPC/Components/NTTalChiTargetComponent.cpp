// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/Components/NTTalChiTargetComponent.h"
#include "GameFramework/Actor.h"

// Sets default values for this component's properties
UNTTalChiTargetComponent::UNTTalChiTargetComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

FTransform UNTTalChiTargetComponent::GetTalchiTransform() const
{
	return GetComponentTransform();
}

bool UNTTalChiTargetComponent::IsTalchiAvailable() const
{
	const AActor* OwnerActor = GetOwner();

	return bTalchiEnabled
		&& IsActive()
		&& IsValid(OwnerActor)
		&& !OwnerActor->IsActorBeingDestroyed()
		&& IsValid(PlayerTalchiMontage)
		&& IsValid(VictimTalchiMontage);
}

void UNTTalChiTargetComponent::SetTalchiEnabled(bool bEnabled)
{
	bTalchiEnabled = bEnabled;
}

UAnimMontage* UNTTalChiTargetComponent::GetPlayerTalchiMontage() const
{
	return PlayerTalchiMontage;
}

UAnimMontage* UNTTalChiTargetComponent::GetVictimTalchiMontage() const
{
	return VictimTalchiMontage;
}

bool UNTTalChiTargetComponent::IsInsideTalchiSector(//🐘
    const AActor* PlayerActor) const
{
    const AActor* EnemyActor = GetOwner();

    if (!IsValid(EnemyActor) || !IsValid(PlayerActor)
        || EnemyActor == PlayerActor)
    {
        return false;
    }

    // 적에서 플레이어로 향하는 벡터입니다.
    FVector ToPlayer =
        PlayerActor->GetActorLocation() - EnemyActor->GetActorLocation();

    // 다른 층에 있는 플레이어는 제외합니다.
    const float MaxHeight =
        FMath::Max(0.0f, TalchiMaxHeightDifference);

    if (FMath::Abs(ToPlayer.Z) > MaxHeight)
    {
        return false;
    }

    // 부채꼴은 바닥 평면을 기준으로 계산합니다.
    ToPlayer.Z = 0.0f;

    const float Radius = FMath::Max(0.0f, TalchiRadius);

    if (ToPlayer.SizeSquared() > FMath::Square(Radius))
    {
        return false;
    }

    // 위치가 완전히 겹치면 방향을 정의할 수 없으므로 제외합니다.
    if (!ToPlayer.Normalize())
    {
        return false;
    }

    FVector EnemyForward = EnemyActor->GetActorForwardVector();
    EnemyForward.Z = 0.0f;

    if (!EnemyForward.Normalize())
    {
        return false;
    }

    const float HalfAngleDegrees =
        FMath::Clamp(TalchiFullAngle, 0.0f, 360.0f) * 0.5f;

    const float MinimumDot =
        FMath::Cos(FMath::DegreesToRadians(HalfAngleDegrees));

    const float DirectionDot =
        FVector::DotProduct(EnemyForward, ToPlayer);

    return DirectionDot >= MinimumDot;
}