// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/Components/NTHPWidgetComponent.h"
#include "MCLOG.h"
#include "NPC/Components/NTHPBarWidget.h"

void UNTHPWidgetComponent::InitWidget()
{
	Super::InitWidget();

	MC_LOG(LogMC, Log, TEXT("Begin"));

	UNTHPBarWidget* UserWidget = Cast<UNTHPBarWidget>(GetWidget());
	if (UserWidget)
	{
		UserWidget->SetOwiningActor(GetOwner());
	}
}
