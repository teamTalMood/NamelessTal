// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NTCombatFeelComponent.generated.h"

// 카메라 셰이킹
class UCameraShakeBase;


// 히트 스톱을 거는 상황, 길이를 따로 튜닝

UENUM(BlueprintType)
enum class ENTHitStopKind : uint8
{
	Normal, // 평타 적중
	SpecialArt, // 스킬(강공격 적중)
	Guard, // 가드 
	Kill // 처치
};

enum class ENTParryCameraState : uint8
{
	None, // 평상시
	Entering, // 0.1초간 빠르게 초근접 위치 접근
	Holding, // 초근접 구도 유지하기
	Returning // 원래 거리 복귀
};


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class NAMELESSTAL_API UNTCombatFeelComponent : public UActorComponent
{
	GENERATED_BODY()


public:	
	// Sets default values for this component's properties
	UNTCombatFeelComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	// 상황에 맞는 히트 스톱을 건다
	void TriggerHitStop(ENTHitStopKind Kind);

	// 패링 슬로우모션 시작 (AnimNotify_Parry가 플레이어를 거쳐 호출)
	void StartParrySlowMo();

	// 패링 슬로우모션 즉시 해제 (카운터 발동 시)
	void StopParrySlowMo();

	bool IsParrySlowMoActive() const { return bParrySlowMoActive; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// 멈추는 정도 (0에 가까울 시 완전 정지)
	UPROPERTY(EditAnywhere, Category = "HitStop", meta = (ClampMin= "0.01", ClampMax = "1.0"))
	float HitStopTimeDilation = 0.05f;

	// 상황병 멈추는 시간
	UPROPERTY(EditAnywhere, Category = "HitStop")
	float HitStopDuration_NormalAttack = 0.00f;

	UPROPERTY(EditAnywhere, Category = "HitStop")
	float HitStopDuration_SpecialAttack = 0.1f;

	UPROPERTY(EditAnywhere, Category = "HitStop")
	float HitStopDuration_Guard = 0.07f;

	UPROPERTY(EditAnywhere, Category = "HitStop")
	float HitStopDuration_Kill = 0.12f;

	// 패링 성공 시 느려지는 정도
	UPROPERTY(EditAnywhere, Category = "ParrySlowMo",
		meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float ParrySlowMoTimeDilation = 0.15f;

	// 패링 슬로우모션 길이(실시간 초)
	UPROPERTY(EditAnywhere, Category = "ParrySlowMo")
	float ParrySlowMoRealDuration = 1.0f;

private :
	// 상황 -> 지속시간
	float GetDurationFor(ENTHitStopKind Kind) const;
		
	// 시간 배율을 바꾸는 유일 통로
	void ApplyDilation(float Value);


	bool bHitStopActive = false;
	//실시간 기준 만료 시각, (게임시간쓰면 시간배율도 적용되니 실시간으로 사용)
	float HitStopEndRealTime = 0.0f;


	// 패리 스탑
	bool bParrySlowMoActive = false;
	float ParrySlowMoEndRealTime = 0.0f;

protected: // 카메라 셰이킹
	// 상황별 셰이크 애셋. BP에서 지정한다
	UPROPERTY(EditAnywhere, Category = "CameraShake")
	TSubclassOf<UCameraShakeBase> Shake_NormalAttack;

	UPROPERTY(EditAnywhere, Category = "CameraShake")
	TSubclassOf<UCameraShakeBase> Shake_SpecialAttack;

	UPROPERTY(EditAnywhere, Category = "CameraShake")
	TSubclassOf<UCameraShakeBase> Shake_Guard;

	UPROPERTY(EditAnywhere, Category = "CameraShake")
	TSubclassOf<UCameraShakeBase> Shake_Kill;

	UPROPERTY(EditAnywhere, Category = "CameraShake")
	TSubclassOf<UCameraShakeBase> Shake_Parry;

	// 전체 세기 배율 0이면 셰이크 끄기
	UPROPERTY(EditAnywhere, Category = "CameraShake",
		meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float CameraShakeScale = 1.0f;

private:
	TSubclassOf<UCameraShakeBase> GetShakeFor(ENTHitStopKind Kind) const;
	void PlayShake(TSubclassOf<UCameraShakeBase> ShakeClass);


public:
	// 패링 카메라 이동을 시작하고 갱신하는 함수
	void StartParryCamera();

	// 패링 카메라 시작 시 고정될  적 전달받기
	void SetParryCameraTarget(AActor* InTarget);

	// 실제 반격 종료 시각 카메라 전달
	void SetParryCameraCounterEndTime(double EndRealTime);

	// 강공격 발동 성공시 천천히 복귀하기
	void StartParryCameraSkillReturn();

	bool IsParryCameraMoving() const { return bParryCameraMoving; }

protected:

	void UpdateParryCamera();


	// 지정한 시간에 걸쳐 원래거리, 위치로 복귀하기
	void BeginParryCameraReturn(float Duration);

	// 현재 카메라가 이동 중인지
	bool bParryCameraMoving = false;

	ENTParryCameraState ParryCameraState =
		ENTParryCameraState::None;

	TWeakObjectPtr<AActor> ParryCameraTarget;

	// 슬로우모션과 무관하게 이동 시간을 계산
	double ParryCameraStartRealTime = 0.0;

	// 이동을 시작한 방향과 도착할 방향
	FRotator ParryCameraStartRotation = FRotator::ZeroRotator;
	FRotator ParryCameraTargetRotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, Category = "Camera|Parry",
		meta = (ClampMin = "0.01"))
	float ParryCameraMoveDuration = 0.1f;

	// 패링 카메라의 위아래 각도
	UPROPERTY(EditAnywhere, Category = "Camera|Parry",
		meta = (ClampMin = "-60.0", ClampMax = "60.0"))
	float ParryCameraPitch = -10.0f;

	// 연출 시작 전 카메라 설정
	float ParryCameraOriginalArmLength = 0.0f;
	FVector ParryCameraOriginalSocketOffset = FVector::ZeroVector;

	// 초근접 카메라 거리: 화면을 보며 조절할 시작값
	UPROPERTY(EditAnywhere, Category = "Camera|Parry",
		meta = (ClampMin = "0.0"))
	float ParryCameraCloseArmLength = 100.0f;

	// 초근접 시 어깨 쪽으로 옮길 위치
	UPROPERTY(EditAnywhere, Category = "Camera|Parry")
	FVector ParryCameraCloseSocketOffset = FVector(0.0f, 40.0f, 10.0f);

	// 공격하지 않았을 때 빠른 복귀 시간
	UPROPERTY(EditAnywhere, Category = "Camera|Parry",
		meta = (ClampMin = "0.01"))
	float ParryCameraFastReturnDuration = 0.1f;

	// 강공격 발동 성공 시 느린 복귀 시간
	UPROPERTY(EditAnywhere, Category = "Camera|Parry",
		meta = (ClampMin = "0.01"))
	float ParryCameraSkillReturnDuration = 0.5f;

	// 실제 반격 종료 시각을 전달받아 저장
	double ParryCameraCounterEndRealTime = 0.0;

	// 복귀 시작 시점의 거리와 위치
	float ParryCameraReturnStartArmLength = 0.0f;
	FVector ParryCameraReturnStartSocketOffset = FVector::ZeroVector;

	// 이번 복귀의 시작 시각과 소요 시간
	double ParryCameraReturnStartRealTime = 0.0;
	float ParryCameraCurrentReturnDuration = 0.1f;

	// 연출 전 카메라 자체의 회전
	FRotator ParryCameraOriginalRelativeRotation = FRotator::ZeroRotator;

	// 복귀를 시작할 때 카메라 자체의 회전
	FRotator ParryCameraReturnStartRelativeRotation = FRotator::ZeroRotator;
	
	UPROPERTY(EditAnywhere, Category = "Camera|Parry",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ParryCameraAimBias = 0.65f;

	// 적의 액터 중심에서 얼마나 위를 바라볼지
	UPROPERTY(EditAnywhere, Category = "Camera|Parry")
	float ParryCameraTargetHeight = 40.0f;
};
