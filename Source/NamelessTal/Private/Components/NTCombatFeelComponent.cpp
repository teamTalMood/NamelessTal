// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/NTCombatFeelComponent.h"
#include "Kismet/GameplayStatics.h"

#include "Engine/World.h"

#include "Camera/CameraShakeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"

#include "Player/TalPlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"



// Sets default values for this component's properties
UNTCombatFeelComponent::UNTCombatFeelComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	// 히트스톱 걸릴 시 걸린 동안 틱 돌기, 평소 비용 = 0
	PrimaryComponentTick.bStartWithTickEnabled = false;
}


// Called when the game starts
void UNTCombatFeelComponent::BeginPlay()
{
	Super::BeginPlay();
}

float UNTCombatFeelComponent::GetDurationFor(ENTHitStopKind Kind) const
{
	switch (Kind)
	{
	case ENTHitStopKind::Normal:
		return HitStopDuration_NormalAttack;
	case ENTHitStopKind::SpecialArt:
		return HitStopDuration_SpecialAttack;
	case ENTHitStopKind::Guard:
		return HitStopDuration_Guard;
	case ENTHitStopKind::Kill:
		return HitStopDuration_Kill;
	default : 
		return 0.0f;
	}
}

void UNTCombatFeelComponent::ApplyDilation(float Value)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	//시간 배율 만지는곳은 여기뿐
	UGameplayStatics::SetGlobalTimeDilation(World, Value);
}


TSubclassOf<UCameraShakeBase> UNTCombatFeelComponent::GetShakeFor(ENTHitStopKind Kind) const
{
	switch (Kind)
	{
	case ENTHitStopKind::Normal: 
		return Shake_NormalAttack;
	case ENTHitStopKind::SpecialArt: 
		return Shake_SpecialAttack;
	case ENTHitStopKind::Guard: 
		return Shake_Guard;
	case ENTHitStopKind::Kill:
		return Shake_Kill;
	default : 
		return nullptr;
	}
}

void UNTCombatFeelComponent::PlayShake(TSubclassOf<UCameraShakeBase> ShakeClass)
{
	if (!ShakeClass || CameraShakeScale <= 0.0f)
	{
		return;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn) {
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
	{
		PC->ClientStartCameraShake(ShakeClass, CameraShakeScale);
	}
}

void UNTCombatFeelComponent::StartParryCamera()
{
	ATalPlayerCharacter* Player =
		Cast<ATalPlayerCharacter>(GetOwner());

	if (!IsValid(Player) || !GetWorld()) return;

	APlayerController* PC =
		Cast<APlayerController>(Player->GetController());

	USpringArmComponent* CameraBoom = Player->GetCameraBoom();

	if (!IsValid(PC) || !PC->IsLocalController()
		|| !IsValid(CameraBoom))
	{
		return;
	}

	// 진행 중인 연출의 원래 카메라 설정을 덮어쓰지 않기
	if (ParryCameraState != ENTParryCameraState::None)
	{
		return;
	}

	// 복귀할 거리와 위치 저장
	ParryCameraOriginalArmLength = CameraBoom->TargetArmLength;
	ParryCameraOriginalSocketOffset = CameraBoom->SocketOffset;
	
	if (UCameraComponent* Camera = Player->GetFollowCamera())
	{
		ParryCameraOriginalRelativeRotation =
			Camera->GetRelativeRotation();
	}
	ParryCameraStartRotation = PC->GetControlRotation();

	float DesiredYaw = Player->GetActorRotation().Yaw;

	if (AActor* Target = ParryCameraTarget.Get())
	{
		const FVector ToTarget =
			(Target->GetActorLocation() - Player->GetActorLocation()).GetSafeNormal2D();

		if (!ToTarget.IsNearlyZero())
		{
			DesiredYaw = ToTarget.Rotation().Yaw;
		}
	}

	ParryCameraTargetRotation = FRotator(ParryCameraPitch, DesiredYaw, 0.0f);

	ParryCameraStartRealTime = GetWorld()->GetRealTimeSeconds();
	ParryCameraState = ENTParryCameraState::Entering;
	bParryCameraMoving = true;

	SetComponentTickEnabled(true);
}

void UNTCombatFeelComponent::SetParryCameraTarget(AActor* InTarget)
{
	ParryCameraTarget = InTarget;
}

void UNTCombatFeelComponent::SetParryCameraCounterEndTime(
	double EndRealTime)
{
	ParryCameraCounterEndRealTime = EndRealTime;
}

void UNTCombatFeelComponent::StartParryCameraSkillReturn()
{
	if (ParryCameraState == ENTParryCameraState::None) return;

	// 자동 빠른 복귀 예약 취소
	ParryCameraCounterEndRealTime = 0.0;

	// 현재 위치에서 천천히 복귀
	BeginParryCameraReturn(ParryCameraSkillReturnDuration);
}

void UNTCombatFeelComponent::UpdateParryCamera()
{
	if (ParryCameraState == ENTParryCameraState::None) return;

	ATalPlayerCharacter* Player =
		Cast<ATalPlayerCharacter>(GetOwner());

	APlayerController* PC = IsValid(Player)
		? Cast<APlayerController>(Player->GetController())
		: nullptr;

	USpringArmComponent* CameraBoom = IsValid(Player)
		? Player->GetCameraBoom()
		: nullptr;

	if (!IsValid(PC) || !IsValid(CameraBoom) || !GetWorld())
	{
		ParryCameraState = ENTParryCameraState::None;
		bParryCameraMoving = false;
		return;
	}

	const double Now = GetWorld()->GetRealTimeSeconds();

	
	// 반격 종료 0.1초 전부터 빠른 복귀
	if (ParryCameraState != ENTParryCameraState::Returning
		&& ParryCameraCounterEndRealTime > 0.0
		&& Now >= ParryCameraCounterEndRealTime
		- ParryCameraFastReturnDuration)
	{
		// 프레임이 늦어졌다면 남은 시간에 맞춰 복귀
		const float RemainingTime = static_cast<float>(
			ParryCameraCounterEndRealTime - Now);

		BeginParryCameraReturn(
			FMath::Max(RemainingTime, 0.01f));
	}

	if (ParryCameraState == ENTParryCameraState::Entering)
	{
		const float Alpha = FMath::Clamp(
			static_cast<float>(Now - ParryCameraStartRealTime)
			/ FMath::Max(ParryCameraMoveDuration, 0.01f),
			0.0f, 1.0f);

		CameraBoom->TargetArmLength = FMath::Lerp(
			ParryCameraOriginalArmLength,
			ParryCameraCloseArmLength,
			Alpha);

		CameraBoom->SocketOffset = FMath::Lerp(
			ParryCameraOriginalSocketOffset,
			ParryCameraCloseSocketOffset,
			Alpha);

		const FQuat Rotation = FQuat::Slerp(
			ParryCameraStartRotation.Quaternion(),
			ParryCameraTargetRotation.Quaternion(),
			Alpha);

		PC->SetControlRotation(Rotation.Rotator());

		if (Alpha >= 1.0f)
		{
			ParryCameraState = ENTParryCameraState::Holding;
		}
	}
	else if (ParryCameraState == ENTParryCameraState::Holding)
	{
		CameraBoom->TargetArmLength = ParryCameraCloseArmLength;
		CameraBoom->SocketOffset = ParryCameraCloseSocketOffset;

		// 플레이어가 돌아도 등 뒤 구도 유지
		PC->SetControlRotation(ParryCameraTargetRotation);
	}
	else if (ParryCameraState == ENTParryCameraState::Returning)
	{
		const float Alpha = FMath::Clamp(
			static_cast<float>(Now - ParryCameraReturnStartRealTime)
			/ ParryCameraCurrentReturnDuration,
			0.0f, 1.0f);

		CameraBoom->TargetArmLength = FMath::Lerp(
			ParryCameraReturnStartArmLength,
			ParryCameraOriginalArmLength,
			Alpha);

		CameraBoom->SocketOffset = FMath::Lerp(
			ParryCameraReturnStartSocketOffset,
			ParryCameraOriginalSocketOffset,
			Alpha);

		if (Alpha >= 1.0f)
		{
			ParryCameraState = ENTParryCameraState::None;
			bParryCameraMoving = false;
			ParryCameraCounterEndRealTime = 0.0;
		}
	}
	
	// 스프링 암의 위치와 별개로 카메라 자체의 시선을 조정
	UCameraComponent* Camera = Player->GetFollowCamera();
	if (!IsValid(Camera)) return;

	// 복귀 완료: 원래 회전으로 복원
	if (ParryCameraState == ENTParryCameraState::None)
	{
		Camera->SetRelativeRotation(
			ParryCameraOriginalRelativeRotation);
		return;
	}

	// 복귀 중: 원래 회전으로 서서히 돌아가기
	if (ParryCameraState == ENTParryCameraState::Returning)
	{
		const float ReturnAlpha = FMath::Clamp(
			static_cast<float>(Now - ParryCameraReturnStartRealTime)
			/ FMath::Max(ParryCameraCurrentReturnDuration, 0.01f),
			0.0f,
			1.0f);

		Camera->SetRelativeRotation(
			FQuat::Slerp(
				ParryCameraReturnStartRelativeRotation.Quaternion(),
				ParryCameraOriginalRelativeRotation.Quaternion(),
				ReturnAlpha).Rotator());
		return;
	}

	// 접근·유지 중에만 적을 바라보기
	if (ParryCameraState != ENTParryCameraState::Entering &&
		ParryCameraState != ENTParryCameraState::Holding)
	{
		return;
	}

	AActor* Target = ParryCameraTarget.Get();
	USceneComponent* Parent = Camera->GetAttachParent();
	if (!IsValid(Target) || !IsValid(Parent)) return;

	const FVector AimLocation =
		FMath::Lerp(
			Player->GetActorLocation(),
			Target->GetActorLocation(),
			ParryCameraAimBias)
		+ FVector(0.0f, 0.0f, ParryCameraTargetHeight);

	const FVector Direction =
		AimLocation - Camera->GetComponentLocation();

	if (Direction.IsNearlyZero()) return;

	// 적을 바라보는 월드 회전
	const FQuat TargetWorldRotation =
		Direction.Rotation().Quaternion();

	// 부모 소켓 기준의 상대 회전으로 변환
	const FQuat ParentWorldRotation =
		Parent->GetSocketTransform(
			Camera->GetAttachSocketName(),
			RTS_World).GetRotation();

	const FQuat TargetRelativeRotation =
		ParentWorldRotation.Inverse() * TargetWorldRotation;

	// 유지 중에는 바로 조준, 접근 중에는 점진적으로 조준
	float AimAlpha = 1.0f;

	if (ParryCameraState == ENTParryCameraState::Entering)
	{
		AimAlpha = FMath::Clamp(
			static_cast<float>(Now - ParryCameraStartRealTime)
			/ FMath::Max(ParryCameraMoveDuration, 0.01f),
			0.0f,
			1.0f);
	}

	Camera->SetRelativeRotation(
		FQuat::Slerp(
			ParryCameraOriginalRelativeRotation.Quaternion(),
			TargetRelativeRotation,
			AimAlpha).Rotator());

}
void UNTCombatFeelComponent::BeginParryCameraReturn(float Duration)
{
	if (ParryCameraState == ENTParryCameraState::None)
	{
		return;
	}

	ATalPlayerCharacter* Player =
		Cast<ATalPlayerCharacter>(GetOwner());

	if (!IsValid(Player) || !GetWorld()) return;

	USpringArmComponent* CameraBoom = Player->GetCameraBoom();
	if (!IsValid(CameraBoom)) return;

	// 현재 위치에서 복귀를 시작해야 카메라가 튀지 않음
	ParryCameraReturnStartArmLength = CameraBoom->TargetArmLength;
	ParryCameraReturnStartSocketOffset = CameraBoom->SocketOffset;

	if (UCameraComponent* Camera = Player->GetFollowCamera())
	{
		ParryCameraReturnStartRelativeRotation = Camera->GetRelativeRotation();
	}

	ParryCameraReturnStartRealTime = GetWorld()->GetRealTimeSeconds();
	ParryCameraCurrentReturnDuration = FMath::Max(Duration, 0.01f);

	ParryCameraState = ENTParryCameraState::Returning;
	bParryCameraMoving = true;

	SetComponentTickEnabled(true);
}

// Called every frame
void UNTCombatFeelComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateParryCamera();

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Now = World->GetRealTimeSeconds();

	if (bParrySlowMoActive)
	{
		if (Now >= ParrySlowMoEndRealTime)
		{
			StopParrySlowMo();
		}
	}
	else if (bHitStopActive)
	{
		if (Now >= HitStopEndRealTime)
		{
			ApplyDilation(1.0f);
			bHitStopActive = false;
		}
	}

	if (!bParrySlowMoActive && !bHitStopActive && !bParryCameraMoving)
	{
		SetComponentTickEnabled(false);
	}
}

void UNTCombatFeelComponent::TriggerHitStop(ENTHitStopKind Kind)
{
	// 셰이크는 패링중에도 나감 히트스톱만 막힘
	PlayShake(GetShakeFor(Kind));

	if (bParrySlowMoActive)
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Duration = GetDurationFor(Kind);

	if (Duration <= 0.0f)
	{
		return;
	}

	ApplyDilation(HitStopTimeDilation);
	bHitStopActive = true;

	// 연속 타격이면 더 긴쪽 늘리기
	HitStopEndRealTime = FMath::Max(HitStopEndRealTime,
		World->GetRealTimeSeconds() + Duration);

	SetComponentTickEnabled(true);
}

void UNTCombatFeelComponent::StartParrySlowMo()
{


	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	PlayShake(Shake_Parry);
	StartParryCamera();
	
	// 패링이 우선 -> 진행중인 히트스탑 멈추기
	bHitStopActive = false;

	ApplyDilation(ParrySlowMoTimeDilation);
	bParrySlowMoActive = true;
	ParrySlowMoEndRealTime = World->GetRealTimeSeconds()
		+ ParrySlowMoRealDuration;

	SetComponentTickEnabled(true);
}

void UNTCombatFeelComponent::StopParrySlowMo()
{
	if (!bParrySlowMoActive)
	{
		return;
	}

	bParrySlowMoActive = false;
	ApplyDilation(1.0f);
}

