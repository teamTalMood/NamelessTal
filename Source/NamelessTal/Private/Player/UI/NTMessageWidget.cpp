// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/UI/NTMessageWidget.h"

#include "Components/TextBlock.h"
#include "TimerManager.h"

void UNTMessageWidget::Show(const FText& Title, const FText& Body, float Duration)
{
	// 배너 디자인엔 제목이 없을 수 없습니다요
	if (IsValid(Txt_Title))
	{
		Txt_Title->SetText(Title);
	}

	if (IsValid(Txt_Body))
	{
		Txt_Body->SetText(Body);
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);

	// 같은 핸들로 다시 걸면 기존 타이머는 알아서 취소
	GetWorld()->GetTimerManager().SetTimer(HideTimer, this, 	&UNTMessageWidget::Hide, Duration, false);

}

void UNTMessageWidget::NativeConstruct()
{
	Txt_Title = Cast<UTextBlock>(GetWidgetFromName(TEXT("Txt_Title")));
	ensure(Txt_Title);

	Txt_Body = Cast<UTextBlock>(GetWidgetFromName(TEXT("Txt_Body")));
	ensure(Txt_Body);
}

void UNTMessageWidget::Hide()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

