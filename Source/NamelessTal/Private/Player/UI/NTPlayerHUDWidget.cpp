#include "Player/UI/NTPlayerHUDWidget.h"

#include "Components/ProgressBar.h"
#include "Stat/StatComponent.h"
#include "Player/UI/NTMessageWidget.h"
#include "Player/UI/NTTalIconWidget.h"


void UNTPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	WDG_Message = Cast<UNTMessageWidget>(GetWidgetFromName(TEXT("WBP_Notice")));
	ensure(WDG_Message);
}

void UNTPlayerHUDWidget::InitializeWithStatComponent(UStatComponent* InStatComponent)
{
	if (!IsValid(InStatComponent)) return;

	if (IsValid(StatComponent))
	{
		StatComponent->OnHPChanged.RemoveDynamic(
			this,
			&UNTPlayerHUDWidget::HandleHPChanged
		);

		StatComponent->OnSoulChanged.RemoveDynamic(
			this,
			&UNTPlayerHUDWidget::HandleSoulChanged
		);
	}

	StatComponent = InStatComponent;

	StatComponent->OnHPChanged.AddUniqueDynamic(
		this,
		&UNTPlayerHUDWidget::HandleHPChanged
	);

	StatComponent->OnSoulChanged.AddUniqueDynamic(
		this,
		&UNTPlayerHUDWidget::HandleSoulChanged
	);

	RefreshAll();
}

void UNTPlayerHUDWidget::InitializeWithTalComponent(UNTTalComponent* InTalComponent)
{
	if (!IsValid(TalIconWidget)) return;

	TalIconWidget->InitializeWithTalComponent(InTalComponent);
}

void UNTPlayerHUDWidget::ShowWidgetMessage(const FText& Title, const FText& Body, float Duration)
{
	if (WDG_Message == nullptr)
	{
		return;
	}

	WDG_Message->Show(Title, Body, Duration);
}

void UNTPlayerHUDWidget::HandleHPChanged(float NewHP, float MaxHP)
{
	if (IsValid(PB_HP))
	{
		PB_HP->SetPercent(
			MaxHP > 0.0f ? NewHP / MaxHP : 0.0f
		);
	}
}

void UNTPlayerHUDWidget::HandleSoulChanged(float NewSoul, float MaxSoul)
{
	if (IsValid(PB_Soul))
	{
		PB_Soul->SetPercent(
			NewSoul > 0.0f ? NewSoul / MaxSoul : 0.0f
		);
	}
}

void UNTPlayerHUDWidget::RefreshAll()
{
	if (!IsValid(StatComponent)) return;

	HandleHPChanged(
		StatComponent->GetCurrentHP(),
		StatComponent->GetMaxHP()
	);
	HandleSoulChanged(
		StatComponent->GetCurrentSoul(),
		StatComponent->GetMaxSoul()
	);
}

void UNTPlayerHUDWidget::NativeDestruct()
{
	if (IsValid(StatComponent))
	{
		StatComponent->OnHPChanged.RemoveDynamic(
			this,
			&UNTPlayerHUDWidget::HandleHPChanged
		);
		StatComponent->OnSoulChanged.RemoveDynamic(
			this,
			&UNTPlayerHUDWidget::HandleSoulChanged
		);

		StatComponent = nullptr;
	
	}
	Super::NativeDestruct();
}