#pragma once

#include "CoreMinimal.h"
#include "Player/UI/NTMenuWidgetBase.h"
#include "NTPauseMenuWidget.generated.h"

class UButton;

UCLASS()
class NAMELESSTAL_API UNTPauseMenuWidget : public UNTMenuWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_MainMenu;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Quit;

	// NTTitleWidget의 StartLevelName과 짝을 이룬다.
	UPROPERTY(EditDefaultsOnly, Category = "Menu")
	FName TitleLevelName = TEXT("TitleMap");

private:
	UFUNCTION() void HandleMainMenuClicked();
	UFUNCTION() void HandleQuitClicked();
	UFUNCTION() void HandleMainMenuHovered();
	UFUNCTION() void HandleQuitHovered();
};