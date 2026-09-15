// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NTTalIconWidget.generated.h"

class UImage;
class UTexture2D;
class UNTTalComponent;
class ATalBase;

/**
 * 
 */
UCLASS()
class NAMELESSTAL_API UNTTalIconWidget : public UUserWidget
{
	GENERATED_BODY()



public:
	UFUNCTION(BlueprintCallable, Category = "Tal|UI")
	void SetTalIcon(class UTexture2D* InTexture);

	void InitializeWithTalComponent(UNTTalComponent* InTalComponent);

protected:
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> Image_RightTal;

private :
	UPROPERTY()
	TObjectPtr<UNTTalComponent> TalComponent;

	UFUNCTION()
	void HandleTalChanged(ATalBase* NewTal);

};
