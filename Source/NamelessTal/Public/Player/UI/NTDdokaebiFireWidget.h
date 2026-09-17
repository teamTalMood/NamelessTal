#pragma once

#include "CoreMinimal.h"
#include "Player/UI/NTMenuWidgetBase.h"
#include "NTDdokaebiFireWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class NAMELESSTAL_API UNTDdokaebiFireWidget : public UNTMenuWidgetBase
{
	GENERATED_BODY()


public:
	// 도깨비불마다 다른 장소 이름을 띄운다. 예: 사찰 입구
	void SetLocationName(const FText& InName);
	
protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Upgrade;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Leave;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> Txt_Location;

private:
	UFUNCTION() void HandleUpgradeClicked();
	UFUNCTION() void HandleLeaveClicked();
	UFUNCTION() void HandleUpgradeHovered();
	UFUNCTION() void HandleLeaveHovered();
};