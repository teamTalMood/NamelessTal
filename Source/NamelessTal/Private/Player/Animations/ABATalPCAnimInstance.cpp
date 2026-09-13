// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/Animations/ABATalPCAnimInstance.h"
#include "Player/TalPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "KismetAnimationLibrary.h"

void UABATalPCAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	TalPlayerCharacter = Cast<ATalPlayerCharacter>(TryGetPawnOwner());
}

void UABATalPCAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (TalPlayerCharacter == nullptr)
	{
		TalPlayerCharacter = Cast<ATalPlayerCharacter>(TryGetPawnOwner());
	}

	if (TalPlayerCharacter != nullptr) {
		Speed = TalPlayerCharacter->GetVelocity().Size2D();
		Direction = UKismetAnimationLibrary::CalculateDirection(
			TalPlayerCharacter->GetVelocity(),
			TalPlayerCharacter->GetActorRotation()
		);
		// 회피중?
		bIsDodging = TalPlayerCharacter->IsDodging();
		// 가드중?
		bIsGuarding = TalPlayerCharacter->IsGuarding();
		// 낙하중?
		bIsFalling = TalPlayerCharacter->IsRealFalling();
		bIsGrounded = !TalPlayerCharacter->GetCharacterMovement()->IsFalling();

		UpdateFootIK(DeltaSeconds);
	}
}

void UABATalPCAnimInstance::UpdateFootIK(float DeltaSeconds)
{
	if (TalPlayerCharacter == nullptr) return;

	if (TalPlayerCharacter->GetCharacterMovement()->IsFalling()) {

		// 공중에선 IK 서서히 0으로
		PelvisOffset = FMath::FInterpTo(PelvisOffset, 0.0f, DeltaSeconds, IKInterpSpeed);
		SmoothedFootZ_L = FMath::FInterpTo(SmoothedFootZ_L, 0.0f, DeltaSeconds, IKInterpSpeed);
		SmoothedFootZ_R = FMath::FInterpTo(SmoothedFootZ_R, 0.0f, DeltaSeconds, IKInterpSpeed);

		FootOffset_L = SmoothedFootZ_L;
		FootOffset_R = SmoothedFootZ_R;

		FootEffector_L = MakeBoneSpaceLift(TEXT("foot_l"), FootOffset_L);
		FootEffector_R = MakeBoneSpaceLift(TEXT("foot_r"), FootOffset_R);

		FootRotation_L = FMath::RInterpTo(FootRotation_L, FRotator::ZeroRotator, DeltaSeconds, FootRotInterpSpeed);
		FootRotation_R = FMath::RInterpTo(FootRotation_R, FRotator::ZeroRotator, DeltaSeconds, FootRotInterpSpeed);

		return;
	}

	float RawL = 0.0f, RawR = 0.0f;
	FRotator RotL = FRotator::ZeroRotator, RotR = FRotator::ZeroRotator;

	TraceFoot(TEXT("foot_l"), RawL, RotL);
	TraceFoot(TEXT("foot_r"), RawR, RotR);

	// 더 낮은 발 기준으로 골반을 내리되, 비율과 상한을 적용
	const float RawPelvis = FMath::Min(0.0f, FMath::Min(RawL, RawR)) * PelvisWeight;
	const float TargetPelvis = FMath::Max(RawPelvis, -MaxPelvisDrop);

	PelvisOffset = FMath::FInterpTo(PelvisOffset, TargetPelvis, DeltaSeconds, IKInterpSpeed);

	// Raw 값을 먼저 스무딩한 뒤, 골반 보정은 보간 없이 빼야 발이 골반을 정확히 따라갑니다
	SmoothedFootZ_L = FMath::FInterpTo(SmoothedFootZ_L, RawL, DeltaSeconds, IKInterpSpeed);
	SmoothedFootZ_R = FMath::FInterpTo(SmoothedFootZ_R, RawR, DeltaSeconds, IKInterpSpeed);

	//골반이 내려간 만큼 발은 다시 올려줍니다
	FootOffset_L = SmoothedFootZ_L - PelvisOffset;
	FootOffset_R = SmoothedFootZ_R - PelvisOffset;

	// 월드 기준 들어올림 -> foot 본 로컬 공간 (BCS_BoneSpace 이펙터용)
	FootEffector_L = MakeBoneSpaceLift(TEXT("foot_l"), FootOffset_L);
	FootEffector_R = MakeBoneSpaceLift(TEXT("foot_r"), FootOffset_R);

	FootRotation_L = FMath::RInterpTo(FootRotation_L, RotL, DeltaSeconds, FootRotInterpSpeed);
	FootRotation_R = FMath::RInterpTo(FootRotation_R, RotR, DeltaSeconds, FootRotInterpSpeed);
}


bool UABATalPCAnimInstance::TraceFoot(const FName BoneName, float& OutOffset, FRotator& OutRotation) const
{
	OutOffset = 0.0f;
	OutRotation = FRotator::ZeroRotator;

	const USkeletalMeshComponent* MeshComp = TalPlayerCharacter->GetMesh();
	if (MeshComp == nullptr || GetWorld() == nullptr) return false;

	const FVector FootLocation = MeshComp->GetSocketLocation(BoneName);
	const float CapsuleBottomZ = TalPlayerCharacter->GetActorLocation().Z -
		TalPlayerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	const FVector Start(FootLocation.X, FootLocation.Y, CapsuleBottomZ + TraceUpDistance);
	const FVector End(FootLocation.X, FootLocation.Y, CapsuleBottomZ - TraceDownDistance);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(FootIK), false, TalPlayerCharacter);
	FHitResult Hit;

	// 라인 대신 구체 스윕 - 계단/자갈 모서리에서 값이 튀는 것을 막습니다
	if (!GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Visibility,
		FCollisionShape::MakeSphere(TraceSphereRadius), Params))
	{
		return false;
	}

	// 시작부터 지오메트리에 박히면 법선이 쓰레기값이라 버린다
	if (Hit.bStartPenetrating) return false;

	// 벽이나 걷지 못할 급경사는 지면으로 치지 않습니다
	const float MinNormalZ = FMath::Cos(FMath::DegreesToRadians(MaxFootSlopeAngle));
	if (Hit.ImpactNormal.Z < MinNormalZ) return false;

	//지면이 캡슐 바닥보다 높으면 양수(발을 올리기), 아니면 음수(발 내리기)
	OutOffset = (Hit.ImpactPoint.Z - CapsuleBottomZ) + FootRadius;

	// 한 발이 캡슐 바닥에서 지나치게 벗어나 다리가 찢어지는 것을 막습니다
	OutOffset = FMath::Clamp(OutOffset, -MaxFootLift, MaxFootLift);

	// 월드 법선 -> 컴포넌트 공간 additive 회전
	// 캐릭터가 어느 방향을 봐도 발바닥이 항상 경사면에 붙습니다
	const FQuat WorldDelta = FQuat::FindBetweenNormals(FVector::UpVector, Hit.ImpactNormal);
	const FQuat CompQuat = MeshComp->GetComponentQuat();

	OutRotation = (CompQuat.Inverse() * WorldDelta * CompQuat).Rotator();

	return true;

}

FVector UABATalPCAnimInstance::MakeBoneSpaceLift(const FName BoneName, float WorldLift) const
{
	const USkeletalMeshComponent* MeshComp = TalPlayerCharacter ? TalPlayerCharacter->GetMesh() : nullptr;
	if (MeshComp == nullptr) return FVector::ZeroVector;

	// 본 축이 어떻게 누워있든, 결과는 항상 월드 +Z 방향 이동
	const FQuat BoneQuat = MeshComp->GetSocketQuaternion(BoneName);
	return BoneQuat.UnrotateVector(FVector(0.0f, 0.0f, WorldLift));
}
