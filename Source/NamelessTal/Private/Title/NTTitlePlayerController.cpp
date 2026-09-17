#include "Title/NTTitlePlayerController.h"

#include "Blueprint/UserWidget.h"
#include "MCLOG.h"
#include "Player/UI/NTTitleWidget.h"

void ANTTitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	MC_LOG(LogMC, Log, TEXT("Begin"));

	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());

	if (TitleWidgetClass)
	{
		TitleWidget = CreateWidget<UNTTitleWidget>(this, TitleWidgetClass);
		if (IsValid(TitleWidget))
		{
			TitleWidget->AddToViewport();
		}
	}
}
