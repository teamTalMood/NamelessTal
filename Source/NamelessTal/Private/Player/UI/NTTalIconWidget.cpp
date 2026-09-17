// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/UI/NTTalIconWidget.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Components/NTTalComponent.h"
#include "TalBase.h"
#include "Components/TextBlock.h"
#include "Player/TalPlayerCharacter.h"



void UNTTalIconWidget::InitializeWithTalComponent(UNTTalComponent* InTalComponent)
{
	//이미 다른 컴포넌트르 구독중이면 먼저 끊기
	if (IsValid(TalComponent))
	{
		TalComponent->OnTalChanged.RemoveDynamic(this,
			&UNTTalIconWidget::HandleTalChanged);
	}

	if (ATalPlayerCharacter* OldOwner = TalOwnerCharacter.Get())
	{
		OldOwner->OnSpecialArtUsesChanged.RemoveDynamic(
			this, &UNTTalIconWidget::RefreshRemainingUses);
	}

	TalComponent = nullptr;
	TalOwnerCharacter.Reset();
	if (IsValid(InTalComponent))
	{
		TalComponent = InTalComponent;
		TalComponent->OnTalChanged.AddUniqueDynamic(
			this, &UNTTalIconWidget::HandleTalChanged);

		ATalPlayerCharacter* OwnerCharacter =
			Cast<ATalPlayerCharacter>(InTalComponent->GetOwner());
		TalOwnerCharacter = OwnerCharacter;
		if (OwnerCharacter)
		{
			OwnerCharacter->OnSpecialArtUsesChanged.AddUniqueDynamic(
				this, &UNTTalIconWidget::RefreshRemainingUses);
		}
	}

	HandleTalChanged(IsValid(TalComponent) ? TalComponent->GetCurrentTal() : nullptr);

}

void UNTTalIconWidget::HandleTalChanged(ATalBase* NewTal)
{

	SetTalIcon(IsValid(NewTal) ? NewTal->GetTalIcon() : nullptr);
	RefreshRemainingUses();
}

void UNTTalIconWidget::RefreshRemainingUses()
{
	if (!IsValid(Text_RemainingUses)) return;

	ATalPlayerCharacter* OwnerCharacter = TalOwnerCharacter.Get();
	if (!IsValid(OwnerCharacter) || !IsValid(TalComponent)
		|| !IsValid(TalComponent->GetCurrentTal()))
	{
		Text_RemainingUses->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	Text_RemainingUses->SetText(
		FText::AsNumber(OwnerCharacter->GetRemainingSpecialArtUses()));
	Text_RemainingUses->SetVisibility(ESlateVisibility::HitTestInvisible);
}



void UNTTalIconWidget::SetTalIcon(UTexture2D* InTexture)
{
	if (!IsValid(Image_RightTal)) return;

	//탈 벗거나 아이콘 없을 시 오른쪽  그림 숨기기
	if (!IsValid(InTexture))
	{
		Image_RightTal->SetVisibility(ESlateVisibility::Hidden);

		return;
	}

	Image_RightTal->SetBrushFromTexture(InTexture, false);
	Image_RightTal->SetVisibility(ESlateVisibility::Visible);

}

void UNTTalIconWidget::NativeDestruct()
{
	if (ATalPlayerCharacter* OwnerCharacter = TalOwnerCharacter.Get())
	{
		OwnerCharacter->OnSpecialArtUsesChanged.RemoveDynamic(
			this, &UNTTalIconWidget::RefreshRemainingUses);
	}
	TalOwnerCharacter.Reset();

	if (IsValid(TalComponent))
	{
		TalComponent->OnTalChanged.RemoveDynamic(
			this, &UNTTalIconWidget::HandleTalChanged);
		TalComponent = nullptr;
	}

	Super::NativeDestruct();
}


