#include "Player/UI/NTDdokaebiFireWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Player/TalPlayerCharacter.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

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


// 아래 버튼(Btn_Leave) : 게임을 종료한다.
void UNTDdokaebiFireWidget::HandleLeaveClicked()
{
	// 종료 전에 일시정지를 반드시 푼다. 멈춘 채로 QuitGame하면 종료가 막힐 수 있다.
	UGameplayStatics::SetGamePaused(this, false);
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UNTDdokaebiFireWidget::HandleUpgradeHovered() { MoveSelectorTo(Btn_Upgrade); }
void UNTDdokaebiFireWidget::HandleLeaveHovered() { MoveSelectorTo(Btn_Leave); }