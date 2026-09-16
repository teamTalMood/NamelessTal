#include "Player/UI/NTTitleWidget.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "MCLOG.h"

void UNTTitleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(Btn_Start))
	{
		Btn_Start->OnClicked.AddUniqueDynamic(this, &UNTTitleWidget::HandleStartClicked);
	}
}

void UNTTitleWidget::HandleStartClicked()
{
	if (StartLevelName.IsNone())
	{
		MC_LOG(LogMC, Warning, TEXT("StartLevelName not set"));
		return;
	}

	UGameplayStatics::OpenLevel(this, StartLevelName);
}
