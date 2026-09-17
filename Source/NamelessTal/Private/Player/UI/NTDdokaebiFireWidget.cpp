#include "Player/UI/NTDdokaebiFireWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UNTDdokaebiFireWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IsValid(Btn_Upgrade))
	{
		Btn_Upgrade->OnClicked.AddUniqueDynamic(this, &UNTDdokaebiFireWidget::HandleUpgradeClicked);
		Btn_Upgrade->OnHovered.AddUniqueDynamic(this, &UNTDdokaebiFireWidget::HandleUpgradeHovered);
	}

	if (IsValid(Btn_Leave))
	{
		Btn_Leave->OnClicked.AddUniqueDynamic(this, &UNTDdokaebiFireWidget::HandleLeaveClicked);
		Btn_Leave->OnHovered.AddUniqueDynamic(this, &UNTDdokaebiFireWidget::HandleLeaveHovered);
	}
}

void UNTDdokaebiFireWidget::SetLocationName(const FText& InName)
{
	if (IsValid(Txt_Location))
	{
		Txt_Location->SetText(InName);
	}
}

void UNTDdokaebiFireWidget::HandleUpgradeClicked()
{
	// 호리병 강화는 아직 미구현. 시스템이 생기면 여기에 연결한다.
	UE_LOG(LogTemp, Warning, TEXT("호리병 강화 선택됨"));
}

void UNTDdokaebiFireWidget::HandleLeaveClicked()
{
	CloseMenu();
}

void UNTDdokaebiFireWidget::HandleUpgradeHovered() { MoveSelectorTo(Btn_Upgrade); }
void UNTDdokaebiFireWidget::HandleLeaveHovered() { MoveSelectorTo(Btn_Leave); }