// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NTTalComponent.generated.h"

class ATalBase;

UCLASS(ClassGroup = (Tal), meta = (BlueprintSpawnableComponent))
class NAMELESSTAL_API UNTTalComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNTTalComponent();

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
