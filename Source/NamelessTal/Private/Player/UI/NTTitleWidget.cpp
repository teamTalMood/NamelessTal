#include "Player/UI/NTTitleWidget.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Title/NTTitleGameMode.h"
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
	ANTTitleGameMode* GameMode = Cast<ANTTitleGameMode>(UGameplayStatics::GetGameMode(this));
	if (!GameMode)
	{
		MC_LOG(LogMC, Warning, TEXT("No ANTTitleGameMode"));
		return;
	}

	GameMode->StartGame();
}
