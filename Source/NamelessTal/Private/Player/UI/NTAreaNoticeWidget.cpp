#include "Player/UI/NTAreaNoticeWidget.h"

#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "TimerManager.h"

#include "Animation/WidgetAnimation.h"

void UNTAreaNoticeWidget::ShowArea(const FText& AreaName, const FText& Description, UTexture2D* Icon)
{
	// 사라지는 중이었다면 되돌린다
	GetWorld()->GetTimerManager().ClearTimer(CollapseTimer);
	StopAnimation(Anim_Show);

	if (IsValid(Txt_AreaName))
	{
		Txt_AreaName->SetText(AreaName);
	}

	if (IsValid(Txt_Description))
	{
		Txt_Description->SetText(Description);
	}

	if (IsValid(Img_Icon))
	{
		if (Icon)
		{
			Img_Icon->SetBrushFromTexture(Icon);
			Img_Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			// 그림을 안 넣은 트리거면 이미지 칸을 접는다
			Img_Icon->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (Anim_Show)
	{
		PlayAnimation(Anim_Show);
	}
}

void UNTAreaNoticeWidget::HideArea()
{
	if (!Anim_Show)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// 등장 애니메이션을 거꾸로 재생해서 사라지게 한다
	PlayAnimationReverse(Anim_Show);

	GetWorld()->GetTimerManager().SetTimer(
		CollapseTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
			{
				SetVisibility(ESlateVisibility::Collapsed);
			}),
		Anim_Show->GetEndTime(), false);
}