// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/Components/NTHPBarWidget.h"
#include "MCLOG.h"
#include "Components/ProgressBar.h"

UNTHPBarWidget::UNTHPBarWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	MaxHp = -1.0f;
}

void UNTHPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	MC_LOG(LogMC, Log, TEXT("Begin"));

	HpProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("PB_HP")));
	ensure(HpProgressBar);
}

void UNTHPBarWidget::UpdateHpBar(float NewCurrentHp, float MaxHP)
{
	MC_LOG(LogMC, Log, TEXT("Begin"));

	ensure(MaxHp > 0.0f);
	if (HpProgressBar)
	{
		HpProgressBar->SetPercent(NewCurrentHp / MaxHp);
	}
	else
	{
		MC_LOG(LogMC, Error, TEXT("No HpProgressBar!!"));
	}
}
