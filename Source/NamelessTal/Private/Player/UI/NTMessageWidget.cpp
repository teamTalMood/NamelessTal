// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/UI/NTMessageWidget.h"

#include "Components/TextBlock.h"
#include "TimerManager.h"

#include "Animation//WidgetAnimation.h"

void UNTMessageWidget::Show(const FText& Title, const FText& Body, float Duration)
{
	GetWorld()->GetTimerManager().ClearTimer(CollapseTimer);
	StopAnimation(Anim_Hide);

	if (IsValid(Txt_Body))
	{
		Txt_Body->SetText(Body);
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (Anim_Show)
	{
		PlayAnimation(Anim_Show);
	}

	// 같은 핸들로 다시 걸면 기존 타이머는 알아서 취소
	GetWorld()->GetTimerManager().SetTimer(HideTimer, this, 	&UNTMessageWidget::Hide, Duration, false);

}

void UNTMessageWidget::NativeConstruct()
{

	Txt_Body = Cast<UTextBlock>(GetWidgetFromName(TEXT("Txt_Body")));
	ensure(Txt_Body);
}

void UNTMessageWidget::Hide()
{

	if (!Anim_Hide)
	{

		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	PlayAnimation(Anim_Hide);
	GetWorld()->GetTimerManager().SetTimer(
		CollapseTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				SetVisibility(ESlateVisibility::Collapsed);
			}),
			Anim_Hide->GetEndTime(), false);
	
}

