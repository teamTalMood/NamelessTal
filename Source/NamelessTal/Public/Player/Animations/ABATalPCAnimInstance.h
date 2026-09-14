// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ABATalPCAnimInstance.generated.h"

class ATalPlayerCharacter;
/**
 * 
 */
UCLASS()
class NAMELESSTAL_API UABATalPCAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public :
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Speed = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float Direction = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsDodging = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsGuarding = false;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsFalling = false;

	// 순수하게 땅에 닿는가 
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool bIsGrounded = false;

protected:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY()
	ATalPlayerCharacter* TalPlayerCharacter;



	//////////////////////////////////////////////////////////////////////
	///////					발 IK ( 땅과의 보간을 위해 )
	///////
	///////

	UPROPERTY(BlueprintReadOnly, Category = "FootIK")
	float PelvisOffset = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "FootIK")
	float FootOffset_L = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "FootIK")
	float FootOffset_R = 0.0f;

	// TwoBoneIK EffectorLocation(로컬 공간 벡터)
	UPROPERTY(BlueprintReadOnly, Category = "FootIK")
	FVector FootEffector_L = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "FootIK")
	FVector FootEffector_R = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "FootIK")
	FRotator FootRotation_L = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "FootIK")
	FRotator FootRotation_R = FRotator::ZeroRotator;

protected:
	// 발 위쪽 트레이스 시작 높이
	UPROPERTY(EditDefaultsOnly, Category = "FootIK")
	float TraceUpDistance = 50.0f;

	//캡슐 바닥 아래로 얼마나 훑을까요
	UPROPERTY(EditDefaultsOnly, Category = "FootIK")
	float TraceDownDistance = 120.0f;

	// 발바닥 두께 보정 (발이 땅에 묻히면 늘리기)
	UPROPERTY(EditDefaultsOnly, Category = "FootIK")
	float FootRadius = 0.0f;

	// 계단/자갈 모서리 값 튀는것 방지용
	UPROPERTY(EditDefaultsOnly, Category = "FootIK")
	float TraceSphereRadius = 6.0f;

	UPROPERTY(EditDefaultsOnly, Category = "FootIK")
	float IKInterpSpeed = 15.0f;

	// 발 회전 위치보다 빠르게 하기 
	UPROPERTY(EditDefaultsOnly, Category = "FootIK")
	float FootRotInterpSpeed = 22.0f;

	//골반이 내려가는 최대치
	UPROPERTY(EditDefaultsOnly, Category = "FootIK")
	float MaxPelvisDrop = 50.0f;

	//골반이 지면차를 따라가는 비율
	UPROPERTY(EditDefaultsOnly, Category = "FootIK")
	float PelvisWeight = 0.8f;

	// 50도 이상은 지면으로 안친다
	UPROPERTY(EditDefaultsOnly, Category = "FootIK")
	float MaxFootSlopeAngle = 50.0f;

	// 발 하나가 캡슐 바닥에서 벗어날 수 있는 최대치
	UPROPERTY(EditDefaultsOnly, Category = "FootIK")
	float MaxFootLift = 45.0f;

	UPROPERTY(EditDefaultsOnly, Category = "FootIK")
	float MaxFootOffsetSpeed = 120.0f;

	void UpdateFootIK(float DeltaSeconds);
	bool TraceFoot(const FName BoneName, float& OutOffset, FRotator& OutRotation) const;

	// 월드 기준 Z 이동량을 해당 본의 로컬 공간 벡터로 변환
	FVector MakeBoneSpaceLift(const FName BoneName, float WorldLift) const;

private:
	// Raw 값을 먼저 스무딩해서 골반/발 이중 보간 지연을 없앱니다
	float SmoothedFootZ_L = 0.0f;
	float SmoothedFootZ_R = 0.0f;
};
