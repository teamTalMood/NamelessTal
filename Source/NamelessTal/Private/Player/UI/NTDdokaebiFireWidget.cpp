#include "Player/UI/NTDdokaebiFireWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Player/TalPlayerCharacter.h"

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

// 위 버튼(Btn_Upgrade) : 호리병 개수를 최대치로 채운다.
void UNTDdokaebiFireWidget::HandleUpgradeClicked()
{
	ATalPlayerCharacter* Player = Cast<ATalPlayerCharacter>(GetOwningPlayerPawn());
	if (!IsValid(Player))
	{
		return;
	}

	Player->RechargeFlask();

	// 확인용. 동작 검증 끝나면 지워도 된다.
	UE_LOG(LogTemp, Warning, TEXT("호리병 충전 완료"));
}


// 아래 버튼(Btn_Leave) : 메뉴를 닫고 게임으로 돌아간다.
void UNTDdokaebiFireWidget::HandleLeaveClicked()
{
	CloseMenu();
}

void UNTDdokaebiFireWidget::HandleUpgradeHovered() { MoveSelectorTo(Btn_Upgrade); }
void UNTDdokaebiFireWidget::HandleLeaveHovered() { MoveSelectorTo(Btn_Leave); }