// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/UI/NTTalIconWidget.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Components/NTTalComponent.h"
#include "TalBase.h"



void UNTTalIconWidget::InitializeWithTalComponent(UNTTalComponent* InTalComponent)
{
	if (!IsValid(InTalComponent)) return;

	//이미 다른 컴포넌트르 구독중이면 먼저 끊기
	if (IsValid(TalComponent))
	{
		TalComponent->OnTalChanged.RemoveDynamic(this,
			&UNTTalIconWidget::HandleTalChanged);
	}

	TalComponent = InTalComponent;

	TalComponent->OnTalChanged.AddUniqueDynamic(
		this, &UNTTalIconWidget::HandleTalChanged);
	

	HandleTalChanged(TalComponent->GetCurrentTal());

}

void UNTTalIconWidget::HandleTalChanged(ATalBase* NewTal)
{

	SetTalIcon(IsValid(NewTal) ? NewTal->GetTalIcon() : nullptr);
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
	if (IsValid(TalComponent))
	{
		TalComponent->OnTalChanged.RemoveDynamic(
			this, &UNTTalIconWidget::HandleTalChanged);
		TalComponent = nullptr;
	}

	Super::NativeDestruct();
}


