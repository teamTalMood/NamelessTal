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
}

void UNTHPBarWidget::UpdateHpBar(float NewCurrentHp, float InMaxHP)
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	ensure(InMaxHP > 0.0f);
	if (PB_HP)
	{
		PB_HP->SetPercent(NewCurrentHp / InMaxHP);
	}
	else
	{
		MC_LOG(LogMC, Error, TEXT("No PB_HP!!"));
	}
}

void UNTHPBarWidget::UpdateSoulBar(float NewCurrentSoul, float InMaxSoul)
{
	//MC_LOG(LogMC, Log, TEXT("Begin"));

	ensure(InMaxSoul > 0.0f);
	if (PB_Soul)
	{
		PB_Soul->SetPercent(NewCurrentSoul / InMaxSoul);
	}
	else
	{
		MC_LOG(LogMC, Error, TEXT("No PB_Soul!!"));
	}
}
