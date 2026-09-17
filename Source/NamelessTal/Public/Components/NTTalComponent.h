// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NTTalComponent.generated.h"

class ATalBase;

// 탈이 바뀔시 알림 벗은 경우 NewTal은 nullptr로 처리할겁니다
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTalChanged, ATalBase*, NewTal);

UCLASS(ClassGroup = (Tal), meta = (BlueprintSpawnableComponent))
class NAMELESSTAL_API UNTTalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNTTalComponent();

	UPROPERTY(BlueprintAssignable, Category = "Tal|Events")
	FOnTalChanged OnTalChanged;

	UFUNCTION(BlueprintPure, Category = "Tal")
	ATalBase* GetCurrentTal() const;

	UFUNCTION(BlueprintPure, Category = "Tal")
	FName GetTalSocketName() const { return TalSocketName; }

	UFUNCTION(BlueprintCallable, Category = "Tal")
	bool EquipTal(ATalBase* NewTal);

	UFUNCTION(BlueprintCallable, Category = "Tal")
	ATalBase* UnequipTal();

	UFUNCTION(BlueprintCallable, Category = "Tal")
	bool SpawnDefaultTal(TSubclassOf<ATalBase> TalClass);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tal")
	FName TalSocketName = TEXT("TalSocket");

	// 착용자가 자기 얼굴에 맞는 탈 크기를 정합니다.
	// 어떤 탈을 뺏어 써도 이 크기로 통일됩니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tal")
	FVector TalWearScale = FVector::OneVector;

	void ApplyTalWearScale();

private:
	UPROPERTY(Transient)
	TObjectPtr<ATalBase> CurrentTal;
};
