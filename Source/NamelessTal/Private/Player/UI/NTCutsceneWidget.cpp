// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/UI/NTCutsceneWidget.h"

FReply UNTCutsceneWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::SpaceBar || Key == EKeys::Escape)
	{
		OnSkipRequested.Broadcast();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
