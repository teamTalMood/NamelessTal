#include "Player/UI/NTMenuWidgetBase.h"

#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Kismet/GameplayStatics.h"


void UNTMenuWidgetBase::OpenMenu()
{
	if (!IsInViewport())
	{
		// HUD보다 위에 올라오도록 ZOrder를 높인다.
		AddToViewport(100);
	}

	UGameplayStatics::SetGamePaused(this, true);

	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetInputMode(FInputModeUIOnly().SetWidgetToFocus(TakeWidget()));
		PC->bShowMouseCursor = true;
	}

	// 포커스가 없으면 NativeOnKeyDown이 안 불린다.
	SetKeyboardFocus();
}

UNTMenuWidgetBase::UNTMenuWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// ESC를 위젯이 직접 받으려면 포커스를 가질 수 있어야 한다.
	SetIsFocusable(true);
}

void UNTMenuWidgetBase::CloseMenu()
{
	// RemoveFromParent 뒤에는 컨트롤러를 못 얻으니 먼저 잡아둔다.
	APlayerController* PC = GetOwningPlayer();

	UGameplayStatics::SetGamePaused(this, false);

	if (PC)
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}

	RemoveFromParent();
}

FReply UNTMenuWidgetBase::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		CloseMenu();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UNTMenuWidgetBase::MoveSelectorTo(UWidget* Target)
{
	if (!IsValid(Target) || !IsValid(Selector))
	{
		return;
	}

	UCanvasPanelSlot* SelectorSlot = Cast<UCanvasPanelSlot>(Selector->Slot);
	UPanelWidget* ParentPanel = Selector->GetParent();

	if (!SelectorSlot || !IsValid(ParentPanel))
	{
		return;
	}

	// 셀렉터가 들어있는 캔버스 기준으로 변환한다. 버튼의 부모가 무엇이든 동작한다.
	const FVector2D AbsPos = Target->GetCachedGeometry().GetAbsolutePosition();
	const FVector2D LocalPos = ParentPanel->GetCachedGeometry().AbsoluteToLocal(AbsPos);

	SelectorSlot->SetPosition(LocalPos + SelectorOffset);
}