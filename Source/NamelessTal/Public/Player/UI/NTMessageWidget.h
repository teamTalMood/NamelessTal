// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NTMessageWidget.generated.h"

class UTextBlock;

/**
 *
 */
UCLASS()
class NAMELESSTAL_API UNTMessageWidget : public UUserWidget
{
	GENERATED_BODY()


public:
	// 메시지 띄우고 Duration(3)초 뒤 자동으로 숨긴다
	void Show(const FText& Title, const FText& Body, float Duration);

protected:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Body;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	TObjectPtr<UWidgetAnimation> Anim_Show;

private :
	void Hide();

	FTimerHandle HideTimer;
	FTimerHandle CollapseTimer;

};
