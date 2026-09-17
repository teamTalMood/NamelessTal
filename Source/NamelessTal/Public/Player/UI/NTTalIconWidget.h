// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NTTalIconWidget.generated.h"

class UImage;
class UTexture2D;
class UNTTalComponent;
class ATalBase;
class UTextBlock;
class ATalPlayerCharacter;

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

	// WBP에 같은 이름의 Text를 추가하면 연결됩니다.
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_RemainingUses;

private :
	UPROPERTY()
	TObjectPtr<UNTTalComponent> TalComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<ATalPlayerCharacter> TalOwnerCharacter;

	UFUNCTION()
	void RefreshRemainingUses();

	UFUNCTION()
	void HandleTalChanged(ATalBase* NewTal);

};
