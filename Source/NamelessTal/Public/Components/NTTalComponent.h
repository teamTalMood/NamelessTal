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

	UFUNCTION(BlueprintCallable, Category = "Tal")
	bool EquipTal(ATalBase* NewTal);

	UFUNCTION(BlueprintCallable, Category = "Tal")
	ATalBase* UnequipTal();

	UFUNCTION(BlueprintCallable, Category = "Tal")
	bool SpawnDefaultTal(TSubclassOf<ATalBase> TalClass);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tal")
	FName TalSocketName = TEXT("TalSocket");

private:
	UPROPERTY(Transient)
	TObjectPtr<ATalBase> CurrentTal;
};
