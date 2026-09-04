// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "NTLockOnTargetComponent.generated.h"


UCLASS( ClassGroup=(LockOn), meta=(BlueprintSpawnableComponent) )
class NAMELESSTAL_API UNTLockOnTargetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UNTLockOnTargetComponent();

public:
	UFUNCTION(BlueprintPure, Category = "LockOn")
	bool IsLockOnAvailable() const;

	UFUNCTION(BlueprintPure, Category = "LockOn")
	FVector GetLockOnLocation() const;

	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void SetLockOnEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "LockOn")
	void SetLockOnWidgetVisible(bool bShouldBeVisible);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LockOn")
	bool bLockOnEnabled = true;
};
