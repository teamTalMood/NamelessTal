// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/Components/NTHPBarWidget.h"
#include "MCLOG.h"
#include "Components/ProgressBar.h"

UNTHPBarWidget::UNTHPBarWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{

}

void UNTHPBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//MC_LOG(LogMC, Log, TEXT("Begin"));

	HpProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("PB_HP")));
	ensure(HpProgressBar);

	SoulProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("PB_Soul")));
	ensure(SoulProgressBar);
}

void UNTHPBarWidget::UpdateHpBar(float NewCurrentHp, float InMaxHP)
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	// WidgetComponent/중첩 UserWidget은 NativeConstruct보다 먼저 Update가 호출될 수 있어 재시도
	if (!HpProgressBar)
	{
		HpProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("PB_HP")));
	}

	ensure(InMaxHP > 0.0f);
	if (HpProgressBar)
	{
		HpProgressBar->SetPercent(NewCurrentHp / InMaxHP);
	}
	else
	{
		MC_LOG(LogMC, Error, TEXT("No HpProgressBar!!"));
	}
}

void UNTHPBarWidget::UpdateSoulBar(float NewCurrentSoul, float InMaxSoul)
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	// WidgetComponent/중첩 UserWidget은 NativeConstruct보다 먼저 Update가 호출될 수 있어 재시도
	if (!SoulProgressBar)
	{
		SoulProgressBar = Cast<UProgressBar>(GetWidgetFromName(TEXT("PB_Soul")));
	}

	ensure(InMaxSoul > 0.0f);
	if (SoulProgressBar)
	{
		SoulProgressBar->SetPercent(NewCurrentSoul / InMaxSoul);
	}
	else
	{
		MC_LOG(LogMC, Error, TEXT("No SoulProgressBar!!"));
	}
}
