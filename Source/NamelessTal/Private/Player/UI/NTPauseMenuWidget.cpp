#include "Player/UI/NTPauseMenuWidget.h"

#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UNTPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(Btn_MainMenu))
	{
		Btn_MainMenu->OnClicked.AddUniqueDynamic(this, &UNTPauseMenuWidget::HandleMainMenuClicked);
		Btn_MainMenu->OnHovered.AddUniqueDynamic(this, &UNTPauseMenuWidget::HandleMainMenuHovered);
	}

	if (IsValid(Btn_Quit))
	{
		Btn_Quit->OnClicked.AddUniqueDynamic(this, &UNTPauseMenuWidget::HandleQuitClicked);
		Btn_Quit->OnHovered.AddUniqueDynamic(this, &UNTPauseMenuWidget::HandleQuitHovered);
	}
}

void UNTPauseMenuWidget::HandleMainMenuClicked()
{
	// 레벨을 열기 전에 반드시 일시정지를 푼다. 안 그러면 새 레벨이 멈춘 채로 시작한다.
	UGameplayStatics::SetGamePaused(this, false);
	UGameplayStatics::OpenLevel(this, TitleLevelName);
}

void UNTPauseMenuWidget::HandleQuitClicked()
{
	UGameplayStatics::SetGamePaused(this, false);
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UNTPauseMenuWidget::HandleMainMenuHovered() { MoveSelectorTo(Btn_MainMenu); }
void UNTPauseMenuWidget::HandleQuitHovered() { MoveSelectorTo(Btn_Quit); }