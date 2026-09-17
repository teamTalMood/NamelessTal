// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "NTTalChiTargetComponent.generated.h"

class UAnimMontage;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class NAMELESSTAL_API UNTTalChiTargetComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UNTTalChiTargetComponent();
	
	UFUNCTION(BlueprintPure, Category = "Talchi")
	FTransform GetTalchiTransform() const;

	UFUNCTION(BlueprintPure, Category = "Talchi")
	bool IsTalchiAvailable() const;

	UFUNCTION(BlueprintCallable, Category = "Talchi")
	void SetTalchiEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Talchi|Animation")
	UAnimMontage* GetPlayerTalchiMontage() const;

	UFUNCTION(BlueprintPure, Category = "Talchi|Animation")
	UAnimMontage* GetVictimTalchiMontage() const;

	//플레이어가 적의 탈취 가능 부채꼴 안에 있는지 확인
	UFUNCTION(BlueprintPure, Category = "Talchi|Range")
	bool IsInsideTalchiSector(const AActor* PlayerActor) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talchi")
	bool bTalchiEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talchi|Animation")
	TObjectPtr<UAnimMontage> PlayerTalchiMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talchi|Animation")
	TObjectPtr<UAnimMontage> VictimTalchiMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talchi|Range", meta =(ClampMin = "0.0"))
	float TalchiRadius = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talchi|Range", meta=(ClampMin = "0.0", ClampMax = "360.0"))
	float TalchiFullAngle = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talchi|Range", meta=(ClampMin = "0.0"))
	float TalchiMaxHeightDifference = 150.0f;
};
